/* Tests for gpii-process.js
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
    path = require("path"),
    child_process = require("child_process"),
    processHandling = require("../src/processHandling.js"),
    windows = require("../src/windows.js"),
    winapi = require("../src/winapi.js");

var processHandlingTests = {
    testData: {}
};
var teardowns = [];

jqUnit.module("GPII Service processHandling tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

processHandlingTests.testData.startChildProcess = [
    // Shouldn't restart if stopChildProcess is used.
    {
        input: {
            childProcess: {
                key: "norestart-stop-test",
                command: "test-command",
                ipc: null,
                autoRestart: false
            },
            stopChildProcess: false
        },
        expect: {
            restart: false
        }
    },
    {
        input: {
            childProcess: {
                key: "restart-stop-test",
                command: "test-command",
                ipc: null,
                autoRestart: true
            },
            stopChildProcess: true
        },
        expect: {
            restart: false
        }
    },
    {
        input: {
            childProcess: {
                key: "norestart-ipc-stop-test",
                command: "test-command",
                ipc: "norestart",
                autoRestart: false
            },
            stopChildProcess: true
        },
        expect: {
            restart: false
        }
    },
    {
        input: {
            childProcess: {
                key: "restart-ipc-stop-test",
                command: "test-command",
                ipc: "restart",
                autoRestart: true
            },
            stopChildProcess: true
        },
        expect: {
            restart: false
        }
    },
    // Should restart if killed, and autoRestart is set
    {
        input: {
            childProcess: {
                key: "norestart-kill-test",
                command: "test-command",
                ipc: null,
                autoRestart: false
            },
            stopChildProcess: false
        },
        expect: {
            restart: false
        }
    },
    {
        input: {
            childProcess: {
                key: "restart-kill-test",
                command: "test-command",
                ipc: null,
                autoRestart: true
            },
            stopChildProcess: false
        },
        expect: {
            restart: true
        }
    },
    {
        input: {
            childProcess: {
                key: "norestart-ipc-kill-test",
                command: "test-command",
                ipc: "norestart",
                autoRestart: false
            },
            stopChildProcess: false
        },
        expect: {
            restart: false
        }
    },
    {
        input: {
            childProcess: {
                key: "restart-ipc-kill-test",
                command: "test-command",
                ipc: "restart",
                autoRestart: true
            },
            stopChildProcess: false
        },
        expect: {
            restart: true
        }
    }
];

processHandlingTests.testData.monitorProcessFailures = [
    { input: null },
    { input: 0  },
    { input: -1 }
];

/**
 * Start a process that self terminates after 10 seconds.
 * @return {ChildProcess} The child process.
 */
processHandlingTests.startProcess = function () {
    var id = "processHandlingTest" + Math.random().toString(32).substr(2);
    var exe = path.join(process.env.SystemRoot, "/System32/waitfor.exe");
    var command = exe + " " + id + " /T 10 ";
    return child_process.exec(command);
};

/**
 * Waits for a mutex to be create, by polling until OpenMutex succeeds.
 *
 * @param {String} mutexName The name of the mutex.
 * @param {Number} timeout [Optional] How long to wait (ms), default 1000.
 * @return {Promise} Resolves when a mutex with the given name has been created, or with value of "timeout".
 */
processHandlingTests.waitForMutex = function (mutexName, timeout) {
    return new Promise(function (resolve, reject) {
        var nameBuffer = winapi.stringToWideChar(mutexName);

        var timedout = false;
        setTimeout(function () {
            timedout = true;
            resolve("timeout");
        }, timeout || 1000);

        // Poll until OpenMutex succeeds.
        var checkMutex = function () {
            var mutexHandle = winapi.kernel32.OpenMutexW(winapi.constants.SYNCHRONIZE, false, nameBuffer);

            if (mutexHandle) {
                winapi.kernel32.ReleaseMutex(mutexHandle);
                winapi.kernel32.CloseHandle(mutexHandle);
                resolve();
            } else if (!timedout) {
                var err = winapi.kernel32.GetLastError();
                var ERROR_FILE_NOT_FOUND = 2;
                if (err === ERROR_FILE_NOT_FOUND) {
                    setTimeout(checkMutex, 200);
                } else {
                    reject(winapi.error("OpenMutex", 0, err));
                }
            }
        };

        checkMutex();
    });
};

// Tests getProcessCreationTime
jqUnit.test("Test getProcessCreationTime", function () {
    var value;

    // This process
    var thisProcess = processHandling.getProcessCreationTime(process.pid);
    jqUnit.assertNotNull("creation time must not be null", thisProcess);
    jqUnit.assertFalse("creation time must be a number", isNaN(thisProcess));

    value = processHandling.getProcessCreationTime(process.pid);
    jqUnit.assertEquals("two calls must return the same value", thisProcess, value);

    // A different process.
    var child = processHandlingTests.startProcess();
    value = processHandling.getProcessCreationTime(child.pid);
    jqUnit.assertNotNull("creation time of child process must not be null", value);
    jqUnit.assertNotNull("creation time of child process must be different to this process", thisProcess);

    // A process that's not running.
    value = processHandling.getProcessCreationTime(1);
    jqUnit.assertNull("creation time for non-running process must be null", value);

    // A pid that's not a pid
    value = processHandling.getProcessCreationTime("not a pid");
    jqUnit.assertNull("creation time for non-pid must be null", value);

});

// Tests isProcessRunning
jqUnit.asyncTest("Test isProcessRunning", function () {
    jqUnit.expect(10);
    var running;

    // This process
    running = processHandling.isProcessRunning(process.pid);
    jqUnit.assertTrue("This process should be running",  running);

    // A process that's not running.
    running = processHandling.isProcessRunning(3);
    jqUnit.assertFalse("This process should not be running", running);

    // A pid that's not a pid
    running = processHandling.isProcessRunning("not a pid");
    jqUnit.assertFalse("This invalid process should not be running", running);

    // A null pid
    running = processHandling.isProcessRunning(null);
    jqUnit.assertFalse("This invalid process should not be running", running);


    var creationTime, pid;

    // This process, with creation time
    pid = process.pid;
    creationTime = processHandling.getProcessCreationTime(pid);
    running = processHandling.isProcessRunning(pid);
    jqUnit.assertTrue("This process (with creation time) should be running", running);

    // This process, with wrong creation time
    pid = process.pid;
    creationTime = "12345";
    running = processHandling.isProcessRunning(pid, creationTime);
    jqUnit.assertFalse("This process (incorrect creation time) should not be running", running);

    // Test a child process, with a handle still open - the pid will still be valid, but it's not a running process.
    // (first tested without the handle opening, for a sanity check)
    var testChild = function (openHandle) {
        var assertSuffix = openHandle ? " (open handle)" : "";
        var child = processHandlingTests.startProcess();
        running = processHandling.isProcessRunning(child.pid);
        jqUnit.assertTrue("Child process should be running" + assertSuffix, running);

        var handle;
        if (openHandle) {
            handle = winapi.kernel32.OpenProcess(winapi.constants.PROCESS_QUERY_LIMITED_INFORMATION, 0, child.pid);
            if (!handle || handle === winapi.NULL) {
                jqUnit.fail(winapi.errorText("OpenProcess failed"));
            }
        }

        child.on("close", function () {
            setTimeout(function () {
                running = processHandling.isProcessRunning(child.pid);
                jqUnit.assertFalse("Child process should not be running after kill" + assertSuffix, running);

                if (openHandle) {
                    winapi.kernel32.CloseHandle(handle);
                    jqUnit.start();
                } else {
                    testChild(true);
                }
            }, 100);
        });
        child.kill();
    };

    testChild(false);
});

// Tests startChildProcess, stopChildProcess, and autoRestartProcess (indirectly)
jqUnit.asyncTest("Test startChildProcess", function () {

    var testData = processHandlingTests.testData.startChildProcess;
    // Don't delay restarting the process.
    var throttleRate_orig = processHandling.throttleRate;
    processHandling.throttleRate = function () {
        return 1;
    };
    teardowns.push(function () {
        processHandling.throttleRate = throttleRate_orig;
    });

    // For each test a child process is started (using the input data). It's then stopped via either stopChildProcess or
    // kill. A check is made if the process has ended, and if the process has been restarted or not.
    // The pid of the new child process is unknown, so a mutex is created by the child then checked here if it exists or
    // not to determine if the new process is running.
    var nextTest = function (testIndex) {
        if (testIndex >= testData.length) {
            jqUnit.start();
            return;
        }
        var test = testData[testIndex];
        var messageSuffix = " - " + test.input.childProcess.key;

        var procConfig = Object.assign({}, test.input.childProcess);
        var mutexName = procConfig.key + Math.random().toString(32);
        procConfig.command = "node.exe " + path.join(__dirname, "gpii-ipc-tests-child.js") + " mutex " + mutexName;

        var promise = processHandling.startChildProcess(procConfig);

        jqUnit.assertTrue("startProcess must return a promise" + messageSuffix,
            promise && typeof(promise.then) === "function");

        promise.then(function (pid) {
            jqUnit.assertFalse("startProcess must resolve with a numeric pid" + messageSuffix, isNaN(pid));
            var pidRunning = processHandling.isProcessRunning(pid);
            jqUnit.assertTrue("startProcess must resolve with a running pid" + messageSuffix, pidRunning);

            // See if the process gets restarted (or not).
            windows.waitForProcessTermination(pid, 1000).then(function (value) {
                if (value === "timeout") {
                    jqUnit.fail("child process didn't terminate" + messageSuffix);
                } else {
                    // See if the new process was restarted, by waiting for the mutex it creates.
                    processHandlingTests.waitForMutex(mutexName, 2000).then(function (value) {
                        if (test.expect.restart) {
                            jqUnit.assertNotEquals("process should restart" + messageSuffix, "timeout", value);
                        } else {
                            jqUnit.assertEquals("process should not restart" + messageSuffix, "timeout", value);
                        }

                        processHandling.stopChildProcess(procConfig.key, false);

                        nextTest(testIndex + 1);
                    }, jqUnit.fail);
                }
            }, jqUnit.fail);

            // Kill the first process.
            if (test.input.stopChildProcess) {
                processHandling.stopChildProcess(procConfig.key, false);
            } else {
                process.kill(pid);
            }

        }, jqUnit.fail);
    };

    nextTest(0);

});

jqUnit.asyncTest("Test monitorProcess - single process", function () {
    jqUnit.expect(3);

    var child = processHandlingTests.startProcess();
    var promise = processHandling.monitorProcess(child.pid);

    jqUnit.assertTrue("monitorProcess must return a promise", promise && typeof(promise.then) === "function");

    var killed = false;
    promise.then(function (value) {
        jqUnit.assertTrue("monitorProcess should not resolve before the process is killed", killed);
        jqUnit.assertEquals("monitorProcess should resolve with the process id", child.pid, value);
        jqUnit.start();
    }, jqUnit.fail);

    setTimeout(function () {
        killed = true;
        child.kill();
    }, 100);

});

jqUnit.asyncTest("Test monitorProcess - multiple processes", function () {

    var killOrder = [ 4, 0, 2, 1, 3 ];
    var procs = [];
    for (var n = 0; n < killOrder.length; n++) {
        procs.push(processHandlingTests.startProcess());
    }

    jqUnit.expect(procs.length * 2);

    var killed = [];
    var killProcess = function () {
        if (killOrder.length > 0) {
            var proc = procs[killOrder.shift()];
            killed.push(proc.pid);
            proc.kill();
        } else {
            jqUnit.start();
        }
    };

    procs.forEach(function (proc) {
        processHandling.monitorProcess(proc.pid).then(function (pid) {
            jqUnit.assertEquals("monitorProcess must resolve with the same pid", proc.pid, pid);
            jqUnit.assertNotEquals("monitorProcess must resolve after the process is killed", -1, killed.indexOf(pid));
            killProcess();
        }, jqUnit.fail);
    });

    killProcess();

});

jqUnit.asyncTest("Test monitorProcess failures", function () {
    var testData = processHandlingTests.testData.monitorProcessFailures;
    jqUnit.expect(testData.length * 4 * 2 + 1);

    var child = processHandlingTests.startProcess();

    setTimeout(jqUnit.fail, 10000, "timeout");

    // Tests are ran twice - the 2nd time, another process is also being monitored. This is to check an innocent process
    // doesn't get caught up in the failure.
    var pass = 0;
    var runTest = function (testIndex) {
        if (testIndex >= testData.length) {
            pass++;
            if (pass > 1) {
                child.kill();
                return;
            } else {
                // Monitor a process to check it doesn't also get rejected.
                processHandling.monitorProcess(child.pid).then(function () {
                    jqUnit.assertTrue("Child shouldn't resolve until the end" + messageSuffix, pass > 1);
                    jqUnit.start();
                }, function () {
                    jqUnit.fail("Child shouldn't fail");
                });
                testIndex = 0;
            }
        }

        var test = testData[testIndex];
        var messageSuffix = " - testIndex=" + testIndex + ", pass=" + pass;
        console.log("Running test" + messageSuffix);

        var promise = processHandling.monitorProcess(test.input, 100, false);

        jqUnit.assertTrue("monitorProcess must return a promise", promise && typeof(promise.then) === "function");

        promise.then(function () {
            jqUnit.fail("monitorProcess should not have resolved" + messageSuffix);
        }, function (e) {
            jqUnit.assert("monitorProcess should have rejected" + messageSuffix);
            jqUnit.assertTrue("monitorProcess should have rejected with a value" + messageSuffix, !!e);
            jqUnit.assertTrue("monitorProcess should have rejected with an error" + messageSuffix,
                e instanceof Error || e.isError);

            runTest(testIndex + 1);
        });

    };

    runTest(0);
});

jqUnit.asyncTest("Test unmonitorProcess", function () {
    jqUnit.expect(4);

    var child1 = processHandlingTests.startProcess();
    var child2 = processHandlingTests.startProcess();
    var promise1 = processHandling.monitorProcess(child1.pid);
    var promise2 = processHandling.monitorProcess(child2.pid);

    var killed = false;
    var removed = false;

    promise1.then(function (value) {
        jqUnit.assertTrue("promise1 should not resolve before it's removed", removed);
        jqUnit.assertEquals("promise1 should resolve with 'removed'", "removed", value);
        killed = true;
        child2.kill();
    }, jqUnit.fail);

    promise2.then(function (value) {
        jqUnit.assertTrue("promise2 should not resolve before the process is killed", killed);
        jqUnit.assertEquals("promise2 should resolve with the process id", child2.pid, value);
        jqUnit.start();
    }, jqUnit.fail);

    setTimeout(function () {
        removed = true;
        processHandling.unmonitorProcess(child1.pid);
    }, 100);

});

// Test starting and stopping the service
jqUnit.asyncTest("Service start+stop", function () {
    jqUnit.expect(1);

    var service = require("../src/service.js");

    var mutexName = "gpii-test-" + Math.random().toString(32);

    // Configure a child process
    service.config = {
        processes: {
            testProcess: {
                command: "node.exe " + path.join(__dirname, "gpii-ipc-tests-child.js") + " mutex " + mutexName,
                autoRestart: false
            }
        }
    };

    // Mock process.exit - the call to this is expected, but not desired.
    var oldExit = process.exit;
    process.exit = function () {
        console.log("process.exit()");
        jqUnit.assert("process.exit");
    };
    teardowns.push(function () {
        process.exit = oldExit;
    });


    // Start the service
    service.start();

    // Wait for the child process to start.
    processHandlingTests.waitForMutex(mutexName, 5000).then(function (value) {
        if (value === "timeout") {
            jqUnit.fail("Timed out waiting for child process");
        } else {
            var pid = processHandling.childProcesses.testProcess.pid;

            // stop the service, see if the child terminates.
            service.stop();

            windows.waitForProcessTermination(pid, 15000).then(function (value) {
                if (value === "timeout") {
                    jqUnit.fail("Timed out waiting for child process to terminate");
                } else {
                    console.log("Process died");
                    jqUnit.start();
                }
            });
        }
    }, jqUnit.fail);
});
