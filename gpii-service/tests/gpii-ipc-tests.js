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
    fs = require("fs"),
    os = require("os"),
    EventEmitter = require("events"),
    child_process = require("child_process"),
    ipc = require("../src/gpii-ipc.js"),
    windows = require("../src/windows.js"),
    winapi = require("../src/winapi.js");

var teardowns = [];

jqUnit.module("GPII service ipc tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

var createLogFile = function () {
    // The child process will write to this file (stdout isn't captured)
    var logFile = os.tmpdir() + "/gpii-test-output" + Date.now();
    console.log("logfile:", logFile);
    fs.writeFileSync(logFile, "");
    teardowns.push(function () {
        try {
            fs.unlinkSync(logFile);
        } catch (e) {
            // Ignored
        }
    });
    return logFile;
};

// Tests generatePipeName
jqUnit.test("Test generatePipeName", function () {
    var pipePrefix = "\\\\.\\pipe\\";

    // Because the names are random, check against a sample of them to avoid lucky results.
    var sampleSize = 300;
    var pipeNames = [];
    for (var n = 0; n < sampleSize; n++) {
        pipeNames.push(ipc.generatePipeName());
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

jqUnit.asyncTest("Test createPipe", function () {
    jqUnit.expect(4);

    var pipeName = ipc.generatePipeName();

    var promise = ipc.createPipe(pipeName);
    jqUnit.assertNotNull("createPipe must return non-null", promise);
    jqUnit.assertEquals("createPipe must return a promise", "function", typeof(promise.then));

    promise.then(function (pipeServer) {
        jqUnit.assertTrue("createPipe should have resolved with a value", !!pipeServer);

        jqUnit.assertTrue("createPipe resolved value should be a net.Server instance",
            pipeServer instanceof net.Server);

        jqUnit.start();
    }, function (err) {
        console.error(err);
        jqUnit.fail("createPipe should have resolved");
    });
});

jqUnit.asyncTest("Test createPipe failures", function () {

    var existingPipe = ipc.generatePipeName();

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
        fluid.log("Checking bad pipe name:", pipeName);

        fluid.log("Error is expected:");
        var promise = ipc.createPipe(pipeName);
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
    ipc.createPipe(existingPipe).then(function () {
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
 * @param {String} pipeName Pipe name.
 * @param {Function} callback What to call.
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

// Tests the execution of a child process with ipc.execute
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

    // Create a pipe so the child process can talk back (ipc.execute doesn't capture the child's stdout).
    var pipeName = ipc.generatePipeName();
    readPipe(pipeName, checkReturn);

    var options = Object.assign({}, testData.execOptions);

    // Two asserts for each environment value.
    jqUnit.expect(Object.keys(options.env).length * 2);

    // Start the child process, passing the pipe name.
    var script = path.join(__dirname, "gpii-ipc-tests-child.js");
    var command = ["node", script, "named-pipe", pipeName].join(" ");
    console.log("Executing", command);
    var pid = ipc.execute(command, options);

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

// Tests validateClient
jqUnit.asyncTest("Test validateClient", function () {

    var tests = [
        {
            // SetEvent is called by this process.
            id: "successful",
            startChildProcess: false,
            setEvent: true,
            timeout: 20,
            expect: {
                end: false,
                resolve: true
            }
        },
        {
            // SetEvent is not called.
            id: "timeout",
            startChildProcess: false,
            setEvent: false,
            timeout: 2,
            expect: {
                end: true,
                resolve: false
            }
        },
        {
            // SetEvent is called by another process (that hasn't been granted the handle).
            id: "other-process",
            startChildProcess: true,
            setEvent: false,
            timeout: 2,
            expect: {
                end: true,
                resolve: false
            }
        }
    ];

    var runTest = function (testIndex) {
        if (testIndex >= tests.length) {
            jqUnit.start();
            return;
        }

        var test = tests[testIndex];
        var endCalled = false;

        // A mock pipe.
        var pipe = new EventEmitter();
        pipe.write = function (data) {
            var challenge = data.substr("challenge:".length).trim();
            jqUnit.assertFalse("challenge must be numeric", isNaN(challenge));

            if (test.startChildProcess) {
                var script = path.join(__dirname, "gpii-ipc-tests-child.js");
                var command = ["node", script, "validate-client", challenge].join(" ");
                console.log("starting", command);
                child_process.exec(command, { shell: false}, function (err, stdout, stderr) {
                    console.log("child stdout:", stdout);
                    console.log("child stderr:", stderr);
                    if (err) {
                        jqUnit.fail(err);
                    } else {
                        jqUnit.assertTrue("child should have called SetEvent",
                            stdout.indexOf("SetEvent returned") > -1);
                    }
                });

            }

            if (test.setEvent) {
                var eventHandle = parseInt(challenge);
                winapi.kernel32.SetEvent(eventHandle);
            }
        };
        pipe.end = function () {
            endCalled = true;
        };

        var pid = process.pid;
        ipc.validateClient(pipe, pid, test.timeout).then(function () {
            jqUnit.assertTrue("validateClient resolved", test.expect.resolve);
        }, function (e) {
            console.log(e);
            jqUnit.assertFalse("validateClient rejected", test.expect.resolve);
        }).then(function () {
            jqUnit.assertEquals("end should be called (if expected)", test.expect.end, endCalled);
            runTest(testIndex + 1);
        });
    };

    runTest(0);
});

// Tests startProcess - creates a child process using startProcess.
jqUnit.asyncTest("Test startProcess", function () {
    var logFile = createLogFile();
    var getLog = function () {
        try {
            return fs.readFileSync(logFile, {encoding: "utf8"});
        } catch (e) {
            // ignore
            return "";
        }
    };

    var script = path.join(__dirname, "gpii-ipc-tests-child.js");
    var command = ["node", script, "inherited-pipe", logFile].join(" ");
    console.log("Starting", command);

    // 0: server sends challenge, client performs it, server sends "OK"
    // 1: child sends expected[0]
    // 2: parent sends sendData
    // 3: child sends expected[1]
    var sendData = "FROM PARENT" + Math.random();
    var expected = [
        // Child responds with what was sent (from the authentication).
        "received: OK\n",
        // Child responds with what was sent (after authentication).
        "received: " + sendData + "\n"
    ];

    var expectedIndex = 0;
    var complete = false;
    var allData = "";
    var pid = null;

    var promise = ipc.startProcess(command, "test-startProcess");

    jqUnit.assertNotNull("startProcess must return non-null", promise);
    jqUnit.assertEquals("startProcess must return a promise", "function", typeof(promise.then));

    promise.then(function (p) {
        jqUnit.assertNotNull("startProcess must return resolve with a value", p);
        jqUnit.assertFalse("startProcess resolved value pid field must be numeric", isNaN(p.pid));
        jqUnit.assertTrue("startProcess resolved value pipeServer field must be a net.Server instance",
            p.pipeServer instanceof net.Server);

        pid = p.pid;

        // Set a timeout.
        windows.waitForProcessTermination(pid, 5000).then(function (value) {
            var logContent = getLog();
            if (value === "timeout") {
                console.log(logContent);
                jqUnit.fail("Child process took too long.");
            } else {
                if (!complete) {
                    console.log(logContent);
                    jqUnit.fail("child should not terminate until completed");
                } else if (logContent.indexOf("FAIL") >= 0) {
                    console.log(logContent);
                    jqUnit.fail("Child process failed");
                }
            }
        }, function (err) {
            console.error(err);
            jqUnit.fail("Unable to wait for child process.");
        });

        p.pipeServer.on("error", function (err) {
            console.error("Pipe server error:", err);
            jqUnit.fail("Pipe server failed");
        });

        // Wait for the child to connect to the pipe
        p.pipeServer.on("connection", function (pipe) {
            console.log("connection from child");

            pipe.on("data", function (data) {
                allData += data;
                if (allData.indexOf("\n") >= 0) {
                    console.log("from pipe:", allData);
                    jqUnit.assertEquals("Expected input from pipe", expected[expectedIndex], allData);
                    allData = "";
                    expectedIndex++;
                    if (expectedIndex >= expected.length) {
                        complete = true;
                        pipe.end();
                    } else {
                        console.log("send: " + sendData);
                        pipe.write(sendData + "\n");
                    }
                }
            });

            pipe.on("end", function () {
                console.log("pipe end");
                if (complete) {
                    jqUnit.start();
                } else {
                    console.log(getLog());
                    jqUnit.fail("child should not terminate until completed");
                }
            });

            pipe.on("error", function (err) {
                console.error("Pipe error:", err);
                jqUnit.fail("Pipe failed.");
            });
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
