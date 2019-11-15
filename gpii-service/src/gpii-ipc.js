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

var ref = require("ref-napi"),
    net = require("net"),
    crypto = require("crypto"),
    service = require("./service.js"),
    windows = require("./windows.js"),
    logging = require("./logging.js"),
    messaging = require("../shared/pipe-messaging.js");

var winapi = windows.winapi;

var ipc = {};
module.exports = ipc;

ipc.pipePrefix = "\\\\.\\pipe\\gpii-";

/**
 * A connection to a client.
 * @typedef {Object} IpcConnection
 * @property {boolean} authenticate true if authentication is required.
 * @property {boolean} admin true to run the process as administrator.
 * @property {number} pid The client pid.
 * @property {String} name Name of the connection.
 * @property {String} processKey Identifies the child process.
 * @property {messaging.Session} messaging Messaging session.
 * @property {function} requestHandler Function to handle requests for this connection.
 */

/**
 * @type {IpcConnection[]}
 */
ipc.ipcConnections = {};

/**
 * Starts a process as the current desktop user, passing the name of a pipe to connect to.
 *
 * @param {String} command The command to execute.
 * @param {String} ipcName [optional] The IPC connection name.
 * @param {Object} options [optional] Options (see also {this}.execute()).
 * @param {Boolean} options.authenticate Child must authenticate to pipe (default is true, if undefined).
 * @param {Boolean} options.admin true to keep pipe access to admin-only.
 * @param {Boolean} options.messaging true to use the messaging wrapper.
 * @param {Boolean} options.processKey Identifies the child process.
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

    var ipcConnection = null;
    if (ipcName) {
        ipcConnection = ipc.ipcConnections[ipcName];
        if (!ipcConnection) {
            ipcConnection = ipc.ipcConnections[ipcName] = {};
        }
        ipcConnection.name = ipcName;
        ipcConnection.authenticate = options.authenticate;
        ipcConnection.admin = options.admin;
        ipcConnection.processKey = options.processKey;
        ipcConnection.messaging = options.messaging ? undefined : false;
    }

    // Create the pipe, and pass it to a new process.
    return ipc.createPipe(pipeName, ipcConnection).then(function (pipeServer) {
        var pid = null;
        if (command) {
            options.env = Object.assign({}, options.env);
            options.env.GPII_SERVICE_PIPE = "pipe:" + pipeName.substr(ipc.pipePrefix.length);
            pid = ipc.execute(command, options);
            if (ipcConnection) {
                ipcConnection.pid = pid;
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
 * @return {String} The name of the pipe.
 */
ipc.generatePipeName = function () {
    var pipeName = ipc.pipePrefix + crypto.randomBytes(18).toString("base64").replace(/[\\/]/g, ".");
    return pipeName;
};

/**
 * Open a named pipe, set the permissions, and start serving.
 *
 * @param {String} pipeName Name of the pipe.
 * @param {IpcConnection} ipcConnection The IPC connection.
 * @return {Promise} A promise resolving with the pipe server when the pipe is ready to receive a connection.
 */
ipc.createPipe = function (pipeName, ipcConnection) {
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

                var p = (ipcConnection && ipcConnection.admin)
                    ? Promise.resolve()
                    : ipc.setPipeAccess(pipeServer, pipeName);

                p.then(function () {
                    if (ipcConnection) {
                        // eslint-disable-next-line dot-notation
                        ipc.servePipe(ipcConnection, pipeServer).catch(reject);
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
 * @param {net.Server} pipeServer The pipe server. All listeners of the "connection" event will be removed.
 * @param {String} pipeName Name of the pipe.
 * @return {Promise} Resolves when complete.
 */
ipc.setPipeAccess = function (pipeServer, pipeName) {
    return new Promise(function (resolve) {
        // setPipePermissions will connect to the pipe (and close it). This connection can be ignored, however the
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
 * @param {IpcConnection} ipcConnection The IPC connection.
 * @param {net.Server} pipeServer The pipe server.
 * @return {Promise} Resolves when the client has been validated, rejects if failed.
 */
ipc.servePipe = function (ipcConnection, pipeServer) {
    return new Promise(function (resolve, reject) {
        pipeServer.on("connection", function (pipe) {
            logging.debug("ipc got connection");

            if (ipcConnection.authenticate) {
                pipeServer.close();
                if (!ipcConnection.pid) {
                    throw new Error("Got pipe connection before client was started.");
                }
            }

            pipe.on("error", function (err) {
                logging.log("Pipe error", ipcConnection.name);
                service.on("ipc.error", ipcConnection.name, ipcConnection, err);
            });
            pipe.on("close", function () {
                logging.log("Pipe close", ipcConnection.name);
                service.emit("ipc.closed", ipcConnection.name, ipcConnection);
            });

            var promise;
            if (ipcConnection.authenticate) {
                promise = ipc.validateClient(pipe, ipcConnection.pid);
            } else {
                pipe.write("challenge:none\nOK\n");
                promise = Promise.resolve();
            }

            promise.then(function () {
                logging.log("Pipe client authenticated:", ipcConnection.name);
                ipcConnection.pipe = pipe;

                var handleRequest = function (request) {
                    return ipc.handleRequest(ipcConnection, request);
                };

                if (ipcConnection.messaging !== false) {
                    ipcConnection.messaging = messaging.createSession(ipcConnection.pipe, ipcConnection.name, handleRequest);
                    ipcConnection.messaging.on("ready", function () {
                        service.emit("ipc.connected", ipcConnection.name, ipcConnection);
                    });
                }
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
 * @param {net.Socket} pipe The pipe to the client.
 * @param {Number} pid The pid of the expected client.
 * @param {Number} timeout Seconds to wait for the event (default 30).
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
        var ownProcess = 0xffffffff; // (uint)-1
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
 * @param {String} command The command to execute.
 * @param {Object} options [optional] Options
 * @param {Boolean} options.alwaysRun true to run as the current user (what this process is running as), if the console
 * user token could not be received. Should only be true if not running as a service.
 * @param {Object} options.env Additional environment key-value pairs.
 * @param {String} options.currentDir Current directory for the new process.
 * @param {Boolean} options.elevated Run with elevated privileges.
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

    var runAsToken;
    if (userToken && options.elevated) {
        try {
            runAsToken = windows.getElevatedToken(userToken);
        } catch (err) {
            runAsToken = 0;
            logging.warn("getElevatedToken failed", err);
        }
    } else {
        runAsToken = userToken;
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

        var ret;
        if (options.elevated && !runAsToken) {
            // There was a problem getting the elevated token (non-admin user), run as the current user.
            ret = winapi.kernel32.CreateProcessW(ref.NULL, commandBuf, ref.NULL, ref.NULL,
                !!options.inheritHandles, creationFlags, envBuf, currentDirectory,
                startupInfo.ref(), processInfoBuf.ref());
        } else {
            ret = winapi.advapi32.CreateProcessAsUserW(runAsToken, ref.NULL, commandBuf, ref.NULL, ref.NULL,
                !!options.inheritHandles, creationFlags, envBuf, currentDirectory,
                startupInfo.ref(), processInfoBuf.ref());
        }
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

/**
 * Handles a request received from a client.
 *
 * @param {IpcConnection} ipcConnection The IPC connection.
 * @param {Object} request The request data.
 * @return {Object} The result of the requestHandler callback.
 */
ipc.handleRequest = function (ipcConnection, request) {
    return ipcConnection.requestHandler && ipcConnection.requestHandler(request);
};

/**
 * Sends a request.
 *
 * @param {IpcConnection|String} ipcConnection The IPC connection.
 * @param {ServiceRequest} request The request data.
 * @return {Promise} Resolves when there's a response.
 */
ipc.sendRequest = function (ipcConnection, request) {
    if (typeof(ipcConnection) === "string") {
        ipcConnection = ipc.ipcConnections[ipcConnection];
    }

    return ipcConnection.messaging.sendRequest(request);
};

service.on("ipc.connected", function (name, connection) {
    // emit another event that's bound to the IPC name
    service.emit("ipc.connected:" + name,  connection);
});
service.on("ipc.closed", function (name, connection) {
    // emit another event that's bound to the IPC name
    service.emit("ipc.closed:" + name,  connection);
});
