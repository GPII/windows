/* IPC for GPII.
 * Starts a process (as another user) with a communications channel.
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

/*
How it works:
- A (randomly) named pipe is created and connected to.
- The child process is created, with one end of the pipe passed to it (using c-runtime file descriptor inheritance).
- The child process is then able to use the pipe as it would with any file descriptor.
- The parent (this process) can trust the client end of the pipe because it opened it itself.
- See GPII-2399.

The server (this process) end of the pipe is a node IPC socket and is created by node. The client end of the pipe can
also be a node socket, however due to how the child process is being started (as another user), node's exec/spawn can't
be used and the file handle for the pipe needs to be known. For this reason, the child-end of the pipe needs to be
created using the Win32 API. This doesn't affect how the client receives the pipe.

*/

var ref = require("ref"),
    net = require("net"),
    crypto = require("crypto"),
    Promise = require("bluebird"),
    service = require("./service.js"),
    windows = require("./windows.js"),
    logging = require("./logging.js");

var winapi = windows.winapi;

var ipc = service.module("gpiiIPC");
module.exports = ipc;

ipc.pipePrefix = "\\\\.\\pipe\\gpii-";
ipc.pipes = {};

/**
 * Starts a process as the current desktop user, passing the name of a pipe to connect to.
 *
 * @param command {String} The command to execute.
 * @param ipcName {String} [optional] The IPC channel name.
 * @param options {Object} [optional] Options (see also {this}.execute()).
 * @param options.authenticate {boolean} Child must authenticate to pipe (default is true, if undefined).
 * @param options.admin {boolean} true to keep pipe access to admin-only.
 * @return {Promise} Resolves with a value containing the pipe server and pid.
 */
ipc.startProcess = function (command, ipcName, options) {
    if (options === undefined && typeof(ipcName) !== "string") {
        options = ipcName;
        ipcName = undefined;
    }
    options = Object.assign({}, options);
    options.authenticate = options.authenticate || options.authenticate === undefined;

    var pipeName;
    if (command) {
        pipeName = ipc.generatePipeName();
    } else if (ipcName) {
        pipeName = ipc.pipePrefix + ipcName;
    } else {
        return Promise.reject("startProcess must be called with command and/or ipcName.");
    }

    var ipcChannel = null;
    if (ipcName) {
        ipcChannel = ipc.pipes[ipcName];
        if (!ipcChannel) {
            ipcChannel = ipc.pipes[ipcName] = {
                name: ipcName
            };
        }
        ipcChannel.authenticate = options.authenticate;
        ipcChannel.admin = options.admin;
        ipcChannel.pid = null;
    }

    // Create the pipe, and pass it to a new process.
    return ipc.createPipe(pipeName, ipcChannel).then(function (pipeServer) {
        var pid = null;
        if (command) {
            options.env = Object.assign({}, options.env);
            options.env.GPII_SERVICE_PIPE = "pipe:" + pipeName.substr(ipc.pipePrefix.length);
            pid = ipc.execute(command, options);
            if (ipcChannel) {
                ipcChannel.pid = pid;
            }
        }

        return {
            pipeServer: pipeServer,
            pid: pid
        };
    });
};

/**
 * Generates a named-pipe name.
 *
 * @return {string} The name of the pipe.
 */
ipc.generatePipeName = function () {
    var pipeName = ipc.pipePrefix + crypto.randomBytes(18).toString("base64").replace(/[\\/]/g, ".");
    return pipeName;
};

/**
 * Open a named pipe, set the permissions, and start serving.
 *
 * @param pipeName {String} Name of the pipe.
 * @param ipcChannel {Object} The IPC channel.
 * @return {Promise} A promise resolving with the pipe server when the pipe is ready to receive a connection.
 */
ipc.createPipe = function (pipeName, ipcChannel) {
    return new Promise(function (resolve, reject) {
        if (pipeName) {
            var pipeServer = net.createServer();
            pipeServer.maxConnections = 1;

            pipeServer.on("error", function (err) {
                logging.debug("ipc server error", err);
                reject(err);
            });

            pipeServer.listen(pipeName, function () {
                logging.debug("pipe listening", pipeName);

                var p = (ipcChannel && ipcChannel.admin)
                    ? Promise.resolve()
                    : ipc.setPipeAccess(pipeServer, pipeName);

                p.then(function () {
                    if (ipcChannel) {
                        ipc.servePipe(ipcChannel, pipeServer);
                    }

                    resolve(pipeServer);
                });
            });
        } else {
            reject({
                isError: true,
                message: "pipeName was null"
            });
        }
    });
};

/**
 * Allows the desktop user to access the pipe.
 *
 * When running as a service, a normal user does not have enough permissions to open it.
 *
 * @param pipeServer {net.Server} The pipe server. All listeners of the "connection" event will be removed.
 * @param pipeName {string} Name of the pipe.
 * @return {Promise} Resolves when complete.
 */
ipc.setPipeAccess = function (pipeServer, pipeName) {
    return new Promise(function (resolve) {
        // setPipePermissions will connect to the pipe (and close it). This connection cna be ignored, however the
        // connection event needs to be swallowed before any more listeners are added.
        pipeServer.removeAllListeners("connection");
        pipeServer.on("connection", function (pipe) {
            pipeServer.removeAllListeners("connection");
            pipe.end();
            resolve();
        });

        windows.setPipePermissions(pipeName);
    });
};

/**
 * Start serving the pipe.
 *
 * @param ipcName {String} Name of the IPC channel.
 * @param pipeServer {net.Server} The pipe server.
 * @return {Promise} Resolves when the client has been validated, rejects if failed.
 */
ipc.servePipe = function (ipcChannel, pipeServer) {
    return new Promise(function (resolve, reject) {
        pipeServer.on("connection", function (pipe) {
            logging.debug("ipc got connection");

            if (ipcChannel.authenticate) {
                pipeServer.close();
                if (!ipcChannel.pid) {
                    throw new Error("Got pipe connection before client was started.");
                }
            }

            pipe.on("error", function (err) {
                logging.log("Pipe error", ipcChannel.name, err);
            });
            pipe.on("end", function () {
                logging.log("Pipe end", ipcChannel.name);
            });

            var promise;
            if (ipcChannel.authenticate) {
                promise = ipc.validateClient(pipe, ipcChannel.pid);
            } else {
                pipe.write("challenge:none\nOK\n");
                promise = Promise.resolve();
            }

            promise.then(function () {
                logging.log("Pipe client authenticated:", ipcChannel.name);
                ipcChannel.pipe = pipe;

                ipcChannel.pipe.on("data", function (data) {
                    logging.log("Pipe data", ipcChannel.name, data);
                });
            }).then(resolve, function (err) {
                logging.error("validateClient rejected the client:", err);
                reject(err);
            });
        });
    });
};

/**
 * Validates the client connection of a pipe.
 *
 * @param pipe {net.Socket} The pipe to the client.
 * @param pid {number} The pid of the expected client.
 * @param timeout {number} Seconds to wait for the event (default 30).
 * @return {Promise} Resolves when successful, rejects on failure.
 */
ipc.validateClient = function (pipe, pid, timeout) {

    // Create an event that's used to cancel waiting for the authentication event.
    var cancelEventHandle = winapi.kernel32.CreateEventW(winapi.NULL, false, false, ref.NULL);
    if (!cancelEventHandle) {
        throw winapi.error("CreateEvent", cancelEventHandle);
    }

    // Cancel waiting when the pipe was closed.
    var onPipeClose = function () {
        winapi.kernel32.SetEvent(cancelEventHandle);
    };
    pipe.on("close", onPipeClose);

    var processHandle = null;
    var childEventHandle = null;
    try {
        // Open the child process.
        processHandle = winapi.kernel32.OpenProcess(winapi.constants.PROCESS_DUP_HANDLE, 0, pid);
        if (!processHandle) {
            throw winapi.error("OpenProcess", processHandle);
        }

        // Create the event.
        var eventHandle = winapi.kernel32.CreateEventW(winapi.NULL, false, false, ref.NULL);
        if (!eventHandle) {
            throw winapi.error("CreateEvent", eventHandle);
        }

        // Duplicate the event handle for the child.
        var eventHandleBuf = ref.alloc(winapi.types.HANDLE);
        var ownProcess = -1 >>> 0;
        var success =
            winapi.kernel32.DuplicateHandle(ownProcess, eventHandle, processHandle, eventHandleBuf, ref.NULL, false, 2);
        if (!success) {
            throw winapi.error("DuplicateHandle", success);
        }
        childEventHandle = eventHandleBuf.deref();

        // Give the handle to the child.
        process.nextTick(function () {
            pipe.write("challenge:" + childEventHandle + "\n");
            service.logDebug("validateClient: send challenge");
        });

        // Wait for the cancel or child process event.
        return windows.waitForMultipleObjects([cancelEventHandle, eventHandle], (timeout || 30) * 1000, false).then(function (handle) {
            pipe.removeListener("close", onPipeClose);
            if (handle === eventHandle) {
                pipe.write("OK\n");
                return Promise.resolve("success");
            } else {
                pipe.end();
                return Promise.reject("failed");
            }
        });

    } finally {
        if (processHandle) {
            winapi.kernel32.CloseHandle(processHandle);
        }
    }
};

/**
 * Executes a command in the context of the console user.
 *
 * https://blogs.msdn.microsoft.com/winsdk/2013/04/30/how-to-launch-a-process-interactively-from-a-windows-service/
 *
 * @param command {String} The command to execute.
 * @param options {Object} [optional] Options
 * @param options.alwaysRun {boolean} true to run as the current user (what this process is running as), if the console
 * user token could not be received. Should only be true if not running as a service.
 * @param options.env {object} Additional environment key-value pairs.
 * @param options.currentDir {string} Current directory for the new process.
 *
 * @return {Number} The pid of the new process.
 */
ipc.execute = function (command, options) {
    options = Object.assign({}, options);

    var userToken = windows.getDesktopUser();
    if (!userToken) {
        // There is no token for this session - perhaps no one is logged on, or is in the lock-screen (screen saver).
        // Continuing could cause something to be executed as the LocalSystem account, which may be undesired.
        if (!options.alwaysRun) {
            throw new Error("Unable to get the current desktop user (error=" + userToken.error + ")");
        }

        logging.warn("ipc.startProcess invoking as current user.");
        userToken = 0;
    }

    var pid = null;

    try {

        // Get a user-specific environment block. Without this, the new process will take the environment variables
        // of the service, causing GPII to use the incorrect data directory.
        var env = windows.getEnv(userToken);
        if (options.env) {
            // Add some extra values.
            for (var name in options.env) {
                var newValue = name + "=" + options.env[name];

                // If there's already a variable with that name, replace it.
                var re = new RegExp("^" + name + "=");
                var index = env.findIndex(re.test, re);
                if (index >= 0) {
                    env.splice(index, 1, newValue);
                } else {
                    env.push(newValue);
                }
            }
        }

        // Convert the environment block into a C string array.
        var envString = env.join("\0") + "\0";
        var envBuf = winapi.stringToWideChar(envString);

        var commandBuf = winapi.stringToWideChar(command);
        var creationFlags = winapi.constants.CREATE_UNICODE_ENVIRONMENT | winapi.constants.CREATE_NEW_CONSOLE;

        var currentDirectory = options.currentDir
            ? winapi.stringToWideChar(options.currentDir)
            : ref.NULL;

        var startupInfo = new winapi.STARTUPINFOEX();
        startupInfo.ref().fill(0);
        startupInfo.cb = winapi.STARTUPINFOEX.size;
        startupInfo.lpDesktop = winapi.stringToWideChar("winsta0\\default");

        var processInfoBuf = new winapi.PROCESS_INFORMATION();
        processInfoBuf.ref().fill(0);

        var ret = winapi.advapi32.CreateProcessAsUserW(userToken, ref.NULL, commandBuf, ref.NULL, ref.NULL,
            !!options.inheritHandles, creationFlags, envBuf, currentDirectory, startupInfo.ref(), processInfoBuf.ref());

        if (!ret) {
            throw winapi.error("CreateProcessAsUser");
        }

        pid = processInfoBuf.dwProcessId;

        winapi.kernel32.CloseHandle(processInfoBuf.hThread);
        winapi.kernel32.CloseHandle(processInfoBuf.hProcess);

    } finally {
        if (userToken) {
            winapi.kernel32.CloseHandle(userToken);
        }
    }

    return pid;
};
