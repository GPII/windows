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

var Promise = require("bluebird"),
    child_process = require("child_process"),
    service = require("./service.js"),
    ipc = require("./gpii-ipc.js");

var gpiiClient = service.module("gpiiClient");

/**
 * A map of functions for the requests handled.
 *
 * @type {function(request)}
 */
gpiiClient.requestHandlers = {
    "echo": function (request) {
        return {
            message: "Echo back from service",
            youSaid: request
        };
    },

    /**
     * Executes something.
     *
     * @param request {Object} The request data.
     * @param request.command {string} The command to run.
     * @param request.args {string[]} Arguments to pass.
     * @param request.options {Object} The options argument for child_process.spawn.
     * @param request.wait {boolean} True to wait for the process to terminate before resolving.
     * @param request.capture {boolean} True capture output to stdout/stderr members of the response; implies wait=true.
     * @return {Promise} Resolves when the process has started, if wait=false, or when it's terminated.
     */
    "execute": function (request) {
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
                        if (output.stdout.length < 0xffff) {
                            output.stdout += data;
                        }
                    });
                    child.stderr.on("data", function (data) {
                        if (output.stderr.length < 0xffff) {
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
    }
};

/**
 * Adds a command handler.
 *
 * @param requestName {string} The request name.
 * @param callback {function(request)} The callback function.
 */
gpiiClient.addRequestHandler = function (requestName, callback) {
    gpiiClient.requestHandlers[requestName] = callback;
};

/**
 * The IPC connection
 * @type {IpcConnection}
 */
gpiiClient.ipcConnection = null;

/**
 * Called when the GPII user process has connected to the service.
 *
 * @param ipcConnection {IpcConnection} The IPC connection.
 */
gpiiClient.connected = function (ipcConnection) {
    this.ipcConnection = ipcConnection;
    ipcConnection.requestHandler = gpiiClient.requestHandler;
    service.log("Established IPC channel with the GPII user process");

    setTimeout(function () {
        gpiiClient.sendRequest("echo", {a: 123}).then(function (r) {
            service.log("echo back", r);
        }, service.log);
    }, 1000);
};

/**
 * Handles a request from the GPII user process.
 *
 * @param request {Object} The request data.
 * @return {Promise|object} The response data.
 */
gpiiClient.requestHandler = function (request) {
    var handler = request.action && gpiiClient.requestHandlers[request.action];
    if (handler) {
        return handler(request.data);
    }
};

/**
 * Sends a request to the GPII user process.
 *
 * @param request {Object} The request data.
 * @return {Promise} Resolves with the response when it is received.
 */
gpiiClient.sendRequest = function (action, requestData) {
    var req = {
        action: action,
        data: requestData
    };
    return ipc.sendRequest("gpii", req);
};

service.on("ipc.connected:gpii", gpiiClient.connected);

module.exports = gpiiClient;
