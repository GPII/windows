/* Bootstrap for the GPII windows service.
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
    fs = require("fs"),
    path = require("path"),
    logging = require("./src/logging.js"),
    parseArgs = require("minimist");

var args = parseArgs(process.argv.slice(2));

if (args.help) {
    showUsage();
} else if (args.install) {
    install();
} else if (args.uninstall) {
    uninstall();
} else if (!args.service && process.versions.pkg) {
    // If running as an executable, then insist upon the --service flag.
    console.error("Invalid command line.");
    showUsage();
} else {
    // Start the service
    startService();
}

/**
 * Print the accepted command-line arguments.
 */
function showUsage() {
    console.log("GPII Windows Service.\n");
    console.log("Command line options:");
    console.log(" --install     Install the Windows Service.");
    console.log(" --serviceArgs=ARGS");
    console.log("         Comma separated arguments to pass to the service (use with --install).");
    console.log(" --uninstall   Uninstall the Windows Service.");
    console.log(" --service     Only used when running as a service.");
    console.log(" --config=FILE Specify the config file to use (default: service.json5).");
}

/**
 * Install the service. This needs to be ran as Administrator.
 *
 * It reads the following (optional) arguments from the command line:
 *  --nodeArgs=ARGS    Comma separated list of arguments to pass to the node, when running the service.
 *  --serviceArgs=ARGS Comma separated list of arguments to pass to the service.
 *  --config=FILE      Config file for the service to use.
 *  --loglevel=LEVEL   Log level for the service.
 *
 */
function install() {

    var serviceName = args.serviceName || "morphic-service";

    var serviceArgs = [ "--service" ];

    if (args.serviceArgs) {
        serviceArgs.push.apply(serviceArgs, args.serviceArgs.split(/,+/));
    }

    // Forward some arguments to the service.
    var forwardArgs = ["config", "loglevel"];
    forwardArgs.forEach(function (argName) {
        if (args.hasOwnProperty(argName)) {
            var value = args[argName];
            if (value === true) {
                serviceArgs.push("--" + argName);
            } else {
                serviceArgs.push("--" + argName + "=" + value);
            }
        }
    });

    var nodeArgs = args.nodeArgs && args.nodeArgs.split(/,+/);

    console.log("Installing");

    os_service.add(serviceName, {
        programArgs: serviceArgs,
        nodeArgs: nodeArgs,
        displayName: "Morphic Service"
    }, function (error) {
        if (error) {
            console.log(error.message);
        } else {
            console.log("Success");
        }
    });
}

/**
 * Removes the service. This needs to be ran as Administrator, and the service should be already stopped.
 *
 * It reads the following arguments from the command line:
 *  --serviceName NAME   Name of the Windows Service (default: morphic-service).
 */
function uninstall() {
    var serviceName = args.serviceName || "morphic-service";

    console.log("Uninstalling");
    os_service.remove(serviceName, function (error) {
        if (error) {
            console.log(error.message);
        } else {
            console.log("Success");
        }
    });
}

function startService() {
    var dataDir = path.join(process.env.ProgramData, "Morphic");

    try {
        fs.mkdirSync(dataDir);
    } catch (e) {
        if (e.code !== "EEXIST") {
            throw e;
        }
    }

    if (args.service) {
        // Set up the logging early - there's no way to capture stdout for windows services.
        var logFile = path.join(dataDir, "morphic-service.log");
        logging.setFile(logFile);
    }
    if (args.loglevel) {
        logging.setLogLevel(args.loglevel);
    }

    process.on("uncaughtException", function (err) {
        if (!args.service) {
            console.error(err);
        }
        logging.error(err, (err && err.stack) ? err.stack : err);
    });

    // Start the service
    if (args.service) {
        logging.log("Starting service");
        os_service.run();
        logging.log("Service initialising");
    }
    require("./src/main.js");
}
