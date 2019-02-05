/* A child process used by some tests.
 *
 * Command line options:
 *
 *  inherited-pipe    File descriptor 3 will be read from and written to. This tests the pipe inheritance.
 *
 *  named-pipe PIPE   Open the named pipe "PIPE" and send some information about the process is sent on it. This tests
 *                    starting a client.
 *
 *  mutex MUTEX       Creates a mutex, and releases it after 10 seconds. Used to detect if this process has been run.
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

var logFile = null;
var fs = require("fs");

function log() {
    var items = Array.from(arguments);
    if (logFile) {
        fs.appendFileSync(logFile, items.join(" ") + "\n");
    } else {
        console.log.apply(console, items);
    }
}

function fail() {
    var args = Array.from(arguments);
    args.unshift("FAIL:");
    log.apply(log, args);
    process.exit(1);
}

process.on("uncaughtException", function (e) {
    fail(e);
    process.exit(1);
});

log("child started");

function setEvent(eventHandle) {
    var ffi = require("ffi-napi");
    var kernel32 = ffi.Library("kernel32", {
        "SetEvent": [
            "int", [ "uint" ]
        ]
    });
    log("Calling SetEvent:", eventHandle);
    var ret = kernel32.SetEvent(eventHandle);
    log("SetEvent returned ", ret);
    return ret;
}

var actions = {
    /**
     * For the gpii-ipc.startProcess test: Read to and from a pipe.
     */
    "inherited-pipe": function () {
        // Standard output isn't available to this process, so write output to a file.
        logFile =  process.argv[3];
        log("child started");

        var net = require("net");

        // Get the pipe name.
        var pipeId = process.env.GPII_SERVICE_PIPE;
        if (!pipeId) {
            fail("GPII_SERVICE_PIPE not set.");
            process.exit(1);
        } else {
            log("GPII_SERVICE_PIPE:", pipeId);
        }

        // expecting pipe:id
        var prefix = "pipe:";
        if (!pipeId.startsWith(prefix)) {
            fail("GPII_SERVICE_PIPE is badly formed");
        }

        var pipeName = "\\\\.\\pipe\\gpii-" + pipeId.substr(prefix.length);

        var pipe = net.connect(pipeName, function () {
            log("client: connected to server");

            var authenticated = false;

            var allData = "";
            pipe.on("data", function (data) {
                allData += data;
                if (allData.indexOf("\n") >= 0) {
                    log("Got data:", allData);
                    if (authenticated) {
                        // Echo what was received.
                        pipe.write("received: " + allData);
                    } else {
                        var match = allData.match(/^challenge:([0-9]+)\n$/);
                        if (!match || !match[1]) {
                            fail("Invalid authentication challenge: '" + allData + "'");
                        }
                        var eventHandle = parseInt(match[1]);
                        setEvent(eventHandle);
                        authenticated = true;
                    }
                    allData = "";
                }
            });
        });
        pipe.on("error", function (err) {
            if (err.code === "EOF") {
                process.nextTick(process.exit);
            } else {
                log("input error", err);
                throw err;
            }
        });
    },

    "validate-client": function () {
        var eventHandle = parseInt(process.argv[3]);
        setEvent(eventHandle);
    },
    /**
     * For the gpii-ipc.execute test: send some information to the pipe named on the command line.
     */
    "named-pipe": function () {
        var net = require("net");

        var info = {
            env: process.env,
            currentDir: process.cwd()
        };

        var pipeName = process.argv[3];
        var connection = net.createConnection(pipeName, function () {
            log("connected");
            connection.write(JSON.stringify(info));
            connection.end();
        });
    },

    /**
     * For the process-monitor.startProcess test: Create a mutex. The parent will check if this mutex exists in order
     * to determine this process has been started.
     */
    "mutex": function () {
        var winapi = require("../src/winapi.js");
        var mutexName = winapi.stringToWideChar(process.argv[3]);
        var mutex = null;

        // Release the mutex and die after 30 seconds.
        setTimeout(function () {
            if (mutex) {
                winapi.kernel32.ReleaseMutex(mutex);
                winapi.kernel32.CloseHandle(mutex);
            }
        }, 30000);

        mutex = winapi.kernel32.CreateMutexW(winapi.NULL, true, mutexName);
        log("mutex", winapi.stringFromWideChar(mutexName), mutex);

    }
};

var option = process.argv[2];
if (actions[option]) {
    actions[option]();
} else {
    log("Unrecognised command line");
    process.exit(1);
}
