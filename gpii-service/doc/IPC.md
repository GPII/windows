# GPII Service + User process communications

## Overview

Service and user process IPC is performed through a named pipe.

The service opens a named pipe, starts the GPII user process (providing the pipe name) which connects to the pipe at
startup.

Once the user process has authenticated itself, a two-way asynchronous messaging session begins.


## Pipe initialisation

A randomly named pipe is opened by the service. The pipe is created with node's built-in `net.Server` and `net.Socket`
classes.

The sole purpose of a random name is to prevent name squatting. The pipe name isn't a secret, as any process can
enumerate all pipe names (eg, `powershell Get-ChildItem \\.\pipe\`).

The name is made up of random (filename safe) characters with a prefix of `\\.\pipe\gpii-`. When starting the user
process, the pipe name is passed via the `GPII_SERVICE_PIPE` environment variable. Only the random character suffix is
placed in this variable.

(For development, there's a special configuration that makes the pipe to always be named `\\.\pipe\gpii-gpii`, to allow
random user processes to connect without authentication or being started by the service).

When running as a Windows service, the default ACL for the pipe forbids connections from user-level processes, so the
ACL is modified to allow connections from the user the GPII process is running as.


## User process start

The service uses [CreateProcessAsUser](https://msdn.microsoft.com/library/ms682429) to start GPII in the context of the
current desktop user.

The shell is not used to perform the invocation, so the process ID returned is that of the GPII user process.

## Authentication

The service needs to know that the thing at the other end of the pipe is the same process which it had started.

When the user process connects to the pipe, the server sends a challenge (a number) on the pipe, which is responded to
out of band with the following technique:

The challenge is a handle to an [event object](https://docs.microsoft.com/windows/desktop/Sync/event-objects) (like
binary semaphore) created by service ([CreateEvent](https://msdn.microsoft.com/library/ms682396)). The service will
wait on this event, granting access to the pipe client when the event is signalled
([SetEvent](https://msdn.microsoft.com/library/ms686211)).

Normally, only the process which created the event can signal it (in general, all handles to things are tied to the
process that owns that handle). However, the service explicitly allows the child process to use this event via
[DuplicateHandle](https://msdn.microsoft.com/library/ms724251) which created another handle to the event that's owned by
the child process.

The handle is not secret, as it is only usable by the process that it's intended to be used by (even the service can not
use that handle). If another process connects to the pipe and receives the handle, signalling the event will not work.
Additionally, the GPII process will not be able to signal the event because it had not received the handle - and because
GPII will only connect to the pipe its told to, there can be no man-in-the-middle.

Pseudo-code:

```
pipeName = "\\.\pipe\gpii-" + random()

childProcess = CreateProcessAsUser(command=gpii-app, env={GPII_SERVICE_PIPE:pipeName})

eventHandle = CreateEvent()
childEventHandle = DuplicateHandle(sourceHandle=eventHandle, targetProcess=childProcess)

net.createServer().listen(pipeName) => {
    pipe.send(childEventHandle)
}

// client calls SetEvent(childEventHandle)

waitForEvent(eventHandle) => {
    // authenticated
    pipe.send("OK")
}
```

## Messaging session

After authentication, the messaging session begins. At this point, there is no concept of a 'server' and 'client'; each
endpoint can send and receive messages.


### Packets

At the lowest level, the packets are blocks of data with a length header.

```
<packet>  :=  <size> + <payload>
<size>    :=  Payload byte count, 32-bit unsigned int
<payload> :=  The message
```

### Messages

Messages are JSON objects, which can be of three different types: _Request_, _Response_, and _Error_. Any end can send
a _Request_, which is replied to by either a _Response_ or _Error_. Messages are asynchronous, and the order in which
messages are sent and received does not matter - while an endpoint waits for a reply to a request, it can still send and
receiving other messages.

#### Request

This message is sent when one endpoint wishes to instruct or interrogate the other endpoint. The implementation returns
a promise which resolves when a matching _Response_ is received, or rejects on an _Error_ message. 

```
{
    // A random string to uniquely identify this request.
    request: "<request id>",
    // Type of request
    requestType: "execute"
    // Additional fields, specific to the type of request.
}
```

#### Response

A successful reply to a _Request_.

```
{
    // The id of the request that's being replied to.
    response: "<request id>",
    // Response data
    data: ...
}
```

#### Error

An error reply to a _Request_.

```
{
    // The id of the request that's being replied to.
    error: "<request id>",
    // Response data
    data: ...
}
```

## Request types

### Handled by the gpii-service

These are defined in [service/src/gpiiClient.js](../src/gpiiClient.js).

#### execute

Starts an executable, in the context of the service.

```javascript
request = {
    // The "execute" request.
    requestType: "execute",
    
    // The executable to run.
    command: "application.exe",
    
    // Arguments to pass.
    args: [ "arg1", "arg2" ],
    
    // True to wait until the process to terminate before returning the response. Otherwise return when the process has
    // started.
    wait: false,
    
    // True to capture the output. Stdout and stderr will be provided in the response. This implies wait=true.
    capture: false,
    
    // Options for the service to pass to child_process.spawn()
    options: { }
    
};

response = {
    // The exit code, if request.wait==true
    code: number,
    // The output, if request.capture==true.
    output: {
        stdout: string,
        stderr: string,
    },
    // The pid of the process, if request.wait==false
    pid: number
};
```


### Handled by gpii-app

Defined in [gpii-service-handler/src/requestHandler.js](../../gpii/node_modules/gpii-service-handler/src/requestHandler.js)

#### status

Returns the status of the GPII process, to determine if GPII has became unresponsive. The service sends this request
and waits for a predefined timeout. If no response has been received in that time, then the gpii process it terminated
otherwise the request is repeated.

```javascript
request = {
    // The 'status' request.
    requestType: "status"
};

response = {
    // always true.
    isRunning: true
};
```

