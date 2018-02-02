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

process.on("uncaughtException", function (e) {
    setTimeout(process.exit, 3000);
    console.error(e);
});

console.log("child started");

var actions = {
    /**
     * For the gpii-ipc.startProcess test: Read to and from an inherited pipe (FD 3)
     */
    "inherited-pipe": function () {
        var net = require("net");

        // A pipe should be at FD 3.
        var pipeFD = 3;
        var pipe = new net.Socket({fd: pipeFD});

        pipe.write("FROM CHILD\n");

        var allData = "";
        pipe.on("data", function (data) {
            allData += data;
            if (allData.indexOf("\n") >= 0) {
                console.log("client got: ", allData);
                pipe.write("received: " + allData);
            }
        });

        pipe.on("error", function (err) {
            if (err.code === "EOF") {
                process.nextTick(process.exit);
            } else {
                console.log("input error", err);
                throw err;
            }
        });
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
            console.log("connected");
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

        // Release the mutex and die after 10 seconds.
        setTimeout(function () {
            if (mutex) {
                winapi.kernel32.ReleaseMutex(mutex);
                winapi.kernel32.CloseHandle(mutex);
            }
        }, 10000);

        mutex = winapi.kernel32.CreateMutexW(winapi.NULL, true, mutexName);
        console.log("mutex", winapi.stringFromWideChar(mutexName), mutex);

    }
};

var option = process.argv[2];
if (actions[option]) {
    actions[option]();
} else {
    console.error("Unrecognised command line");
    process.exit(1);
}
