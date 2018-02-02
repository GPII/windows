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
    windows = require("./windows.js"),
    logging = require("./logging.js");

var winapi = windows.winapi;
var ipc = exports;

/**
 * Starts a process as the current desktop user, with an open pipe inherited.
 *
 * @param command {String} The command to execute.
 * @param options {Object} [optional] Options (see {this}.execute()).
 * @return {Promise} Resolves with a value containing the pipe and pid.
 */
ipc.startProcess = function (command, options) {
    options = Object.assign({}, options);
    var pipeName = ipc.generatePipeName();

    // Create the pipe, and pass it to a new process.
    return ipc.createPipe(pipeName).then(function (pipePair) {
        options.inheritHandles = [pipePair.clientHandle];
        options.env = Object.assign({}, options.env);
        // Tell the child how to connect to the pipe. '3' is the 1st inherited handle after the 3 standard streams.
        options.env.GPII_SERVICE_IPC = "fd://3";

        var pid = ipc.execute(command, options);

        return {
            pipe: pipePair.serverConnection,
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
    var pipeName = "\\\\.\\pipe\\gpii-" + crypto.randomBytes(18).toString("base64").replace(/[\\/]/g, ".");
    return pipeName;
};

/**
 * Open a named pipe, and connect to it.
 *
 * @param pipeName {String} Name of the pipe.
 * @return {Promise} A promise resolving when the pipe has been connected to, with an object containing both ends to the
 * pipe.
 */
ipc.createPipe = function (pipeName) {
    return new Promise(function (resolve, reject) {
        var pipe = {
            serverConnection: null,
            clientHandle: null
        };

        var server = net.createServer();

        server.maxConnections = 1;
        server.on("connection", function (connection) {
            logging.debug("ipc got connection");
            pipe.serverConnection = connection;
            server.close();
            if (pipe.clientHandle) {
                resolve(pipe);
            }
        });

        server.on("error", function (err) {
            //logging.log("ipc server error", err);
            reject(err);
        });

        server.listen(pipeName, function () {
            ipc.connectToPipe(pipeName).then(function (pipeHandle) {
                logging.debug("ipc connected to pipe");
                pipe.clientHandle = pipeHandle;
                if (pipe.serverConnection) {
                    resolve(pipe);
                }
            }, reject);
        });
    });
};

/**
 * Connect to a named pipe.
 *
 * @param pipeName {String} Name of the pipe.
 * @return {Promise} Resolves when the connection is made, with the win32 handle of the pipe.
 */
ipc.connectToPipe = function (pipeName) {
    return new Promise(function (resolve, reject) {
        var pipeNameBuf = winapi.stringToWideChar(pipeName);
        winapi.kernel32.CreateFileW.async(pipeNameBuf, winapi.constants.GENERIC_READWRITE, 0, ref.NULL,
            winapi.constants.OPEN_EXISTING, winapi.constants.FILE_FLAG_OVERLAPPED, 0,
            function (err, pipeHandle) {
                if (err) {
                    reject(err);
                } else if (pipeHandle === winapi.constants.INVALID_HANDLE_VALUE || !pipeHandle) {
                    reject(winapi.error("CreateFile"));
                } else {
                    resolve(pipeHandle);
                }
            });
    });
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
 * @param options.inheritHandles {Number[]} An array of win32 file handles for the child to inherit.
 * @param options.keepHandles {boolean} True to keep the handles in inheritHandles open (default: false to close).
 * @param options.standardHandles {Number[]} Standard handles (stdin, stdout, and stderr) to pass to the child.
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

        if (options.standardHandles || options.inheritHandles) {
            var STARTF_USESTDHANDLES = 0x00000100;
            startupInfo.dwFlags = STARTF_USESTDHANDLES;

            // Provide the standard handles.
            if (options.standardHandles) {
                startupInfo.hStdInput = options.standardHandles[0] || 0;
                startupInfo.hStdOutput = options.standardHandles[1] || 0;
                startupInfo.hStdError = options.standardHandles[2] || 0;
            }

            // Add the handles to the lpReserved2 structure. This is how the CRT passes handles to a child. When the
            // child starts it is able to use the file as a normal file descriptor.
            // Node uses this same technique: https://github.com/nodejs/node/blob/master/deps/uv/src/win/process.c#L1048
            var allHandles = [startupInfo.hStdInput, startupInfo.hStdOutput, startupInfo.hStdError];
            if (options.inheritHandles) {
                allHandles.push.apply(allHandles, options.inheritHandles);
            }

            var handles = winapi.createHandleInheritStruct(allHandles.length);
            handles.ref().fill(0);
            handles.length = allHandles.length;

            for (var n = 0; n < allHandles.length; n++) {
                handles.flags[n] = winapi.constants.FOPEN;
                handles.handle[n] = allHandles[n];
                // Mark the handle as inheritable.
                winapi.kernel32.SetHandleInformation(
                    allHandles[n], winapi.constants.HANDLE_FLAG_INHERIT, winapi.constants.HANDLE_FLAG_INHERIT);
            }

            startupInfo.cbReserved2 = handles["ref.buffer"].byteLength;
            startupInfo.lpReserved2 = handles.ref();
        }
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

        if (options.keepHandles && options.inheritHandles) {
            // Close the handles that where inherited
            options.inheritHandles.forEach(function (handle) {
                winapi.kernel32.CloseHandle(handle);
            });
        }

    } finally {
        if (userToken) {
            winapi.kernel32.CloseHandle(userToken);
        }
    }

    return pid;
};
