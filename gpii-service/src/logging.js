/* Logging.
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

var fs = require("fs");

var logging = {};

logging.logFile = null;

logging.levels = {
    "FATAL": 0,
    "ERROR": 10,
    "IMPORTANT": 10,
    "WARN": 20,
    "INFO": 30,
    "DEBUG": 40
};

logging.setLogLevel = function (newLevel) {
    var level = newLevel || logging.defaultLevel;
    if (!level.isLevel) {
        level = level.toString().toUpperCase();
        if (logging.levels.hasOwnProperty(level)) {
            level = logging.levels[level];
        } else {
            logging.error("Unknown log level: " + newLevel);
            level = logging.defaultLevel;
        }
    }

    logging.logLevel = level;
    logging.log(logging.logLevel, "Log level set: " + logging.logLevel.name);
};

/**
 * Log something.
 */
logging.log = function () {
    var args = logging.argsArray(arguments);

    var level = (args[0] && args[0].isLevel)
        ? args.shift()
        : logging.defaultLevel;

    logging.doLog(level, args);
};

logging.doLog = function (level, args) {
    if (!level || !level.isLevel) {
        level = logging.defaultLevel;
    }

    if (level.value <= logging.logLevel.value) {

        var timestamp = new Date().toISOString();
        args.unshift(timestamp, level.name);
        if (logging.logFile) {
            // Serialise any objects in the arguments.
            var text = args.map(function (arg) {
                var argOut;
                var type = typeof(arg);
                var isPrimitive = !arg || type === "string" || type === "number" || type === "boolean";
                if (isPrimitive) {
                    argOut = arg;
                } else {
                    var obj;
                    argOut = JSON.stringify(obj || arg, function (key, value) {
                        if (value instanceof Error) {
                            // Error doesn't serialise - make it a normal object.
                            obj = {};
                            Object.getOwnPropertyNames(value).forEach(function (a) {
                                obj[a] = value[a];
                            });
                            return obj;
                        } else {
                            return value;
                        }
                    });
                }
                return argOut;
            }).join(" ");

            fs.appendFileSync(logging.logFile, text + "\n");
        } else {
            console.log.apply(console, args);
        }
    }
};

/**
 * Sets the log file that stdout/stderr is sent to.
 *
 * @param {String} file The file to log to.
 */
logging.setFile = function (file) {
    logging.logFile = file;

    // Capture and log writes to standard out and err.
    var write = function (chunk, encoding, done) {
        // This assumes a whole line is in one chunk. If not, then the parts of line will be on different log entries.
        logging.log(chunk.toString().trim());
        done && done();
    };

    process.stdout._write = process.stderr._write = write;
};

/**
 * Converts an arguments object into an array.
 * @param {Object} args An arguments object, or an array.
 * @return {Array} The args object as an array.
 */
logging.argsArray = function (args) {
    var togo;
    if (Array.isArray(args)) {
        togo = args;
    } else {
        togo = [];
        for (var n = 0; n < args.length; n++) {
            togo.push(args[n]);
        }
    }

    return togo;
};

/**
 * Returns a function that logs to the given log level.
 * @param {Object} level A member of logging.levels identifying the log level.
 * @return {Function} A function that logs at the given level.
 */
logging.createLogFunction = function (level) {
    return function () {
        logging.doLog(level, logging.argsArray(arguments));
    };
};

for (var level in logging.levels) {
    if (logging.levels.hasOwnProperty(level)) {
        // Create a level object.
        var levelObj = {
            isLevel: true,
            value: logging.levels[level],
            name: level
        };
        // Add a convenience function for that level.
        logging[level.toLowerCase()] = logging.createLogFunction(levelObj);
        logging.levels[level] = levelObj;
    }
}

// Default level for Log entries when unspecified.
logging.defaultLevel = logging.levels.INFO;
// The current logging level
logging.logLevel = logging.defaultLevel;

/** @name logging.fatal
 *  @function
 */
/** @name logging.error
 *  @function
 */
/** @name logging.important
 *  @function
 */
/** @name logging.warn
 *  @function
 */
/** @name logging.info
 *  @function
 */
/** @name logging.debug
 *  @function
 */
module.exports = logging;
