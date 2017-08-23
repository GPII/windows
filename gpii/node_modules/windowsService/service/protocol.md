# GPII Service Protocol

** This doesn't exactly match the current implementation**

## Connecting (abstract/hand-waving)
* Service generates a URL.
* Service starts GPII, passing the URL to it.
* GPII connects to the URL.
* Service ensures the client is GPII.

## Connecting (via sockets)
* Service listens on an arbitrary port number, bound to 127.0.0.1.
* Service starts GPII, passing the port to it.
* GPII connects to the port, Service accepts.
* Service inspects the TCP table, to check the PID of the remote end is that of the GPII process.

Once the connection is established, there is no distinction between 'server' and 'client' - any end can initiate a request.

## Packets
Messages go through the socket in the following format:

```abnf
<packet>   = <length> <message>
<length>   = uint32le ; length of <message> (LE)
<message>  = json     ; The message
```

## Message

* Three types of message: `Request`, `Response`, and `Error`.
* A `Request` is responded to with either a `Response` or `Error`.
* Requests/responses are asynchronous - the order in which responses are returned doesn't matter.

### `Request`

Ask the remote end to perform some action/query.

```javascript
{
    request: "...",    // Something to uniquely identify the request.
    type: "...",       // The type of request.
    data: { ... }      // Additional data (optional).
}
```
A Request will be responded to with either a `Response` or `Error`.

### `Response`

A successful reply to a `Request`.

```javascript
{
    response: "...",   // The "request" value of the request this is message responding to.
    type: "...",       // Same as the request.
    data: { ... }      // Additional data (optional).
}
```

### `Error`
```javascript
{
    error: "...",      // The "request" value of the request this error is in response to.
    message: "error"
    data: { ... }      // Additional data (optional).
}
```

## Request types

### Service and application requests
### Service requests
### Application requests
#### `todo`

Request:
```javascript
{
    request: "...",
    type: "todo",
    data: {
        todo: "todo"
    }
}
```

Response:
```javascript
{
    response: "...",
    type: "todo",
    data: {
        todo: "todo"
    }
}
```

