/* Tests for gpii-client.js
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

var jqUnit = require("node-jqunit"),
    gpiiClient = require("../src/gpiiClient.js"),
    service = require("../src/service.js");

var teardowns = [];

jqUnit.module("GPII client tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

var gpiiClientTests = {};

gpiiClientTests.requestTests = [
    {
        id: "execute: simple command",
        action: "execute",
        data: {
            command: "whoami"
        },
        expect: {
            promise: {
                pid: /^[0-9]+$/
            }
        }
    },
    {
        id: "execute: bad command",
        action: "execute",
        data: {
            command: "gpii-test-bad-command"
        },
        expect: {
            promise: "reject"
        }
    },
    {
        id: "execute: wait",
        action: "execute",
        data: {
            command: "whoami",
            wait: true
        },
        expect: {
            promise: {
                code: 0
            }
        }
    },
    {
        id: "execute: wait + exit code 1",
        action: "execute",
        data: {
            command: "whoami",
            args: ["/bad-option"],
            wait: true
        },
        expect: {
            promise: {
                code: 1
            }
        }
    },
    {
        id: "execute: capture output",
        action: "execute",
        data: {
            command: "cmd.exe",
            // Send something to stdout and stderr
            args: ["/c", "echo hello stdout & echo hello stderr 1>&2"],
            options: {
                // Set the working directory to prevent cmd.exe complaining about being on a UNC path.
                cwd: process.env.SystemRoot || "C:\\"
            },
            wait: true,
            capture: true
        },
        expect: {
            promise: {
                code: 0,
                signal: null,
                output: {
                    stdout: "hello stdout \r\n",
                    stderr: "hello stderr \r\n"
                }
            }
        }
    },
    {
        id: "sign",
        action: "sign",
        data: {
            payload: "hello",
            keyName: "site"
        },
        // sha256-hmac(hello, testing.gpii.net)
        expect: "81ba311bd1c768eaeabccccc0c208bef1a3e3be1b1476b6e35a7c4464fba5bd5"
    },
    {
        id: "client credentials",
        action: "getClientCredentials",
        data: undefined,
        expect: {
            "client_id": "pilot-computer",
            "client_secret": "pilot-computer-secret"
        }
    }
];

/**
 * Check if all properties of expected are also in subject and are equal or match a regular expression, ignoring any
 * extra ones in subject.
 *
 * @param {Object} subject The object to check against
 * @param {Object} expected The object containing the values to check for.
 * @param {Number} maxDepth [Optional] How deep to check.
 * @return {Boolean} true if there's a match.
 */
gpiiClientTests.deepMatch = function (subject, expected, maxDepth) {
    var match = false;
    if (maxDepth < 0) {
        return false;
    } else if (!maxDepth && maxDepth !== 0) {
        maxDepth = 10;
    }

    if (!subject) {
        return subject === expected;
    }

    for (var prop in expected) {
        if (expected.hasOwnProperty(prop)) {
            var exp = expected[prop];
            if (["string", "number", "boolean"].indexOf(typeof(exp)) >= 0) {
                match = subject[prop] === exp;
            } else if (exp instanceof RegExp) {
                match = exp.test(subject[prop]);
            } else {
                match = gpiiClientTests.deepMatch(subject[prop], exp, maxDepth - 1);
            }
            if (!match) {
                break;
            }
        }
    }

    return match;
};

gpiiClientTests.assertDeepMatch = function (msg, expect, actual) {
    var match = gpiiClientTests.deepMatch(actual, expect);
    jqUnit.assertTrue(msg, match);
    if (!match) {
        console.log("expected:", expect);
        console.log("actual:", actual);
    }
};


jqUnit.asyncTest("Test request handlers", function () {

    var tests = gpiiClientTests.requestTests;
    jqUnit.expect(tests.length * 3);

    service.loadConfig();

    var testIndex = -1;
    var nextTest = function () {
        if (++testIndex >= tests.length) {
            jqUnit.start();
            return;
        }
        var test = tests[testIndex];

        var suffix = " - testIndex=" + testIndex + " (" + (test.id || test.action) + ")";

        var handler = gpiiClient.requestHandlers[test.action];

        jqUnit.assertEquals("request handler should be a function" + suffix, "function", typeof handler);

        var result = handler(test.data);

        if (test.expect.promise) {
            jqUnit.assertTrue("request handler should return a promise" + suffix,
                result && typeof(result.then) === "function");

            result.then(function (value) {
                gpiiClientTests.assertDeepMatch("request handler should resolve with the expected value" + suffix,
                    test.expect.promise, value);

                nextTest();
            }, function (err) {
                jqUnit.assertEquals("request handler should only reject if expected" + suffix,
                    test.expect.promise, "reject");

                if (test.expect.promise !== "reject") {
                    console.log("Rejection:", err);
                }

                nextTest();
            });
        } else {
            gpiiClientTests.assertDeepMatch("request handler should return the expected value" + suffix,
                test.expect, result);
            jqUnit.assert("balancing assert count");
            nextTest();
        }
    };

    nextTest();

});
