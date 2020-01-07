/* Handles the connection between the service and GPII user process.
 *
 * Copyright 2018 Raising the Floor - International
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

/**
 * Message protocol:
 * packet   :=  size + payload
 * size     :=  sizeof(payload) (32-bit uint)
 * payload  :=  The message.
 *
 * Message is a string, JSON, or buffer. Object can look like the following:
 *
 * requests:
 * {
 *   request: "<request-id>",
 *   data: { ... }
 * }
 *
 * responses:
 * {
 *   response: "<request-id>",
 *   data: { ... }
 * }
 *
 * error responses:
 * {
 *   error: "<request-id>",
 *   data: { ... }
 * }
 *
 */
"use strict";

var util = require("util"),
    EventEmitter = require("events");

var messaging = {};

/**
 * Creates a session with the given connected pipe (or socket).
 *
 * This wraps a pipe, which fires a `message` event when for every JSON object received.
 *
 * @param {Socket} pipe The pipe.
 * @param {String} sessionType [Optional] Initial text that is sent and checked by both ends to ensure both sides are
 *  compatible.
 * @param {Function} requestCallback [Optional] Function to call when a request has been received. The function should
 *  return the result, or a promise resolving to the result.
 * @param {Buffer} initialData [Optional] Initial data.
 * @return {Session} The new session instance.
 */
messaging.createSession = function (pipe, sessionType, requestCallback, initialData) {
    return new Session(pipe, sessionType, requestCallback, initialData);
};

/**
 * Wraps a pipe with a session.
 *
 * @param {Socket} pipe The pipe.
 * @param {String} sessionType [Optional] Initial text that is sent and checked by both ends to ensure both sides are
 *  compatible.
 * @param {Function} requestCallback [Optional] Function to call when a request has been received. The function should
 *  return the result, or a promise resolving to the result.
 * @param {Buffer} initialData [Optional] Initial data.
 * @constructor
 */
function Session(pipe, sessionType, requestCallback, initialData) {
    this.pipe = pipe;

    this.sessionType = sessionType;
    this.sessionTypeChecked = !this.sessionType;
    if (this.sessionType) {
        this.sendMessage(this.sessionType);
    }

    this.requestCallback = requestCallback;
    this.promises = {};
    this.buffer = null;
    this.payloadLength = null;
    var session = this;

    // Read the initial data (if any) in the next tick, because it will emit events which aren't yet bound to.
    process.nextTick(function () {
        if (initialData) {
            session.gotData(initialData);
        }

        pipe.on("data", function (data) {
            session.gotData(data);
        });

        pipe.on("close", function (hadError) {
            session.emit("close", hadError);
        });
    });
}

util.inherits(Session, EventEmitter);

// Number of bytes used for the message length.
messaging.lengthByteCount = 2;

messaging.Session = Session;

/**
 * Sends a message to the pipe.
 *
 * @param {String|Object|Buffer} payload The message payload.
 */
Session.prototype.sendMessage = function (payload) {
    var payloadBuf;
    if (Buffer.isBuffer(payload)) {
        payloadBuf = payload;
    } else {
        var payloadString;
        if (typeof(payload) === "string") {
            payloadString = payload;
        } else {
            payloadString = JSON.stringify(payload);
        }
        payloadBuf = Buffer.from(payloadString);
    }

    // <message> = <length> + <payload>
    var message = Buffer.alloc(payloadBuf.length + messaging.lengthByteCount);
    message.writeUIntBE(payloadBuf.length, 0, messaging.lengthByteCount);
    payloadBuf.copy(message, messaging.lengthByteCount);

    this.pipe.write(message, "utf8");
};

/**
 * Handle the pipe's "data" event.
 *
 * Messages may not arrive in a single chunk, so the data is added to a buffer until there is enough data to process.
 *
 * packet   :=  size + payload
 * size     :=  sizeof(payload) (32-bit uint)
 * payload  :=  The message.
 *
 * @param {Buffer} data The data.
 */
Session.prototype.gotData = function (data) {
    if (data) {
        this.buffer = this.buffer ? Buffer.concat([this.buffer, data]) : data;
    }

    if (!this.payloadLength) {
        // The first bytes are the length of the payload.
        if (this.buffer.length >= messaging.lengthByteCount) {
            this.payloadLength = this.buffer.readUIntBE(0, messaging.lengthByteCount);
            this.buffer = this.buffer.slice(messaging.lengthByteCount);
        }
    }

    if (this.payloadLength && this.buffer.length >= this.payloadLength) {
        // Got enough data for the payload.
        this.gotPacket(this.buffer.toString("utf8", 0, this.payloadLength));
        this.buffer = this.buffer.slice(this.payloadLength);
        this.payloadLength = 0;

        if (this.buffer.length > 0) {
            // Still data after the payload, process it now
            this.gotData(null);
        }
    }
};

/**
 * Called when a message has been received.
 *
 * @param {String} packet The JSON string of the message.
 */
Session.prototype.gotPacket = function (packet) {
    if (this.sessionTypeChecked) {
        var message = JSON.parse(packet);
        this.emit("message", message);

        if (message.request) {
            this.handleRequest(message);
        } else if (message.response || message.error) {
            this.handleReply(message);
        }
    } else if (packet === this.sessionType) {
        this.sessionTypeChecked = true;
        this.emit("ready");
    } else {
        this.pipe.end();
        this.emit("error", new Error("Unexpected client session type " + packet));
    }
};

/**
 * Call the request callback when a request is received, and sends the response.
 *
 * @param {Object} message The request message.
 */
Session.prototype.handleRequest = function (message) {
    var session = this;
    var promise;
    try {
        var result = this.requestCallback && this.requestCallback(message);
        promise = Promise.resolve(result);
    } catch (e) {
        promise = Promise.reject(e);
    }

    promise.then(function (result) {
        session.sendMessage({
            response: message.request,
            data: result
        });
    }, function (err) {
        var e = null;
        if (err instanceof Error) {
            // Error doesn't serialise (nor does it work with Object.assign)
            e = {};
            Object.getOwnPropertyNames(err).forEach(function (a) {
                e[a] = err[a];
            });
        }
        session.sendMessage({
            error: message.request,
            data: e || err
        });
    });
};

/**
 * Resolves (or rejects) the promise for a request.
 *
 * @param {Object} message The response object.
 */
Session.prototype.handleReply = function (message) {
    // Resolve or reject the promise that is waiting on the result.
    var id = message.response || message.error;
    var promise = id && this.promises[id];

    if (promise) {
        if (message.response) {
            promise.resolve(message.data);
        } else {
            promise.reject(message.data);
        }
        delete this.promises[id];
    }
};

/**
 * Send a request.
 *
 * @param {ServiceRequest} request The request data.
 * @return {Promise} Resolves when the response has been received, rejects on error.
 */
Session.prototype.sendRequest = function (request) {
    var session = this;
    return new Promise(function (resolve, reject) {

        // Store the resolve/reject methods so they can be called when there's a response.
        var requestId = Math.random().toString(36).substring(2);
        session.promises[requestId] = {
            resolve: resolve,
            reject: reject
        };

        var message = Object.assign({}, request);
        message.request = requestId;
        session.sendMessage(message);
    });
};

module.exports = messaging;
