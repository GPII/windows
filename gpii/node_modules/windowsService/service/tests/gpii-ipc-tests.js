/* Tests for gpii-ipc.js
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

var jqUnit = require("node-jqunit"),
    net = require("net"),
    path = require("path"),
    gpiiIPC = require("../src/gpii-ipc.js"),
    windows = require("../src/windows.js"),
    winapi = require("../src/winapi.js");

var teardowns = [];

jqUnit.module("GPII pipe tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

// Tests generatePipeName
jqUnit.test("Test generatePipeName", function () {
    var pipePrefix = "\\\\.\\pipe\\";

    // Because the names are random, check against a sample of them to avoid lucky results.
    var sampleSize = 300;
    var pipeNames = [];
    for (var n = 0; n < sampleSize; n++) {
        pipeNames.push(gpiiIPC.generatePipeName());
    }

    for (var pipeIndex = 0; pipeIndex < sampleSize; pipeIndex++) {
        var fullName = pipeNames[pipeIndex];

        // Pipe Names: https://msdn.microsoft.com/library/aa365783
        jqUnit.assertTrue("Pipe path must begin with " + pipePrefix, fullName.startsWith(pipePrefix));
        jqUnit.assertTrue("Entire pipe name must <= 256 characters", fullName.length <= 256);

        var pipeName = fullName.substr(pipePrefix.length);
        jqUnit.assertTrue("Pipe name must at least 1 character", pipeName.length > 0);
        // "any character other than a backslash, including numbers and special characters"
        // This also includes '/' because node swaps it with '\'.
        jqUnit.assertFalse("Pipe name must not contain a slash or blackslash", pipeName.match(/[\\\/]/));

        // There shouldn't be any repeated names in a sample size of this size.
        var dup = pipeNames.indexOf(fullName) !== pipeIndex;
        jqUnit.assertFalse("There shouldn't be any repeated pipe names", dup);
    }
});

// Tests a successful connectToPipe.
jqUnit.asyncTest("Test connectToPipe", function () {
    jqUnit.expect(6);

    var pipeName = gpiiIPC.generatePipeName();

    // The invocation order of the callbacks for client or server connection varies.
    var serverConnected = false,
        clientConnected = false;
    var connected = function () {
        if (serverConnected && clientConnected) {
            jqUnit.start();
        }
    };

    // Create a server to listen for the connection.
    var server = net.createServer();
    server.on("connection", function () {
        jqUnit.assert("Got connection");
        serverConnected = true;
        connected();
    });

    server.listen(pipeName, function () {
        var promise = gpiiIPC.connectToPipe(pipeName);

        jqUnit.assertNotNull("connectToPipe must return non-null", promise);
        jqUnit.assertEquals("connectToPipe must return a promise", "function", typeof(promise.then));

        promise.then(function (pipeHandle) {
            jqUnit.assert("connectToPipe promise resolved (connection worked)");
            jqUnit.assertTrue("pipeHandle must be something", !!pipeHandle);
            jqUnit.assertFalse("pipeHandle must be a number", isNaN(pipeHandle));
            clientConnected = true;
            connected();
        });
    });
});

// Make connectToPipe fail.
jqUnit.asyncTest("Test connectToPipe failures", function () {

    var pipeNames = [
        // A pipe that doesn't exist.
        gpiiIPC.generatePipeName(),
        // A pipe with a bad name.
        gpiiIPC.generatePipeName() + "\\",
        // Badly formed name
        "invalid",
        null
    ];

    jqUnit.expect(pipeNames.length * 3);

    var testPipes = function (pipeNames) {
        var pipeName = pipeNames.shift();
        console.log("Checking bad pipe name:", pipeName);
        var promise = gpiiIPC.connectToPipe(pipeName);
        jqUnit.assertNotNull("connectToPipe must return non-null", promise);
        jqUnit.assertEquals("connectToPipe must return a promise", "function", typeof(promise.then));

        promise.then(function () {
            jqUnit.fail("connectToPipe promise resolved (connection should not have worked)");
        }, function () {
            jqUnit.assert("connectToPipe promise should reject");

            if (pipeNames.length > 0) {
                testPipes(pipeNames);
            } else {
                jqUnit.start();
            }
        });
    };

    testPipes(Array.from(pipeNames));
});

jqUnit.asyncTest("Test createPipe", function () {
    jqUnit.expect(8);

    var pipeName = gpiiIPC.generatePipeName();

    var promise = gpiiIPC.createPipe(pipeName);
    jqUnit.assertNotNull("createPipe must return non-null", promise);
    jqUnit.assertEquals("createPipe must return a promise", "function", typeof(promise.then));

    promise.then(function (pipePair) {
        jqUnit.assertTrue("createPipe should have resolved with a value", !!pipePair);

        jqUnit.assertTrue("serverConnection should be set", !!pipePair.serverConnection);
        jqUnit.assertTrue("clientHandle should be set", !!pipePair.clientHandle);

        jqUnit.assertTrue("serverConnection should be a Socket", pipePair.serverConnection instanceof net.Socket);
        jqUnit.assertFalse("clientHandle should be a number", isNaN(pipePair.clientHandle));
        jqUnit.assertNotEquals("clientHandle should be a valid handle",
            pipePair.clientHandle, winapi.constants.INVALID_HANDLE_VALUE);

        jqUnit.start();
    }, function (err) {
        console.error(err);
        jqUnit.fail("createPipe should have resolved");
    });
});

jqUnit.asyncTest("Test createPipe failures", function () {

    var existingPipe = gpiiIPC.generatePipeName();

    var pipeNames = [
        // A pipe that exists.
        existingPipe,
        // Badly formed name
        "invalid",
        null
    ];

    jqUnit.expect(pipeNames.length * 3);

    var testPipes = function (pipeNames) {
        var pipeName = pipeNames.shift();
        console.log("Checking bad pipe name:", pipeName);

        var promise = gpiiIPC.createPipe(pipeName);
        jqUnit.assertNotNull("createPipe must return non-null", promise);
        jqUnit.assertEquals("createPipe must return a promise", "function", typeof(promise.then));

        promise.then(function () {
            jqUnit.fail("createPipe should not have resolved");
        }, function () {
            jqUnit.assert("createPipe should reject");

            if (pipeNames.length > 0) {
                testPipes(pipeNames);
            } else {
                jqUnit.start();
            }
        });
    };

    // Create a pipe to see what happens if another pipe is created with the same name.
    gpiiIPC.createPipe(existingPipe).then(function () {
        // run the tests.
        testPipes(Array.from(pipeNames));
    }, function (err) {
        console.error(err);
        jqUnit.fail("initial createPipe failed");
    });

});

/**
 * Read from a pipe, calling callback with all the data when it ends.
 *
 * @param pipeName {String} Pipe name.
 * @param callback {Function(err,data)} What to call.
 */
function readPipe(pipeName, callback) {
    var buffer = "";
    var server = net.createServer(function (con) {
        con.setEncoding("utf8");
        buffer = "";

        con.on("error", function (err) {
            console.error(err);
            callback(err);
        });
        con.on("data", function (data) {
            buffer += data;
        });
        con.on("end", function () {
            callback(null, buffer);
        });
    });
    server.listen(pipeName);
    server.on("error", function (err) {
        console.error(err);
        jqUnit.fail("Error with the pipe server");
    });
}

// Tests the execution of a child process with gpiiIPC.execute (file handle inheritance is not tested here).
jqUnit.asyncTest("Test execute", function () {

    jqUnit.expect(4);

    var testData = {
        execOptions: {
            env: {
                "GPII_TEST_VALUE1": "value1",
                "GPII_TEST_VALUE2": "value2"
            },
            currentDir: process.env.APPDATA
        }
    };

    // Create a pipe so the child process can talk back (gpiiIPC.execute doesn't capture the child's stdout).
    var pipeName = gpiiIPC.generatePipeName();
    readPipe(pipeName, checkReturn);

    var options = Object.assign({}, testData.execOptions);

    // Two asserts for each environment value.
    jqUnit.expect(Object.keys(options.env).length * 2);

    // Start the child process, passing the pipe name.
    var script = path.join(__dirname, "gpii-ipc-tests-child.js");
    var command = ["node", script, "named-pipe", pipeName].join(" ");
    console.log("Executing", command);
    var pid = gpiiIPC.execute(command, options);

    jqUnit.assertEquals("execute should return a number", "number", typeof(pid));

    function checkReturn(err, data) {
        if (err) {
            console.error(err);
            jqUnit.fail("The was something wrong with the pipe");
        } else {
            var obj;

            try {
                obj = JSON.parse(data);
            } catch (e) {
                console.log("child returned: ", data);
                throw e;
            }

            jqUnit.assertTrue("child process should return something", !!obj);
            if (obj.error) {
                console.log("Error from child:", obj.error);
                jqUnit.fail("child process returned an error");
                return;
            }
            jqUnit.assertEquals("'currentDir' should return from child", options.currentDir, obj.currentDir);
            jqUnit.assertTrue("'env' should return from child", !!obj.env);

            for (var key in options.env) {
                var value = options.env[key];
                jqUnit.assertTrue("Environment should contain " + key, key, obj.hasOwnProperty(key));
                jqUnit.assertEquals("Environment '" + key + "' should be the expected value", value, obj.env[key]);
            }

            jqUnit.start();
        }
    }

    teardowns.push(function () {
        try {
            process.kill(pid);
        } catch (e) {
            // Ignored.
        }
    });

    // Set a timeout.
    windows.waitForProcessTermination(pid, 5000).then(function (value) {
        if (value === "timeout") {
            jqUnit.fail("Child process took too long.");
        }
    }, function (err) {
        console.error(err);
        jqUnit.fail("Unable to wait for child process.");
    });

});

// Tests startProcess - creates a child process with an inherited open pipe.
jqUnit.asyncTest("Test startProcess", function () {
    var script = path.join(__dirname, "gpii-ipc-tests-child.js");
    var command = ["node", script, "inherited-pipe"].join(" ");
    console.log("Starting", command);

    var sendData = "FROM PARENT";
    var expected = [
        // Test reading, child sends this first.
        "FROM CHILD\n",
        // Test writing; child responds with what was sent.
        "received: " + sendData + "\n"
    ];

    var expectedIndex = 0;

    var allData = "";
    var pid = null;

    gpiiIPC.startProcess(command).then(function (p) {
        pid = p.pid;

        // Set a timeout.
        windows.waitForProcessTermination(pid, 5000).then(function (value) {
            if (value === "timeout") {
                jqUnit.fail("Child process took too long.");
            }
        }, function (err) {
            console.error(err);
            jqUnit.fail("Unable to wait for child process.");
        });

        p.pipe.setEncoding("utf8");
        p.pipe.on("data", function (data) {
            allData += data;
            if (allData.indexOf("\n") >= 0) {
                console.log("from pipe:", allData);
                jqUnit.assertEquals("Expected input from pipe", expected[expectedIndex], allData);
                allData = "";
                expectedIndex++;
                if (expectedIndex >= expected.length) {
                    p.pipe.end();
                } else {
                    p.pipe.write(sendData + "\n");
                }
            }
        });

        p.pipe.on("end", function () {
            jqUnit.start();
        });

        p.pipe.on("error", function (err) {
            console.error("Pipe error:", err);
            jqUnit.fail("Pipe failed.");
        });

    }, jqUnit.fail);

    teardowns.push(function () {
        try {
            process.kill(pid);
        } catch (e) {
            // Ignored.
        }
    });
});
