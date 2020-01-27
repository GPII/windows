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

var os_service = require("@gpii/os-service"),
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
 * @typedef {Object} Config
 * @property {Object<String,ProcessConfig>} processes Child process.
 * @property {Object} logging Logging settings
 * @property {String} secretFile The file containing site-specific information.
 * @property {String} package.json The package.json file for gpii-app.
 * @property {String} siteConfigFile The site-config file.
 * @property {AutoUpdateConfig} autoUpdate Auto update settings.
 * @property {String} morphicVersion [generated] The version field from gpii-app's package.json.
 */

/**
 * @type Config
 */
service.config = {
    processes: {},
    logging: {
        level: "DEBUG"
    },
    autoUpdate: {
        lastUpdatesFile: path.join(process.env.ProgramData, "Morphic/last-updates.json5")
    },
    "package.json": "resources/app/package.json"
};

/**
 * Loads the config file, which may be found in the first of the following locations:
 * - The file parameter.
 * - "--config" command line option.
 * - "service.json5" next to the service executable.
 * - "service.json5" in the config directory.
 *
 * @param {String} dir The directory form which relative paths are used.
 * @param {String} file [optional] The config file.
 * @return {Config} The loaded configuration.
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
            } else if (fluid) {
                // fluid is only defined during testing
                configFile = "config/service.testing.json5";
            } else {
                configFile = "config/service.dev.json5";
            }
        }
    }
    if ((configFile.indexOf("/") === -1) && (configFile.indexOf("\\") === -1)) {
        configFile = path.join(dir, "config", configFile);
    }

    service.log("Loading config file", configFile);
    var config = JSON5.parse(fs.readFileSync(configFile));
    // Expand all environment %variables% within the config.
    config = windows.expandEnvironmentStrings(config);

    // Change to the configured log level (if it's not passed via command line)
    if (!service.args.loglevel && config.logging && config.logging.level) {
        logging.setLogLevel(config.logging.level);
    }

    // Get the gpii-app version
    var packageJson = service.loadJSON(config["package.json"], "package.json file");
    config.morphicVersion = packageJson && packageJson.version;

    return config;
};

/**
 * Loads a JSON/JSON5 file, parsing the content.
 *
 * @param {String} file Path to the file.
 * @param {String} description A descriptive name of the file, for logging.
 * @return {Object|null} The de-serialised file, or null on error.
 */
service.loadJSON = function (file, description) {
    var obj = null;

    try {
        file = file && path.resolve(file);
        if (file) {
            service.log("Reading", description, file);
            obj = JSON5.parse(fs.readFileSync(file));
        } else {
            service.logError("The path for", description, "is not configured");
        }
    } catch (e) {
        service.logWarn("Unable to read", description, file, e);
    }

    return obj ? obj : null;
};

/**
 * The site-specific secrets file.
 * @typedef {Object} SecretFile
 * @property {String} site Site identifier ("domain").
 * @property {Object} clientCredentials Client credentials (private).
 * @property {String} signKey Signing key (private).
 */

/**
 * Gets the secrets, which is the data stored in the secrets file.
 *
 * The secret is installed in a separate installer, which could occur after Morphic was installed. Also, the secret
 * may be later updated. Because of this, the secret is read each time it is used.
 *
 * @return {SecretFile} The secret, or null if the secret could not be read. This shouldn't be logged.
 */
service.getSecrets = function () {
    return service.loadJSON(service.config.secretFile, "secrets file");
};

/**
 * Loads the site-config file, from `service.config.siteConfigFile`.
 *
 * siteConfigFile can be an array, where the first successful loaded file is returned.
 *
 * @return {Object} The de-serialised site-config file.
 */
service.getSiteConfig = function () {
    var files = Array.isArray(service.config.siteConfigFile)
        ? service.config.siteConfigFile
        : [service.config.siteConfigFile];
    var result = null;

    files.some(function (file) {
        result = service.loadJSON(file, "site-config file");
        return result;
    });
    return result;
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
 * Stop the service, after shutting down the child processes.
 */
service.stop = function () {

    // Timeout waiting for things to shutdown.
    var timer = setTimeout(service.stopNow, 10000);

    var promises = [];
    service.logImportant("Shutting down");
    service.emit("stopping", promises);

    Promise.all(promises).then(function () {
        clearTimeout(timer);
        service.stopNow();
    });
};

/**
 * Stop the service immediately. This function should not return.
 */
service.stopNow = function () {
    service.logFatal("Stopping now");

    // Ensure the process always terminates.
    process.nextTick(process.exit);

    try {
        service.emit("stop");
    } finally {
        // This will end the process, and not return.
        os_service.stop();
    }
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

/**
 * Returns a promise that resolves when the service is ready to start the child processes. That is, when all promises
 * in `service.readyPromises` have resolved.
 * @return {Promise} Resolves when the service is ready to start the child processes.
 */
service.isReady = function () {
    return Promise.all(service.readyPromises);
};

service.readyWhen = function (promise) {
    service.readyPromises.push(promise);
};

service.readyPromises = [];

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
service.config = Object.assign(service.config, service.loadConfig(dir));

module.exports = service;
