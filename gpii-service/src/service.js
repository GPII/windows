/* The GPII windows service.
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

var os_service = require("os-service"),
    path = require("path"),
    fs = require("fs"),
    events = require("events"),
    JSON5 = require("json5"),
    logging = require("./logging.js"),
    windows = require("./windows.js"),
    parseArgs = require("minimist");

/**
 * The service object is a central event source, to reduce coupling between the different modules. Events can be emitted
 * for other modules to act upon.
 *
 * The events are:
 *  start - The service has started.
 *
 *  stopping(promises) - The service is about to stop. Add a promise to the first argument to delay the shutdown.
 *  stop - The service is about to stop (second pass).
 *
 *  service.<control code name> - The service has received a control code (see service.controlHandler())
 *
 *  ipc.connected(<connection name>, {IpcConnection}) - Something has connected (and authenticated) to an IPC channel.
 *
 *  process.stop(<process key>) - A child process has stopped.
 */
var service = new events.EventEmitter();

service.args = parseArgs(process.argv.slice(2));

// true if the process running as a Windows Service, otherwise a normal user process.
service.isService = !!service.args.service;
// true if the service is an exe file (rather than node)
service.isExe = !!process.versions.pkg;

/** Log something */
service.log = logging.log;
service.logFatal = logging.fatal;
service.logError = logging.error;
service.logImportant = logging.important;
service.logWarn = logging.warn;
service.logDebug = logging.debug;

/**
 * Loads the config file, which may be found in the first of the following locations:
 * - The file parameter.
 * - "--config" command line option.
 * - "service.json5" next to the service executable.
 * - "service.json5" in the config directory.
 *
 * @param {String} dir The directory form which relative paths are used.
 * @param {String} file [optional] The config file.
 */
service.loadConfig = function (dir, file) {
    // Load the config file.
    var configFile = file || service.args.config;
    if (!configFile) {
        if (service.isService) {
            // Check if there's a config file next to the service executable.
            var tryFile = path.join(dir, "service.json5");
            if (fs.existsSync(tryFile)) {
                configFile = tryFile;
            }
        }
        if (!configFile) {
            if (service.isService) {
                // Use the built-in config file.
                configFile = path.join(__dirname, "../config/service.json5");
            } else {
                configFile = "config/service.dev.json5";
            }
        }
    }
    if ((configFile.indexOf("/") === -1) && (configFile.indexOf("\\") === -1)) {
        configFile = path.join(dir, "config", configFile);
    }

    service.log("Loading config file", configFile);
    service.config = JSON5.parse(fs.readFileSync(configFile));

    // Change to the configured log level (if it's not passed via command line)
    if (!service.args.loglevel && service.config.logging && service.config.logging.level) {
        logging.setLogLevel(service.config.logging.level);
    }
};

/**
 * Gets the the secrets, which is the data stored in the secrets file.
 *
 * The secret is installed in a separate installer, which could occur after Morphic was installed. Also, the secret
 * may be later updated. Because of this, the secret is read each time it is used.
 *
 * @return {Object} The secret, or null if the secret could not be read. This shouldn't be logged.
 */
service.getSecrets = function () {
    var secret = null;

    try {
        var file = service.config.secretFile
            && path.resolve(windows.expandEnvironmentStrings(service.config.secretFile));
        if (file) {
            service.log("Reading secrets from " + file);
            secret = JSON5.parse(fs.readFileSync(file));
        } else {
            service.logError("The secrets file is not configured");
        }
    } catch (e) {
        service.logWarn("Unable to read the secrets file " + service.config.secretFile, e);
    }

    return secret ? secret : null;
};

/**
 * Called when the service has just started.
 */
service.start = function () {
    service.isService = os_service.getState() !== "stopped";
    // Control codes are how Windows tells services about certain system events. These are caught in os_service.
    // Register the control codes that the service would be interested in.
    os_service.acceptControl(["start", "stop", "shutdown", "sessionchange"], true);
    // Handle all registered control codes.
    os_service.on("*", service.controlHandler);
    os_service.on("stop", service.stop);

    service.emit("start");
    service.log("service start");

    if (windows.isUserLoggedOn()) {
        // The service was started while a user is already active; fake a session-change event to get things started.
        service.controlHandler("sessionchange", "session-logon");
    }
};

/**
 * Stop the service.
 */
service.stop = function () {
    var promises = [];
    service.emit("stopping", promises);
    Promise.all(promises).then(function () {
        service.emit("stop", promises);
        os_service.stop();
    });
};

/**
 * Called when the service receives a control code. This is what's used to detect a shutdown, service stop, or Windows
 * user log-in/out.
 *
 * This emits a "service.<controlName>" event.
 *
 * Possible control codes: start, stop, pause, continue, interrogate, shutdown, paramchange, netbindadd, netbindremove,
 * netbindenable, netbinddisable, deviceevent, hardwareprofilechange, powerevent, sessionchange, preshutdown,
 * timechange, triggerevent.
 *
 * For this function to receive a control code, it needs to be added via os_service.acceptControl()
 *
 * For the "sessionchange" control code, the eventType parameter will be one of:
 * console-connect, console-disconnect, remote-connect, remote-disconnect, session-logon, session-logoff, session-lock,
 * session-unlock, session-remote, session-create, session-terminate.
 *
 * See also: https://msdn.microsoft.com/library/ms683241
 *
 * @param {String} controlName Name of the control code.
 * @param {String} [eventType] For the "sessionchange" control code, this specifies the type of event.
 */
service.controlHandler = function (controlName, eventType) {
    service.logDebug("Service control: ", controlName, eventType);
    service.emit("service." + controlName, eventType);
};


// Change directory to a sane location, allowing relative paths in the config file.
var dir = null;
if (service.isExe) {
    // The directory containing this executable (morphic-service.exe)
    dir = path.dirname(process.execPath);
} else {
    // Path of the index.js.
    dir = path.join(__dirname, "..");
}

process.chdir(dir);

// Load the configuration
service.loadConfig(dir);


module.exports = service;
