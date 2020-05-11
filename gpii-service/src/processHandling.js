/* Manages the child processes.
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

var service = require("./service.js"),
    ipc = require("./gpii-ipc.js"),
    windows = require("./windows.js"),
    winapi = require("./winapi.js");

var processHandling = {};
module.exports = processHandling;

/**
 * Configuration of a child process (taken from service.json5)
 * @typedef {Object} ProcessConfig
 * @property {String} command The command.
 * @property {String} key Identifier.
 * @property {Boolean} autoRestart true to re-start the process if terminates.
 * @property {String} ipc IPC channel name (optional).
 * @property {Object} env Environment variables to set (optional).
 * @property {String} currentDir The current dir (optional).
 */

/**
 * A running child process
 * @typedef {Object} ChildProcess
 * @property {ProcessConfig} procConfig The process configuration.
 * @property {Number} pid Process ID.
 * @property {Array<Number>} lastStart When the process was started (used for restart-throttling)
 * @property {Number} failureCount How many times this process has failed to start.
 * @property {Boolean} shutdown true if shutting down.
 * @property {String} creationTime A number representing the time the process started. Used to ensure the process
 *   identified by pid is the same process.
 */

/**
 * @type {Array<ChildProcess>}
 */
processHandling.childProcesses = {};

/**
 * The active console session has changed.
 * @param {String} eventType The event type for the sessionChange event (see service.controlHandler()).
 */
processHandling.sessionChange = function (eventType) {
    service.logDebug("session change", eventType);

    switch (eventType) {
    case "session-logon":
        // User just logged on - start the processes.
        processHandling.startChildProcesses();
        break;
    case "session-logoff":
        // User just logged off - stop the processes (windows should have done this already).
        processHandling.stopChildProcesses();
        break;
    }
};

/**
 * Starts the configured processes.
 */
processHandling.startChildProcesses = function () {
    var processes = Object.keys(service.config.processes);
    // Start each child process sequentially.
    var startNext = function () {
        var key = processes.shift();
        if (key && !service.config.processes[key].disabled) {
            var proc = Object.assign({key: key}, service.config.processes[key]);
            processHandling.startChildProcess(proc).then(startNext, function (err) {
                service.logError("startChildProcess failed for " + key, err);
                startNext();
            });
        }
    };
    startNext();
};

/**
 * Starts a process.
 *
 * @param {ProcessConfig} procConfig The process configuration (from service-config.json).
 * @return {Promise} Resolves (with the pid) when the process has started.
 */
processHandling.startChildProcess = function (procConfig) {
    var childProcess = processHandling.childProcesses[procConfig.key];

    service.log("Starting " + procConfig.key + ": " + (procConfig.command || "pipe only"));
    service.logDebug("Process config: ", JSON.stringify(procConfig));

    if (childProcess) {
        if (processHandling.isProcessRunning(childProcess.pid, childProcess.creationTime)) {
            service.logWarn("Process " + procConfig.key + " is already running");
            return Promise.reject();
        }
    } else {
        childProcess = {
            procConfig: procConfig
        };
        processHandling.childProcesses[procConfig.key] = childProcess;
    }

    childProcess.pid = 0;
    childProcess.pipe = null;
    childProcess.lastStart = process.hrtime();
    childProcess.shutdown = false;

    var startOptions = {
        env: procConfig.env,
        currentDir: procConfig.currentDir,
        authenticate: !procConfig.noAuth,
        admin: procConfig.admin,
        processKey: procConfig.key
    };

    var processPromise = null;

    if (procConfig.ipc) {
        startOptions.messaging = true;
        // Start the process with a pipe.
        processPromise = ipc.startProcess(procConfig.command, procConfig.ipc, startOptions).then(function (p) {
            if (procConfig.command) {
                childProcess.pid = p.pid;
                childProcess.pipe = null;
                childProcess.creationTime = processHandling.getProcessCreationTime(childProcess.pid);
            }
            return p.pid;
        });
    } else if (procConfig.command) {
        // Start the process without a pipe.
        childProcess.pid = ipc.execute(procConfig.command, startOptions);
        childProcess.creationTime = processHandling.getProcessCreationTime(childProcess.pid);
        processPromise = Promise.resolve(childProcess.pid);
    }

    return processPromise.then(function (pid) {
        if (procConfig.command && procConfig.autoRestart) {
            processHandling.autoRestartProcess(procConfig.key);
        }
        return pid;
    });
};

/**
 * Stops all child processes. This is performed when the service has been told to stop.
 */
processHandling.stopChildProcesses = function () {
    service.log("Stopping processes");
    var processKeys = Object.keys(processHandling.childProcesses);
    processKeys.forEach(function (processKey) {
        processHandling.stopChildProcess(processKey);
    });
};

/**
 * Stops a child process.
 * @param {String} processKey Identifies the child process.
 * @param {Boolean} [restart] Allow the process to be restarted, if configured.
 */
processHandling.stopChildProcess = function (processKey, restart) {
    var childProcess = processHandling.childProcesses[processKey];
    if (childProcess) {
        service.log("Stopping " + processKey + ": " + childProcess.procConfig.command);

        childProcess.shutdown = !restart;

        if (processHandling.isProcessRunning(childProcess.pid,  childProcess.creationTime)) {
            try {
                process.kill(childProcess.pid);
            } catch (e) {
                // Ignored.
            }
        } else {
            service.logDebug("Process '" + processKey + "' is not running");
        }
    }
};

/**
 * Set a running process to not restart. Called when the impending termination is intentional.
 * @param {String} processKey Identifies the child process.
 */
processHandling.dontRestartProcess = function (processKey) {
    var childProcess = processHandling.childProcesses[processKey];
    if (childProcess) {
        childProcess.shutdown = true;
    }
};

/**
 * Auto-restarts a child process when it terminates.
 *
 * @param {String} processKey Identifies the child process.
 */
processHandling.autoRestartProcess = function (processKey) {
    var childProcess = processHandling.childProcesses[processKey];
    processHandling.monitorProcess(childProcess.pid).then(function () {
        service.log("Child process '" + processKey + "' died");
        service.emit("process.stop", processKey);

        if (childProcess.shutdown) {
            service.log("Not restarting process (shutting down)");
        } else {
            var restart = true;
            // Check if it's failing to start - if it's been running for less than 20 seconds.
            var timespan = process.hrtime(childProcess.lastStart);
            var seconds = timespan[0];
            if (seconds > 20) {
                childProcess.failureCount = 0;
            } else {
                service.logWarn("Process '" + processKey + "' failed at start.");
                childProcess.failureCount = (childProcess.failureCount || 0) + 1;
                if (childProcess.failureCount > 5) {
                    // Crashed at the start too many times.
                    service.logError("Unable to start process '" + processKey + "'");
                    restart = false;
                }
            }

            if (restart) {
                // Delay restart it.
                var delay = processHandling.throttleRate(childProcess.failureCount);
                service.logDebug("Restarting process '" + processKey + "' in " + Math.round(delay / 1000) + " seconds.");
                setTimeout(processHandling.startChildProcess, delay, childProcess.procConfig);
            }
        }
    });
};

/**
 * Gets the number of milliseconds to delay a process restart.
 *
 * @param {Number} failureCount The number of times the process has failed to start.
 * @return {Number} Returns 10 seconds for every failure count.
 */
processHandling.throttleRate = function (failureCount) {
    return failureCount * 10000;
};

/**
 * Determine if a process is running.
 *
 * To deal with PID re-use, provide creationTime (from a previous call to getProcessCreationTime) to also determine if
 * the running process ID still refers to the original one at the time of the getProcessCreationTime call, and hasn't
 * been re-used.
 *
 * @param {Number} pid The process ID.
 * @param {String} creationTime [Optional] Numeric string representing the time the process started.
 * @return {Boolean} true if the process is running, and has the same creation time (if provided).
 */
processHandling.isProcessRunning = function (pid, creationTime) {
    var running = false;

    if (pid > 0) {
        try {
            process.kill(pid, 0);
            running = true;
        } catch (e) {
            // It's not running.
        }

        var newCreationTime = processHandling.getProcessCreationTime(pid);
        if (running) {
            if (creationTime && newCreationTime) {
                // The pid is running, return true if it's the same process.
                running = newCreationTime === creationTime;
            } else {
                running = !!newCreationTime;
            }
        } else {
            if (newCreationTime) {
                // The process isn't running, but the pid is still valid.
                // This could mean CloseHandle hasn't been called (by this, or any other process).
                service.logDebug("Possible process handle leak on pid " + pid);
            }
        }
    }

    return running;
};

/**
 * Gets the time that the given process was started. This is used when determining if a pid still refers to the same
 * process, due to the high level of pid re-use that Windows provides.
 *
 * The return value is intended to be compared to another call to this function, so the actual value (microseconds
 * between 1601-01-01 and when the process started) isn't important.
 *
 * @param {Number} pid The process ID.
 * @return {String} A numeric string, representing the time the process started - null if there's no such process.
 */
processHandling.getProcessCreationTime = function (pid) {
    var creationTime = new winapi.FILETIME(),
        exitTime = new winapi.FILETIME(),
        kernelTime = new winapi.FILETIME(),
        userTime = new winapi.FILETIME();

    var success = false;
    var processHandle = null;

    try {

        if (pid > 0) {
            processHandle = winapi.kernel32.OpenProcess(winapi.constants.PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
        }

        if (processHandle === winapi.NULL) {
            service.logDebug(winapi.errorText("OpenProcess", "NULL"));
        } else if (processHandle) {
            success = winapi.kernel32.GetProcessTimes(
                processHandle, creationTime.ref(), exitTime.ref(), kernelTime.ref(), userTime.ref());
            if (!success) {
                service.logDebug(winapi.errorText("GetProcessTimes", success));
            }
        }
    } finally {
        if (processHandle) {
            winapi.kernel32.CloseHandle(processHandle);
        }
    }

    return success
        ? creationTime.ref().readUInt64LE()
        : null;
};

// handle: { handle, pid, resolve, reject }
processHandling.monitoredProcesses = {};
// The last process to be monitored.
processHandling.lastProcess = null;

/**
 * Resolves when the given process terminates.
 *
 * Using WaitForSingleObject is normally enough, but because calling that (using FFI's async method) creates a thread,
 * and there will be several processes to wait upon, WaitForMultipleObjects is used instead.
 *
 * An event (https://msdn.microsoft.com/library/ms686915) will also be added to the things to wait for, so when another
 * process is added to the monitoring list WaitForMultipleObjects can be restarted. (A nicer way would be to alert the
 * thread, but the thread is handled by ffi+libuv).
 *
 * @param {Number} pid The process ID.
 * @return {Promise} Resolves when the process identified by pid terminates.
 */
processHandling.monitorProcess = function (pid) {

    return new Promise(function (resolve, reject) {
        // Get the process handle.
        var processHandle = winapi.kernel32.OpenProcess(winapi.constants.SYNCHRONIZE, 0, pid);
        if (processHandle === winapi.NULL) {
            reject(windows.win32Error("OpenProcess"));
        }

        processHandling.lastProcess = {
            handle: processHandle,
            pid: pid,
            resolve: resolve,
            reject: reject
        };

        // Add this process to the monitored list.
        processHandling.monitoredProcesses[processHandle] = processHandling.lastProcess;

        // (Re)start the waiting.
        var event = processHandling.monitoredProcesses.event;
        if (event) {
            // Cause the current call to WaitForMultipleObjects to unblock, so the new process can also be monitored.
            winapi.kernel32.SetEvent(event.handle);
        } else {
            // Create the event.
            var eventHandle = winapi.kernel32.CreateEventW(winapi.NULL, false, false, winapi.NULL);
            processHandling.monitoredProcesses.event = {
                handle: eventHandle,
                isEvent: true
            };

            processHandling.startWait();
        }
    });
};

/**
 * Stops a monitored process from being monitored. The promises for the process will resolve with "removed".
 *
 * @param {Number|Object} process  The process ID, or the object in processHandling.monitoredProcesses.
 * @param {Boolean} removeOnly true to only remove it from the list of monitored processes.
 */
processHandling.unmonitorProcess = function (process, removeOnly) {
    var resolves = [];
    var pid = parseInt(process);
    if (pid) {
        for (var key in processHandling.monitoredProcesses) {
            var proc = !isNaN(key) && processHandling.monitoredProcesses[key];
            if (proc && proc.pid === pid) {
                processHandling.unmonitorProcess(proc, removeOnly);
            }
        }
    } else {
        winapi.kernel32.CloseHandle(process.handle);
        resolves.push(process.resolve);
        delete processHandling.monitoredProcesses[process.handle];

        if (!removeOnly) {
            if (processHandling.monitoredProcesses.event) {
                // Cause the current call to WaitForMultipleObjects to unblock to update the list.
                winapi.kernel32.SetEvent(processHandling.monitoredProcesses.event.handle);
            }

            resolves.forEach(function (resolve) {
                resolve("removed");
            });
        }
    }
};

/**
 * Performs the actual monitoring of the processes added by monitorProcess().
 * Explained in processHandling.monitorProcess().
 */
processHandling.startWait = function () {
    var handles = Object.keys(processHandling.monitoredProcesses).map(function (key) {
        return processHandling.monitoredProcesses[key].handle;
    });

    if (handles.length <= 1) {
        // Other than the event, there's nothing to wait for. Release the event and don't start the wait.
        winapi.kernel32.CloseHandle(processHandling.monitoredProcesses.event.handle);
        delete processHandling.monitoredProcesses.event;
    } else {
        // Wait for one or more of the handles (processes or the event) to do something.
        windows.waitForMultipleObjects(handles).then(function (handle) {
            var proc = processHandling.monitoredProcesses[handle] || processHandling.monitoredProcesses.event;
            if (proc.isEvent) {
                // The event was triggered to re-start waiting.
            } else {
                // Remove it from the list, and resolve.
                processHandling.unmonitorProcess(proc, true);
                proc.resolve(proc.pid);
            }
            // Start waiting again.
            processHandling.startWait();

        }, function (reason) {
            // The wait failed - it could be due to the most recent one so reject+remove that one and try again.
            var last = processHandling.lastProcess &&
                processHandling.monitoredProcesses[processHandling.lastProcess.handle];
            if (last) {
                if (processHandling.lastProcess.reject) {
                    processHandling.unmonitorProcess(processHandling.monitoredProcesses[last.handle], true);
                    processHandling.lastProcess.reject(reason);
                }
            } else {
                // Reject + remove all of them
                Object.keys(processHandling.monitoredProcesses).forEach(function (proc) {
                    if (!proc.isEvent) {
                        processHandling.unmonitorProcess(proc, true);
                        if (proc.reject) {
                            proc.reject(reason);
                        }
                    }
                });
            }
            processHandling.lastProcess = null;
            // Try the wait again (or release the event).
            processHandling.startWait();
        });
    }
};

// Listen for session change.
service.on("service.sessionchange", processHandling.sessionChange);
// Listen for service stop.
service.on("stop", processHandling.stopChildProcesses);
