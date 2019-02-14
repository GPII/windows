/* Things related to the operating system.
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
    logging = require("./logging.js"),
    winapi = require("./winapi.js"),
    path = require("path");

var windows = {
    winapi: winapi
};

/**
 * Determine if this process is running as a service.
 *
 * @return {Boolean} true if running as a service.
 */
windows.isService = function () {
    // Services run in session 0
    var sessionId = ref.alloc(winapi.types.DWORD);
    var success = winapi.kernel32.ProcessIdToSessionId(process.pid, sessionId);

    if (!success) {
        throw windows.win32Error("ProcessIdToSessionId", success);
    }

    return sessionId.deref() === 0;
};

/**
 * Returns an Error containing the arguments.
 *
 * @param {String} message The message.
 * @param {String|Number} returnCode [optional] The return code.
 * @param {String|Number} errorCode [optional] The last win32 error (from GetLastError), if already known.
 * @return {Error} The error.
 */
windows.win32Error = function (message, returnCode, errorCode) {
    return winapi.error(message, returnCode, errorCode);
};

/**
 * Get the user token for the current process.
 *
 * This token must be closed with closeToken when no longer needed.
 *
 * @return {Number} The token handle.
 */
windows.getOwnUserToken = function () {
    // It's possible to just call GetCurrentProcessToken, but that returns a pseudo handle that doesn't have the
    // required permission to start a process as that user.

    // A pseudo handle - doesn't need to be closed;
    var processHandle = winapi.kernel32.GetCurrentProcess();
    // Enough for CreateProcessAsUser
    var access = winapi.constants.TOKEN_ASSIGN_PRIMARY | winapi.constants.TOKEN_DUPLICATE
        | winapi.constants.TOKEN_QUERY;
    var tokenBuf = ref.alloc(winapi.types.HANDLE);
    var success = winapi.advapi32.OpenProcessToken(processHandle, access, tokenBuf);

    if (!success) {
        throw winapi.error("OpenProcessToken failed");
    }

    return tokenBuf.deref();
};

/**
 * Closes a user token.
 * @param {Number} userToken The user token.
 */
windows.closeToken = function (userToken) {
    if (userToken) {
        winapi.kernel32.CloseHandle(userToken);
    }
};

/**
 * Gets the user token for the active desktop session.
 *
 * This token must be closed with closeToken when no longer needed.
 *
 * @return {Number} The token, 0 if there is no active desktop session.
 */
windows.getDesktopUser = function () {

    var userToken;

    if (windows.isService()) {
        // Get the session ID of the console session.
        var sessionId = winapi.kernel32.WTSGetActiveConsoleSessionId();
        logging.debug("session id:", sessionId);


        if (sessionId === 0xffffffff) {
            // There isn't a session.
            userToken = 0;
        } else {
            // Get the access token of the user logged into the session.
            var tokenBuf = ref.alloc(winapi.types.HANDLE);
            var success = winapi.wtsapi32.WTSQueryUserToken(sessionId, tokenBuf);

            if (success) {
                userToken = tokenBuf.deref();
            } else {
                var errorCode = winapi.kernel32.GetLastError();
                logging.warn("WTSQueryUserToken failed (win32=" + errorCode + ")");

                switch (errorCode) {
                case winapi.errorCodes.ERROR_NO_TOKEN:
                case winapi.errorCodes.ERROR_SUCCESS:
                    // There is no user on this session.
                    userToken = 0;
                    break;
                case winapi.errorCodes.ERROR_ACCESS_DENIED:
                case winapi.errorCodes.ERROR_PRIVILEGE_NOT_HELD:
                    // Not running as a service?
                    throw winapi.error("WTSQueryUserToken (isService may be wrong)", errorCode);
                    break;
                default:
                    throw winapi.error("WTSQueryUserToken", errorCode);
                    break;
                }
            }
        }
    } else {
        // If not running as a service, then assume the current user is the desktop user.
        userToken = windows.getOwnUserToken();
    }

    return userToken;
};

/**
 * Determines if the active console session is a user logged on.
 * @return {Boolean} true if the active console session is a user logged on.
 */
windows.isUserLoggedOn = function () {
    var token = windows.getDesktopUser();
    var loggedOn = !!token;
    if (token) {
        windows.closeToken(token);
    }

    return loggedOn;
};

/**
 * Gets the environment variables for the specified user.
 *
 * @param {Number} token Token handle for the user.
 * @return {Array<String>} An array of strings for each variable, in the format of "name=value"
 */
windows.getEnv = function (token) {
    var envPtr = ref.alloc(winapi.types.LP);
    var success = winapi.userenv.CreateEnvironmentBlock(envPtr, token, false);
    if (!success) {
        throw winapi.error("CreateEnvironmentBlock");
    }
    return winapi.stringFromWideCharArray(envPtr.deref(), true);
};

/**
 * Gets the GPII data directory for the specified user (identified by token).
 *
 * When running as a service, this process's "APPDATA" value will not point to the current user's.
 *
 * @param {Number} userToken Token handle for the user.
 * @return {String} The GPII data directory for the given user.
 */
windows.getUserDataDir = function (userToken) {
    // Search the environment block for the APPDATA value. (A better way would be to use SHGetKnownFolderPath)
    var env = windows.getEnv(userToken);
    var appData = null;
    for (var n = 0, len = env.length; n < len; n++) {
        var match = env[n].match(/^APPDATA=(.*)/i);
        if (match) {
            appData = match[1];
            break;
        }
    }

    return appData && path.join(appData, "GPII");
};

/**
 * Returns a promise that resolves when a process has terminated, or after the given timeout.
 *
 * @param {Number} pid The process ID.
 * @param {Number} timeout Milliseconds to wait before timing out. (default: infinate)
 * @return {Promise} Resolves when the process has terminated, or when timed out (with a value of "timeout"). Rejects
 * upon failure.
 */
windows.waitForProcessTermination = function (pid, timeout) {

    return new Promise(function (resolve, reject) {
        var hProcess = winapi.kernel32.OpenProcess(winapi.constants.SYNCHRONIZE, 0, pid);
        if (!hProcess) {
            reject(windows.win32Error("OpenProcess"));
        } else {
            if (!timeout && timeout !== 0) {
                timeout = winapi.constants.INFINITE;
            }
            winapi.kernel32.WaitForSingleObject.async(hProcess, timeout, function (err, ret) {
                winapi.kernel32.CloseHandle(hProcess);

                switch (ret) {
                case winapi.constants.WAIT_OBJECT_0:
                    resolve();
                    break;
                case winapi.constants.WAIT_TIMEOUT:
                    resolve("timeout");
                    break;
                case winapi.constants.WAIT_FAILED:
                default:
                    reject(windows.win32Error("WaitForSingleObject", ret));
                    break;
                }
            });
        }
    });
};

/**
 * Waits until one of the Win32 objects in an array are in the signalled state, resolving with that handle (or
 * "timeout").
 *
 * Wrapper for WaitForMultipleObjects (https://msdn.microsoft.com/library/ms687025)
 *
 * @param {Array<Number>} handles The win32 handles to wait on.
 * @param {Number} timeout [Optional] The timeout, in milliseconds. (default: infinite)
 * @param {Boolean} waitAll [Optional] Wait for all handles to be signalled, instead of just one.
 * @return {Promise} Resolves with the handle that triggered, "timeout", or "all" if waitAll is true.
 */
windows.waitForMultipleObjects = function (handles, timeout, waitAll) {

    return new Promise(function (resolve, reject) {
        if (!Array.isArray(handles)) {
            reject({
                message: "handles must be an array",
                isError: true
            });
            return;
        }
        // Use a copy the handle array, so it can't be modified after the function returns.
        handles = handles.slice();

        if (!timeout && timeout !== 0) {
            timeout = winapi.constants.INFINITE;
        }

        winapi.kernel32.WaitForMultipleObjects.async(handles.length, handles, waitAll, timeout,
            function (err, ret) {
                if (err) {
                    reject(err);
                } else {
                    switch (ret) {
                    case winapi.constants.WAIT_TIMEOUT:
                        resolve("timeout");
                        break;
                    case winapi.constants.WAIT_FAILED:
                        // GetLastError will not work, because WaitForMultipleObjects is called in a different thread.
                        // Call WaitForMultipleObjects again, but in this thread (with a short timeout in case it works)
                        var newRet = winapi.kernel32.WaitForMultipleObjects(
                            handles.length, handles, waitAll, 1);
                        var errorCode = winapi.kernel32.GetLastError();
                        var message = "WAIT_FAILED";
                        if (newRet !== ret) {
                            message += " 2nd return:" + newRet;
                        }
                        reject(winapi.error("WaitForMultipleObjects:" + message, ret, errorCode));
                        break;
                    default:
                        // The return is the handle index that triggered the return, offset by WAIT_OBJECT_0 or WAIT_ABANDONED_0
                        var index = (ret < winapi.constants.WAIT_ABANDONED_0)
                            ? ret - winapi.constants.WAIT_OBJECT_0
                            : ret - winapi.constants.WAIT_ABANDONED_0;

                        if (index < 0 || index >= handles.length) {
                            // Unknown return
                            reject(winapi.win32Error("WaitForMultipleObjects", ret));
                        } else {
                            resolve(handles[index]);
                        }

                        break;
                    }
                }
            });
    });
};

/**
 * Gets the security identifier (SID) from a user token.
 *
 * @param {Integer} token The user token.
 * @return {*} The SID of the user.
 */
windows.getSidFromToken = function (token) {
    // winnt.h:
    var TokenUser = 1;

    var lengthBuffer = ref.alloc(winapi.types.DWORD);
    // // Get the length
    var success = winapi.advapi32.GetTokenInformation(token, TokenUser, ref.NULL, 0, lengthBuffer);
    if (!success) {
        var err = winapi.kernel32.GetLastError();
        // ERROR_INSUFFICIENT_BUFFER is expected.
        if (err !== winapi.errorCodes.ERROR_INSUFFICIENT_BUFFER) {
            throw winapi.error("GetTokenInformation", success);
        }
    }

    // GetTokenInformation fills a TOKEN_USER structure, which contains another struct containing a pointer to the SID
    // and a dword. The sid pointer points to a chunk of data, which is located after the struct.
    var length = lengthBuffer.deref();
    var tokenUserBuffer = Buffer.alloc(length);
    // Get the sid data.
    success = winapi.advapi32.GetTokenInformation(token, TokenUser, tokenUserBuffer, length, lengthBuffer);
    if (!success) {
        throw winapi.error("GetTokenInformation", success);
    }

    // Take the SID from the buffer.
    var TokenUserHeader = 2 * ref.types["int"].size;
    var sid = tokenUserBuffer.slice(TokenUserHeader);

    return sid;
};

/**
 * Set the permissions of the pipe so the logged in user can access it.
 *
 * This connects to the pipe, modifies the ACL to include the desktop user's security descriptor, then closes the pipe.
 *
 * @param {String} pipeName Name of the pipe.
 */
windows.setPipePermissions = function (pipeName) {
    // winnt.h
    var FILE_GENERIC_READ = 0x120089;
    var FILE_GENERIC_WRITE = 0x120116;
    var DACL_SECURITY_INFORMATION = 0x4;
    // AccCtl.h
    var SE_KERNEL_OBJECT = 0x6;
    var GRANT_ACCESS = 1;
    var TRUSTEE_IS_SID = 0;
    var TRUSTEE_IS_USER = 1;

    var token = windows.getDesktopUser();
    var sid;
    try {
        sid = windows.getSidFromToken(token);
    } finally {
        windows.closeToken(token);
    }

    var pipeHandle = null;

    try {
        // Open the pipe.
        var pipeNameBuf = winapi.stringToWideChar(pipeName);
        pipeHandle = winapi.kernel32.CreateFileW(
            pipeNameBuf, (winapi.constants.GENERIC_READ | winapi.constants.WRITE_DAC) >>> 0, 0,
            ref.NULL, winapi.constants.OPEN_EXISTING, ref.NULL, ref.NULL);

        if (pipeHandle === winapi.constants.INVALID_HANDLE_VALUE) {
            throw winapi.error("CreateFile", pipeHandle);
        }

        // Get the ACL.
        var daclP = ref.alloc(winapi.PACL);
        daclP.ref().fill(0);
        var result = winapi.advapi32.GetSecurityInfo(pipeHandle, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
            ref.NULL, ref.NULL, daclP, ref.NULL, ref.NULL);

        var dacl = daclP.deref();
        if (result) {
            throw winapi.error("GetSecurityInfo", result);
        }

        // Add the user to the ACL.
        var access = new winapi.EXPLICIT_ACCESS();
        access.ref().fill(0);
        access.grfAccessMode = GRANT_ACCESS;
        access.grfAccessPermissions = (FILE_GENERIC_READ | FILE_GENERIC_WRITE);
        access.grfInheritance = 0;

        access.Trustee.pMultipleTrustee = ref.NULL;
        access.Trustee.MultipleTrusteeOperation = 0;
        access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        access.Trustee.TrusteeType = TRUSTEE_IS_USER;
        access.Trustee.ptstrName = sid;

        var newDacl = ref.alloc(winapi.PACL);
        newDacl.ref().fill(0);
        result = winapi.advapi32.SetEntriesInAclW(1, access.ref(), dacl, newDacl);
        if (result) {
            throw winapi.error("SetEntriesInAclW", result);
        }

        // Set the ACL.
        result = winapi.advapi32.SetSecurityInfo(pipeHandle, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
            ref.NULL, ref.NULL, newDacl.deref(), ref.NULL);

        if (result) {
            throw winapi.error("SetSecurityInfo", result);
        }
    } finally {
        if (pipeHandle) {
            winapi.kernel32.CloseHandle(pipeHandle);
        }
    }
};

module.exports = windows;
