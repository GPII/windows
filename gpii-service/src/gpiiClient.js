/* Handles the requests and responses of the GPII user process.
 *
 * Copyright 2017 Raising the Floor - International
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * The R&D leading to these results received funding from the
 * Department of Education - Grant H421A150005 (GPII-APCP). However,
 * these results do not necessarily represent the policy of the
 * Department of Education, and you should not assume endorsement by the
 * Federal Government.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

"use strict";

var child_process = require("child_process"),
    crypto = require("crypto"),
    fs = require("fs"),
    path = require("path"),
    service = require("./service.js"),
    ipc = require("./gpii-ipc.js"),
    processHandling = require("./processHandling.js"),
    windows = require("./windows.js");

var gpiiClient = {};
module.exports = gpiiClient;

gpiiClient.options = {
    // Number of seconds to wait for a response from the client before determining that the process is unresponsive.
    clientTimeout: 120
};

/**
 * A map of functions for the requests handled.
 *
 * @type {Function(request)}
 */
gpiiClient.requestHandlers = {};

/**
 * Executes something.
 *
 * @param {Object} request The request data.
 * @param {String} request.command The command to run.
 * @param {Array<String>} request.args Arguments to pass.
 * @param {Object} request.options The options argument for child_process.spawn.
 * @param {Boolean} request.wait True to wait for the process to terminate before resolving.
 * @param {Boolean} request.capture True capture output to stdout/stderr members of the response; implies wait=true.
 * @return {Promise} Resolves when the process has started, if wait=false, or when it's terminated.
 */
gpiiClient.requestHandlers.execute = function (request) {
    return new Promise(function (resolve, reject) {
        if (request.capture) {
            request.wait = true;
        }

        // spawn is used instead of exec, to avoid using the shell and worry about escaping.
        var child = child_process.spawn(request.command, request.args, request.options);

        child.on("error", function (err) {
            reject({
                isError: true,
                error: err
            });
        });

        if (child.pid) {
            var output = null;
            if (request.capture) {
                output = {
                    stdout: "",
                    stderr: ""
                };
                child.stdout.on("data", function (data) {
                    // Limit the output to ~1 million characters
                    if (output.stdout.length < 0xfffff) {
                        output.stdout += data;
                    }
                });
                child.stderr.on("data", function (data) {
                    if (output.stderr.length < 0xfffff) {
                        output.stderr += data;
                    }
                });
            }

            if (request.wait) {
                child.on("exit", function (code, signal) {
                    var result = {
                        code: code,
                        signal: signal
                    };
                    if (output) {
                        result.output = output;
                    }
                    resolve(result);
                });
            } else {
                resolve({pid: child.pid});
            }
        }
    });
};

/**
 * The user process is shutting down (eg, due to the user logging out of the system). The client sends this request
 * to prevent the service restarting it when it terminates.
 *
 */
gpiiClient.requestHandlers.closing = function () {
    service.logImportant("GPII Client is closing itself");
    gpiiClient.inShutdown = true;
    processHandling.dontRestartProcess(gpiiClient.ipcConnection.processKey);
};

/**
 * Gets the client credentials from the secrets file.
 * @return {Object} The client credentials.
 */
gpiiClient.requestHandlers.getClientCredentials = function () {
    var secrets = service.getSecrets();
    return secrets && secrets.clientCredentials;
};

/**
 * Signs a string or Buffer (or an array of such), using the secret.
 *
 * @param {Object} request The signing request
 * @param {String|Buffer} request.payload The thing to sign.
 * @param {String} request.keyName Field name in the secrets file whose value is used as a key.
 * @return {String} The HMAC digest of payload, as a hex string.
 */
gpiiClient.requestHandlers.sign = function (request) {
    var result = null;

    var secrets = service.getSecrets();
    var key = secrets && secrets[request.keyName];

    if (key) {
        var hmac = crypto.createHmac("sha256", key);

        var payloads = Array.isArray(request.payload) ? request.payload : [request.payload];
        payloads.forEach(function (item) {
            hmac.update(item);
        });

        result = hmac.digest("hex");
    } else {
        service.logError("Attempted to sign with a key named "
            + request.keyName + ", but no such value exists in the secrets file");
    }

    return result;
};

/** @type {Boolean} true if the client is being shutdown */
gpiiClient.inShutdown = false;

/**
 * Adds a command handler.
 *
 * @param {String} requestType The request type.
 * @param {Function} callback The callback function.
 */
gpiiClient.addRequestHandler = function (requestType, callback) {
    gpiiClient.requestHandlers[requestType] = callback;
};

/**
 * The IPC connection
 * @type {IpcConnection}
 */
gpiiClient.ipcConnection = null;

/**
 * Called when the GPII client process is about to start.
 *
 * Determines if the process should be started (based on the metrics switch), and sets some environment variables.
 *
 * @param {ProcessConfig} processConfig The process config. Will be modified.
 */
gpiiClient.clientStarting = function (processConfig) {
    var metrics = gpiiClient.getMetricsConfig();
    if (metrics) {
        // Get the NODE_ENV value ("the metrics switch")
        if (metrics.value === "off:off") {
            processConfig.disabled = "disabled via metrics switch";
        } else if (metrics.config) {
            processConfig.env[metrics.envName] = metrics.config;
        }

        if (windows.isService()) {
            gpiiClient.toggleDesktopIcons(metrics.value.startsWith("on:"));
        }
    }
};

/**
 * Gets the config for the GPII process (the metrics switch).
 * @return {Object} Object containing the metricsSwitch (`value`), the config to use (`config`), and the name of the
 * environment variable to set (`envName`)
 */
gpiiClient.getMetricsConfig = function () {
    var togo;

    if (service.config.gpiiConfig) {
        var siteConfig = service.getSiteConfig();
        if (siteConfig && siteConfig.metricsSwitch) {
            togo = {
                value: siteConfig.metricsSwitch.toLowerCase(),
                envName: service.config.gpiiConfig.env || "NODE_ENV"
            };
            togo.config = service.config.gpiiConfig[togo.value];
        }
    }

    return togo;
};

/**
 * Hide or show the morphic desktop icon, depending on the value of the metrics switch.
 * When hiding, they're moved from C:\Users\Public\Desktop to C:\ProgramData\Morphic\Icons. Showing will move them back.
 * This needs to be performed by the service, because they're owned by administrator.
 *
 * @param {Boolean} showIcons true to show the icons on the desktop. false to remove them.
 */
gpiiClient.toggleDesktopIcons = function (showIcons) {

    var iconFiles = [ "Morphic QuickStrip.lnk", "Reset to Standard.lnk" ];

    var desktopPath = path.join(process.env.PUBLIC || "C:\\Users\\Public", "Desktop");
    var stashPath = path.join(process.env.PROGRAMDATA || "C:\\ProgramData", "Morphic");


    iconFiles.forEach(function (file) {

        var desktop = path.join(desktopPath, file);
        var stashed = path.join(stashPath, file);

        service.logDebug("Setting desktop icon: ", desktop, stashed, showIcons ? "show" : "hide");

        try {
            // Ensure there's a copy in the stash location
            if (!fs.existsSync(stashed)) {
                fs.copyFileSync(desktop, stashed);
            }

            var iconExists = fs.existsSync(desktop);
            if (showIcons) {
                if (!iconExists) {
                    // Copy it back from the stash
                    fs.copyFileSync(stashed, desktop);
                }
            } else {
                // Remove the desktop icon
                if (iconExists) {
                    fs.unlinkSync(desktop);
                }
            }

        } catch (e) {
            service.logError("Error setting desktop icon", e.message, e);
        }
    });
};

/**
 * Called when the GPII user process has connected to the service.
 *
 * @param {IpcConnection} ipcConnection The IPC connection.
 */
gpiiClient.connected = function (ipcConnection) {
    gpiiClient.ipcConnection = ipcConnection;
    gpiiClient.inShutdown = false;
    ipcConnection.requestHandler = gpiiClient.requestHandler;

    service.log("Established IPC channel with the GPII user process");

    gpiiClient.monitorStatus(gpiiClient.options.clientTimeout);
};

/**
 * Called when the GPII user process has disconnected from the service.
 *
 * @param {IpcConnection} ipcConnection The IPC connection.
 */
gpiiClient.closed = function (ipcConnection) {
    service.log("Lost IPC channel with the GPII user process");
    gpiiClient.ipcConnection = null;
    if (!gpiiClient.inShutdown) {
        processHandling.stopChildProcess(ipcConnection.processKey, true);
    }
};

/**
 * Monitors the status of the GPII process, by continually sending a request and waiting for a reply. If there is no
 * reply within a timeout, then the process is killed.
 *
 * @param {Number} timeout Seconds to wait before determining that the process is unresponsive.
 */
gpiiClient.monitorStatus = function (timeout) {

    var isRunning = false;
    var processKey = gpiiClient.ipcConnection && gpiiClient.ipcConnection.processKey;

    gpiiClient.sendRequest("status").then(function (response) {
        isRunning = response && response.isRunning;
    });

    setTimeout(function () {
        if (gpiiClient.inShutdown || !gpiiClient.ipcConnection) {
            // No longer needs to be monitored.
        } else if (isRunning) {
            gpiiClient.monitorStatus(timeout);
        } else {
            service.logError("GPII client is not responding.");
            processHandling.stopChildProcess(processKey, true);
        }
    }, timeout * 1000);
};

/**
 * Handles a request from the GPII user process.
 *
 * @param {ServiceRequest} request The request data.
 * @return {Promise|Object} The response data.
 */
gpiiClient.requestHandler = function (request) {
    var handler = request.requestType && gpiiClient.requestHandlers[request.requestType];
    service.logDebug("Got request:", request);
    if (handler) {
        return handler(request.requestData);
    }
};

/**
 * Sends a request to the GPII user process.
 *
 * @param {String} requestType The request type.
 * @param {Object} requestData The request data.
 * @return {Promise} Resolves with the response when it is received.
 */
gpiiClient.sendRequest = function (requestType, requestData) {
    var req = {
        requestType: requestType,
        requestData: requestData
    };
    return ipc.sendRequest(gpiiClient.ipcConnection, req);
};

/**
 * Tell the GPII user process to shutdown.
 * @return {Promise} Resolves with the response when it is received.
 */
gpiiClient.shutdown = function () {
    if (gpiiClient.ipcConnection && !gpiiClient.inShutdown) {
        gpiiClient.inShutdown = true;
        return gpiiClient.sendRequest("shutdown");
    }
};

service.on("ipc.connected:gpii", gpiiClient.connected);
service.on("ipc.closed:gpii", gpiiClient.closed);

service.on("process.starting", function (processKey, procConfig) {
    if (procConfig.ipc === "gpii") {
        gpiiClient.clientStarting(procConfig);
    }
});

service.on("stopping", function (promises) {
    promises.push(gpiiClient.shutdown());
});
