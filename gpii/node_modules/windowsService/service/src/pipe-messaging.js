/* Handles the connection between the service and GPII user process.
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

var util = require("util"),
    EventEmitter = require("events");

var messaging = {};

/**
 * Creates a session with the given connected pipe (or socket).
 *
 * @param pipe {Socket} The pipe.
 * @param sessionType {String} [Optional] Initial text that is sent and checked by both ends to ensure both sides are
 * compatible.
 * @return {Session}
 */
messaging.createSession = function (pipe, sessionType) {
    return new Session(pipe, sessionType);
};

/**
 *
 * @param pipe {Socket} The pipe.
 * @constructor
 */
function Session(pipe, sessionType, isServer) {
    if (!(this instanceof Session)) {
        return new Session(pipe, sessionType, isServer);
    }

    this.pipe = pipe;

    this.sessionType = sessionType || "gpii-pipe";
    this.sessionTypeChecked = false;

    this.sendMessage(this.sessionType);
    this.buffer = null;
    this.payloadLength = null;

    pipe.on("data", function (data) {
        this.gotData(data);
    });
    pipe.on("error", function (err) {
        console.log(err);
        this.on("error", err);
    });
    pipe.on("end", function () {
        this.on("close");
    });
    pipe.on("close", function () {
        this.on("close");
    });
};

util.inherits(Session, EventEmitter);

// Number of bytes used for the message length.
messaging.lengthByteCount = 2;

/**
 * Handle the pipe's "data" event.
 *
 * Messages may not arrive in a single chunk, so the data is added to a buffer until there is enough data to process.
 *
 * @param data {Buffer}
 */
Session.prototype.gotData = function (data) {
    this.buffer = Buffer.concat(this.buffer, data);

    if (!this.payloadLength) {
        // The first bytes are the length of the payload.
        if (this.buffer.length >= messaging.lengthByteCount) {
            this.payloadLength = this.buffer.readUIntBE(0, messaging.lengthByteCount);
            this.buffer = this.buffer.slice(messaging.lengthByteCount);
        }
    }

    if (this.buffer.length >= this.payloadLength) {
        this.gotMessage(this.buffer.toString("utf8", 0, this.payloadLength));
        this.buffer = this.buffer.slice(this.payloadLength);
    }
};

/**
 * Called when a message has been received.
 * @param message
 */
Session.prototype.gotMessage = function (message) {
    if (this.sessionTypeChecked) {
        var messageObject = JSON.parse(message);
        this.emit("message", messageObject);
    } else if (message === this.sessionType) {
        this.sessionTypeChecked = true;
    } else {
        this.pipe.end();
        this.emit("error", new Error("Unexpected client session type " + message));
    }
};

/**
 * Sends a message to the pipe.
 *
 * @param payload {String|Object|Buffer} The message payload.
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

module.exports = messaging;
