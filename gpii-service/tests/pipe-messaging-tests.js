/* Tests for pipe-messaging.js
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

"use strict";

var jqUnit = require("node-jqunit"),
    net = require("net"),
    EventEmitter = require("events");


var messaging = require("../shared/pipe-messaging.js");

var teardowns = [];

jqUnit.module("GPII pipe tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

// Test gotData by sending packets using difference chunk sizes.
jqUnit.test("Test gotData", function () {
    var fakeSocket = new EventEmitter();
    var session = messaging.createSession(fakeSocket);

    // Create a packet buffer.
    var createPacket = function (message) {
        var messageBuf = Buffer.from(JSON.stringify(message));
        var packet = Buffer.alloc(messageBuf.length + messaging.lengthByteCount);
        packet.writeUIntBE(messageBuf.length, 0, messaging.lengthByteCount);
        messageBuf.copy(packet, messaging.lengthByteCount);
        return packet;
    };

    var maxChunkSize = createPacket(100).length;
    jqUnit.expect((maxChunkSize - 1) * 3);

    var count = 0;
    session.on("message", function (msg) {
        jqUnit.assertDeepEq("message expected", {id: count++}, msg);
    });

    var packetNumber = 0;
    for (var chunkLen = 1; chunkLen < maxChunkSize; chunkLen++) {
        var data = Buffer.concat([
            createPacket({id: packetNumber++}),
            createPacket({id: packetNumber++}),
            createPacket({id: packetNumber++})
        ]);

        while (data.length > 0) {
            var chunk = data.slice(0, chunkLen);
            data = data.slice(chunkLen);
            session.gotData(chunk);
        }
    }
});

/**
 * Create a pipe.
 *
 * @param {String|Number} pipeName Name of the pipe, or port number.
 * @return {Promise} resolves with an object containing both ends of the pipe {server, client}.
 */
var createPipe = function (pipeName) {
    if (!pipeName) {
        pipeName = 0;
    }

    return new Promise(function (resolve) {
        var serverEnd = null, clientEnd = null;
        var connected = function () {
            if (serverEnd && clientEnd) {
                resolve({
                    server: serverEnd, client: clientEnd
                });
            }
        };

        var server = net.createServer(function (socket) {
            console.log("connected server");
            serverEnd = socket;
            connected();
        });

        server.listen(pipeName, function () {
            console.log("listening");

            net.connect(server.address(), function () {
                console.log("connected client");
                clientEnd = this;
                connected();
            });

        });

    });
};

// Tests sending and receiving a message to each end of a pipe.
jqUnit.asyncTest("Test session", function () {

    jqUnit.expect(2);
    var test = {
        clientMessage: { message1: "a" },
        serverMessage: { message2: "b" }
    };

    createPipe().then(function (pipe) {
        var server = messaging.createSession(pipe.server, "test1");
        var client = messaging.createSession(pipe.client, "test1");

        server.on("message", function (msg) {
            jqUnit.assertDeepEq("message from client", test.clientMessage, msg);
            server.sendMessage(test.serverMessage);
        });

        client.on("message", function (msg) {
            jqUnit.assertDeepEq("message from server", test.serverMessage, msg);
            jqUnit.start();
        });

        client.on("ready", function () {
            client.sendMessage(test.clientMessage);
        });

        client.on("error", jqUnit.fail);
        server.on("error", jqUnit.fail);
    }, jqUnit.fail);
});

// Tests connecting with the wrong session type.
jqUnit.asyncTest("Test session type mismatch", function () {

    var result = {
        server: false,
        client: false
    };

    var gotError = function (side, err) {
        var match = err && err.message && err.message.startsWith("Unexpected client session type");
        jqUnit.assertTrue(match, "Error must be the expected error for " + side + ": " + err);

        if (result[side]) {
            jqUnit.fail("Got more than one error for " + side);
        }

        result[side] = true;

        if (result.client && result.server) {
            jqUnit.start();
        }
    };

    createPipe().then(function (pipe) {
        var server = messaging.createSession(pipe.server, "session type");
        var client = messaging.createSession(pipe.client, "wrong session type");

        server.on("message", function (msg) {
            jqUnit.fail("Unexpected message to server: " + msg);
        });
        client.on("message", function (msg) {
            jqUnit.fail("Unexpected message to client: " + msg);
        });

        server.on("ready", function () {
            jqUnit.fail("Unexpected server.ready event");
        });
        client.on("ready", function () {
            jqUnit.fail("Unexpected client.ready event");
        });

        server.on("error", function (err) {
            gotError("server", err);
        });
        client.on("error", function (err) {
            gotError("client", err);
        });
    });
});

// Tests sending a request and resolving a promise when responded.
jqUnit.asyncTest("Test requests", function () {

    var tests = [
        {
            action: "resolve",
            reply: {value: "the response"},
            expectResolve: true
        },
        {
            action: "return",
            reply: {value: "the other response"},
            expectResolve: true
        },
        {
            action: "reject",
            reply: {value: "the rejection"},
            expectResolve: false
        },
        {
            action: "error",
            reply: "the error (Error object)",
            expectResolve: false
        },
        {
            action: "throw",
            reply: {value: "the error"},
            expectResolve: false
        }
    ];

    jqUnit.expect(tests.length * 5);
    var currentTest;
    var testIndex = 0;
    var suffix;

    createPipe().then(function (pipe) {

        // Reply to the request.
        var serverRequest = function (req) {
            req = Object.assign({}, req);
            jqUnit.assertTrue("received request should contain the 'request' field" + suffix, !!req.request);
            delete req.request;
            jqUnit.assertDeepEq("received request should match the sent request" + suffix, currentTest, req);
            switch (req.action) {
            case "resolve":
                return Promise.resolve(req.reply);
            case "return":
                return req.reply;
            case "reject":
                return Promise.reject(req.reply);
            case "throw":
                throw req.reply;
            case "error":
                throw new Error(req.reply);
            }
        };

        var clientRequest = function () {
            jqUnit.fail("Request to the client is unexpected" + suffix);
        };

        var server = messaging.createSession(pipe.server, "test1", serverRequest);
        var client = messaging.createSession(pipe.client, "test1", clientRequest);

        client.on("error", jqUnit.fail);
        server.on("error", jqUnit.fail);

        var sendRequest = function () {
            if (testIndex >= tests.length) {
                jqUnit.start();
                return;
            }

            suffix = " - testIndex=" + testIndex;
            currentTest = tests[testIndex++];

            var p = client.sendRequest(currentTest);
            jqUnit.assertTrue("sendRequest must return promise" + suffix, p && typeof(p.then) === "function");

            p.then(function (reply) {
                jqUnit.assertTrue("promise should resolve for this test" + suffix, currentTest.expectResolve);
                jqUnit.assertDeepEq("resolved value must be the expected" + suffix, currentTest.reply, reply);
                sendRequest();
            }, function (err) {
                jqUnit.assertFalse("promise should reject for this test" + suffix, currentTest.expectResolve);
                if (currentTest.action === "error") {
                    jqUnit.assertDeepEq("error message must be the expected" + suffix, currentTest.reply, err.message);
                } else {
                    jqUnit.assertDeepEq("rejection value must be the expected" + suffix, currentTest.reply, err);
                }
                sendRequest();
            });
        };

        client.on("ready", sendRequest);

    }, jqUnit.fail);
});
