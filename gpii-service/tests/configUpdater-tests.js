/* Tests for configUpdater.js
 *
 * Copyright 2019 Raising the Floor - International
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
    os = require("os"),
    http = require("http"),
    JSON5 = require("json5"),
    path = require("path"),
    URL = require("url").URL,
    fs = require("fs");

var configUpdater = require("../src/configUpdater.js");

var teardowns = [];

jqUnit.module("GPII configUpdater tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

var configUpdaterTests = {};

configUpdaterTests.testDateOld = "Fri, 31 Dec 1999 23:59:59 GMT";
configUpdaterTests.testDate = "Wed, 01 Jan 2020 12:34:56 GMT";

configUpdaterTests.validateJSONTests = [
    {
        id: "JSON",
        input: "{\"key\": \"value\"}",
        expect: true
    },
    {
        id: "JSON5",
        input: "/* */ {key: \"value\",}",
        expect: true
    },
    {
        id: "invalid",
        input: ":}",
        expect: false
    },
    {
        id: "no file",
        input: null,
        expect: "reject"
    }
];

configUpdaterTests.hashTests = [
    {
        id: "valid",
        input: "test subject",
        algorithm: undefined,
        expect: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072"
    },
    {
        id: "valid md5",
        input: "test subject",
        algorithm: "md5",
        expect: "850be8166b39e19a6ff11e998509954a"
    },
    {
        id: "no file",
        input: null,
        expect: "reject"
    }
];

configUpdaterTests.expandTests = [
    {
        obj: {
            a: "A",
            b: {
                a: "BA",
                b: "BB"
            },
            c: {
                b: "CB",
                c: {
                    d: "CCD"
                }
            },
            d: "${a}",
            e: "${c.b}",
            f: "123${c.b}"
        },
        tests: {
            "": "",
            "a": "a",
            "${a}": "A",
            "${a}${a}": "AA",
            "1${a}2${c.b}3": "1A2CB3",
            "${b.a}": "BA",
            "${b.x}": ["", null],
            "12${b.x}34": ["1234", null],
            "12${a}34${b.x}56": ["12A3456", null],
            "${c.c.d}": "CCD",
            "${b.c.d}": ["", null],

            "${d}": "${a}",
            "${e}": "${c.b}",
            "${f}": "123${c.b}",

            "${a?XX}": "A",
            "${a}?": "A?",
            "${a}?}": "A?}",
            "${a?XX}${a?XX}": "AA",
            "1${a?XX}2${c.b?XX}3": "1A2CB3",
            "${b.a?XX}": "BA",
            "${b.x?XX}": "XX",
            "${c.c.d?XX}": "CCD",
            "${c?XX}": "XX",
            "${c.a.a?XX}": "XX",
            "${c.a.a?X.X}": "X.X",
            "${c.a.a?X?X}": "X?X",
            "${c.a.a?X.X?X.X}": "X.X?X.X",
            "${c.a.a??}": "?",

            "${?}": "",
            "${?X}": "X",

            "${a": "${a",
            "${a?": "${a?",
            "$": "$",
            "$${a}": "$A",
            "${}": ["", null]
        }
    },
    {
        obj: {},
        tests: {
            "": "",
            "a": "a",
            "${a}": ["", null],
            "${a.b.c}": ["", null],
            "${a?}": "",
            "${a?XX}": "XX"
        }
    },
    {
        obj: null,
        tests: {
            "": "",
            "a": "a",
            "${a}": ["", null],
            "${a.b.c}": ["", null],
            "${a?XX}": "XX"
        }
    }
];

configUpdaterTests.downloadTests = [
    {
        id: "success",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072"
    },
    {
        id: "etag in response",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072",
        etagResponse: "\"1234\"",
        etagExpect: "1234"
    },
    {
        id: "weak etag in response",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072",
        etagResponse: "W/\"ABCD\"",
        etagExpect: undefined
    },
    {
        id: "etag in request",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072",
        options: {
            etag: "1234"
        }
    },
    {
        id: "date",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072",
        options: {
            date: configUpdaterTests.testDate
        }
    },
    {
        id: "date + etag",
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072",
        options: {
            date: configUpdaterTests.testDate,
            etag: "1234"
        }
    },
    {
        id: "overwrite destination",
        overwrite: true,
        content: "test subject",
        hash: "0ec931a49e2dcfb265aa2b554be7d5d9c1b39e46dd81e24048796a54f32c361ff83f1dcec09162dc896f86cfacbb88f11c4a5a5365b556fda597efd8f77c6072"
    },
    {
        id: "success (this file)",
        file: __filename
    },
    {
        id: "success (large file)",
        file: path.join(process.env.SystemRoot, "system32/WindowsCodecsRaw.dll")
    },
    {
        id: "error: 404",
        content: "error",
        statusCode: 404,
        expect: "reject"
    },
    {
        id: "no update: 304",
        hash: undefined,
        content: "not updated",
        statusCode: 304,
        expect: null
    },
    {
        id: "bad destination",
        content: "error",
        destination: path.join(os.tmpdir(), "does/not/exist"),
        expect: "reject"
    },
    {
        id: "ssl error",
        url: "https://untrusted-root.badssl.com/",
        expect: "reject"
    },
    {
        id: "remote server",
        url: "https://raw.githubusercontent.com/GPII/windows/8152ce42da1091268c18f3f9e0f7d66a16cbaf32/README.md",
        hash: "8c57e997c3cb28f6078648b47c881ef4e5b833f2f427d30e1c246b0e790decda8ab53c474d131b2500c26a2e16c1918b81caad5ed3cf975584f1eeed5f601c2f"
    }
];

configUpdaterTests.applyUpdateTests = [
    {
        id: "update (no current file)",
        input: {
            source: "new",
            destination: null,
            backup: null
        },
        expect: {
            source: null,
            destination: "new",
            backup: null
        }
    },
    {
        id: "update",
        input: {
            source: "new",
            destination: "current",
            backup: null
        },
        expect: {
            source: null,
            destination: "new",
            backup: "current"
        }
    },
    {
        id: "update, existing backup",
        input: {
            source: "new",
            destination: "current",
            backup: "old"
        },
        expect: {
            source: null,
            destination: "new",
            backup: "current"
        }
    },
    {
        id: "update, no current file, existing backup",
        input: {
            source: "new",
            destination: null,
            backup: "old"
        },
        expect: {
            source: null,
            destination: "new",
            backup: "old"
        }
    }
];

configUpdaterTests.updateFileTests = [
    {
        id: "not updated previously",
        input: {
            // No last update info
            lastUpdate: {},
            content: "old"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": undefined,
                "if-none-match": undefined
            },
            content: "new"
        }
    },
    {
        id: "updated (no ETag)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new"
        }
    },
    {
        id: "updated (ETag)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the ETag value",
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new"
        }
    },
    {
        id: "updated (new ETag)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the new ETag value",
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "new"
        }
    },
    {
        id: "updated (new ETag, same content)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "new"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the new ETag value",
                previous: false
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "new"
        }
    },
    {
        id: "always update",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the old ETag value"
            },
            content: "old",
            always: true
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the new ETag value",
                previous: true
            },
            request: {
                "if-modified-since": undefined,
                "if-none-match": undefined
            },
            content: "new"
        }
    },
    {
        id: "always update (not required)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the new ETag value"
            },
            content: "new",
            always: true
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: "the new ETag value"
                // no previous field - the content hasn't changed, so back-up is not required.
            },
            request: {
                "if-modified-since": undefined,
                "if-none-match": undefined
            },
            content: "new"
        }
    },
    {
        id: "no update",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old"
        },
        response: {
            statusCode: 304,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: null
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "old"
        }
    },
    {
        id: "no update (no url)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old",
            url: ""
        },
        response: null,
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            request: null,
            content: "old"
        }
    },
    {
        id: "no update (with body)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old"
        },
        response: {
            statusCode: 304,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value",
                previous: false
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "old"
        }
    },
    {
        id: "updated (JSON)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            isJSON: true
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "{\"key\": \"value\",\n\"key2\":123\n}"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "{\"key\": \"value\",\n\"key2\":123\n}"
        }
    },
    {
        id: "updated (JSON5)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            isJSON: true
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "/* JSON5 */ {key: \"value\",\nkey2:123,\n}"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "/* JSON5 */ {key: \"value\",\nkey2:123,\n}"
        }
    },
    {
        id: "updated (invalid JSON/JSON5)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            isJSON: true
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "} invalid JSON"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "old"
        }
    },
    {
        id: "error",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old"
        },
        response: {
            statusCode: 418,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: null
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value",
                previous: false
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "old"
        }
    },
    {
        id: "error (with body)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old"
        },
        response: {
            statusCode: 418,
            headers: {
                "Last-Modified": configUpdaterTests.testDate,
                ETag: "\"the new ETag value\""
            },
            body: "error message"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value",
                previous: false
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": "\"the last ETag value\""
            },
            content: "old"
        }
    },
    {
        id: "updated (expander in url)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: "${site}"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?testing.gpii.net"
        }
    },
    {
        id: "updated (expander in url, ${version})",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: "${version}"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?testing-1.2.3"
        }
    },
    {
        id: "updated (expander in url, ${siteConfig.xxx})",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: "${siteConfig.someValue}"
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?value-from-site-config"
        }
    },
    {
        id: "updated (multiple urls, no expanders, uses first)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: ["it-worked", "you-broke-it"]
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?it-worked"
        }
    },
    {
        id: "updated (multiple urls, with expander, uses first)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: ["it-worked-${site}", "you-broke-it"]
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?it-worked-testing.gpii.net"
        }
    },
    {
        id: "updated (multiple urls, with expander, uses third)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld
            },
            content: "old",
            urlQuery: [
                "you-broke-it-${stupidValue}",
                "you-broke-it-again-${siteConfig.stupid}",
                "it-worked-${site}",
                "you-broke-it-last"
            ]
        },
        response: {
            statusCode: 200,
            headers: {
                "Last-Modified": configUpdaterTests.testDate
            },
            body: "new"
        },
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDate,
                etag: undefined,
                previous: true
            },
            request: {
                "if-modified-since": configUpdaterTests.testDateOld,
                "if-none-match": undefined
            },
            content: "new",
            search: "?it-worked-testing.gpii.net"
        }
    },
    {
        id: "no update (multiple urls)",
        input: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            content: "old",
            url: ["", "${stupidValue}", ""]
        },
        response: null,
        expect: {
            lastUpdate: {
                date: configUpdaterTests.testDateOld,
                etag: "the last ETag value"
            },
            request: null,
            content: "old"
        }
    }
];

configUpdaterTests.updateAllTest = {
    input: {
        lastUpdates: {
            files: {
                file1: {
                    id: "1"
                },
                file3: {
                    id: "3"
                },
                file4: {
                    id: "4"
                },
                file5: {
                    id: "5"
                }
            }
        },
        autoUpdateFiles: [
            {url: "1", path: "file1"},
            {url: undefined, path: "file2"},
            {url: "3", path: "file3", error: true},
            {url: "4", path: "file4"},
            {url: "5", path: "file5", retry: 3}
        ]
    },
    expect: {
        updateAttempts: [
            "file1",
            "file2",
            "file3", "file3", "file3", "file3", "file3", "file3",
            "file4",
            "file5", "file5", "file5"
        ],
        lastUpdates: {
            files: {
                file1: {
                    id: "1",
                    handled: true
                },
                file3: {
                    id: "3"
                },
                file4: {
                    id: "4",
                    handled: true
                },
                file2: {
                    handled: true
                },
                file5: {
                    id: "5",
                    handled: true
                }
            }
        }
    }
};

/**
 * Gets a temporary file name, and deletes it at the end of the current test.
 * @param {String} testName Name of the current test.
 * @return {String} Path to the temporary file.
 */
configUpdaterTests.getTempFile = function (testName) {
    var tempFile = path.join(os.tmpdir(), "gpii-test-" + testName + Math.random());

    teardowns.push(function () {
        try {
            fs.unlinkSync(tempFile);
        } catch (e) {
            // ignore
        }
    });

    return tempFile;
};

/**
 * Asserts that a file's content matches what is expected.
 *
 * @param {String} message The assertion message.
 * @param {String} expected The expected content, or null if the file is not expected to exist.
 * @param {String} path The file to check.
 */
configUpdaterTests.assertFile = function (message, expected, path) {
    var content ;
    try {
        content = fs.readFileSync(path, "utf8");
    } catch (err) {
        if (err.code === "ENOENT") {
            content = null;
        } else {
            jqUnit.fail(err);
            content = err.message;
        }
    }
    jqUnit.assertEquals(message, expected || "(no file)", content || "(no file)");
};

jqUnit.asyncTest("Test validateJSON", function () {

    jqUnit.expect(configUpdaterTests.validateJSONTests.length);

    // For each test, write to a file and check the result of validateJSON.
    var promises = configUpdaterTests.validateJSONTests.map(function (test) {
        var tempFile = configUpdaterTests.getTempFile("validateJSON");

        if (test.input !== null) {
            fs.writeFileSync(tempFile, test.input);
        }

        var suffix = " - test.id: " + test.id;
        return configUpdater.validateJSON(tempFile).then(function (value) {
            jqUnit.assertEquals("validateJSON should resolve with expected value" + suffix, test.expect, value);
        }, function (err) {
            if (test.expect !== "reject") {
                console.log(err);
            }
            jqUnit.assertEquals("validateJSON rejected" + suffix, "reject", test.expect);
        });
    });

    Promise.all(promises).then(jqUnit.start, jqUnit.fail);
});

jqUnit.asyncTest("Test hashFile", function () {

    jqUnit.expect(configUpdaterTests.hashTests.length);

    // For each test, write to a file and check the result of hashFile.
    var promises = configUpdaterTests.hashTests.map(function (test) {
        var tempFile = configUpdaterTests.getTempFile("hashFile");

        if (test.input !== null) {
            fs.writeFileSync(tempFile, test.input);
        }

        var suffix = " - test.id: " + test.id;
        return configUpdater.hashFile(tempFile, test.algorithm).then(function (value) {
            jqUnit.assertEquals("hashFile should resolve with expected value" + suffix, test.expect, value);
        }, function (err) {
            if (test.expect !== "reject") {
                console.log(err);
            }
            jqUnit.assertEquals("hashFile rejected" + suffix, "reject", test.expect);
        });
    });

    Promise.all(promises).then(jqUnit.start, jqUnit.fail);
});

jqUnit.test("expand tests", function () {
    configUpdaterTests.expandTests.forEach(function (test) {
        console.log("expand object:", test.obj);
        for (var unexpanded in test.tests) {
            var expect = test.tests[unexpanded];

            if (!Array.isArray(expect)) {
                expect = [expect, expect];
            }

            var result = configUpdater.expand(unexpanded, test.obj, true);
            jqUnit.assertEquals("result of expand(alwaysExpand=true) must match the expected result for " + unexpanded,
                expect[0], result);

            var result2 = configUpdater.expand(unexpanded, test.obj, false);
            jqUnit.assertEquals("result of expand(alwaysExpand=false) must match the expected result for " + unexpanded,
                expect[1], result2);
        }
    });
});

jqUnit.asyncTest("Test downloadFile", function () {

    jqUnit.expect(configUpdaterTests.downloadTests.length * 3);

    var server = http.createServer();
    server.listen(0, "127.0.0.1");

    server.on("request", function (req, res) {
        var index = parseInt(req.url.substr(1));
        var test = configUpdaterTests.downloadTests[index];

        if (test.statusCode) {
            res.statusCode = test.statusCode;
        }
        if (test.etagResponse) {
            res.setHeader("ETag", test.etagResponse);
        }

        jqUnit.expect(2);
        var suffix = " - test.id: " + test.id;

        // Check the ETag in the request.
        if (test.options && test.options.etag) {
            var expectETag = "\"" + test.options.etag + "\"";
            jqUnit.assertEquals("If-None-Match header must match the ETag" + suffix,
                expectETag, req.headers["if-none-match"]);
        } else {
            jqUnit.assertNull("If-None-Match header should not have been sent" + suffix,
                req.headers["if-none-match"]);
        }

        // Check the date in the request.
        if (test.options && test.options.date) {
            jqUnit.assertEquals("If-Modified-Since header must match the date" + suffix,
                test.options.date, req.headers["if-modified-since"]);
        } else {
            jqUnit.assertNull("If-Modified-Since header should not have been sent" + suffix,
                req.headers["if-modified-since"]);
        }

        // Send the response data.
        if (test.file) {
            fs.createReadStream(test.file).pipe(res);
        } else {
            res.end(test.content);
        }
    });

    server.on("listening", function () {
        var localUrl = "http://" + server.address().address + ":" + server.address().port + "/";
        console.log("http server listening on " + localUrl);

        // For each test, write to a file and check the result of hashFile.
        var promises = configUpdaterTests.downloadTests.map(function (test, index) {
            var destination;
            if (test.destination) {
                destination = test.destination;
            } else {
                destination = configUpdaterTests.getTempFile("downloadFile");

                if (test.overwrite) {
                    // Testing overwriting an existing file; create one.
                    fs.writeFileSync(destination, "existing content");
                }
            }

            var url = test.url || (localUrl + index);

            var suffix = " - test.id: " + test.id;
            return configUpdater.downloadFile(url, destination, test.options).then(function (value) {
                // Check the resolve value
                if (test.hasOwnProperty("hash")) {
                    jqUnit.assertEquals("downloadFile should resolve with expected value.hash" + suffix,
                        test.hash, value.hash);
                } else {
                    // Hash can't be precalculated, just check if it looks like one.
                    var isValid = /^[a-f0-9]{128}$/i.test(value.hash);
                    jqUnit.assertTrue("downloadFile should resolve with a hash" + suffix, isValid);
                }

                // Check the ETag header has been read correctly
                if (test.hasOwnProperty("etagExpect")) {
                    jqUnit.assertEquals("downloadFile should resolve with expected value.etag" + suffix,
                        test.etagExpect, value.etag);
                } else {
                    jqUnit.assert("balancing assert count");
                }

                if (value.hash) {
                    // Check the content (downloadFile does do this, however let's check if it's written to the right file).
                    return configUpdater.hashFile(destination).then(function (hash) {
                        jqUnit.assertEquals("downloadFile content should be what's expected" + suffix,
                            test.hash || value.hash, hash);
                    });
                } else {
                    // Download did not provide a hash (304 response)
                    jqUnit.assertEquals("downloadFile should not return a hash for 304 response", 304, test.statusCode);
                }
            }, function (err) {
                console.log("this error is probably expected:", err);
                jqUnit.assertEquals("downloadFile rejected" + suffix, "reject", test.expect);
                // This execution branch has 2 less asserts than the other.
                jqUnit.expect(-2);
            });
        });
        Promise.all(promises)["catch"](jqUnit.fail).then(function () {
            jqUnit.start();
        });
    });

    teardowns.push(function () {
        server.close();
        server.unref();
    });

});

jqUnit.asyncTest("Test load/save last updates file", function () {
    jqUnit.expect(4);

    // Test saving
    var tempFile = configUpdaterTests.getTempFile("lastUpdates");

    var original = {
        lastCheck: "1999-12-31T12:34:56.789Z",
        files: {
            file1: {path: "first"},
            file2: {path: "second"}
        }
    };

    var dateBefore = new Date().toISOString();

    configUpdater.saveLastUpdates(original, tempFile).then(function () {
        // Read and parse the written content (the content doesn't matter, as long as it parses back to the original)
        var writtenContent = fs.readFileSync(tempFile, "utf8");
        var written = JSON5.parse(writtenContent);

        // Test the lastCheck has been updated (any time during saveLastUpdates call)
        var dateAfter = new Date().toISOString();
        jqUnit.assertTrue("lastCheck field should be updated",
            written.lastCheck >= dateBefore && written.lastCheck <= dateAfter);

        // Set it back to the original, just to force it through the next assert.
        var lastCheck = written.lastCheck;
        written.lastCheck = original.lastCheck;

        jqUnit.assertDeepEq("saveLastUpdates should have written the original data", original, written);

        // Test loading
        return configUpdater.loadLastUpdates(tempFile).then(function (loaded) {
            jqUnit.assertDeepEq("Re-loaded lastCheck field should match the new one", lastCheck, loaded.lastCheck);
            loaded.lastCheck = original.lastCheck;
            jqUnit.assertDeepEq("Re-loaded object should match the original object", original, loaded);
        });
    })["catch"](jqUnit.fail).then(jqUnit.start);
});

jqUnit.asyncTest("Test load last updates file (failures)", function () {

    jqUnit.expect(2);

    var badFile = configUpdaterTests.getTempFile("badLastUpdate");
    fs.writeFileSync(badFile, "}:");

    // Skeleton lastUpdated object. loadLastUpdates should return this if there's no current file.
    var expect = {
        files: {}
    };

    var tests = [
        configUpdater.loadLastUpdates(badFile).then(function (loaded) {
            jqUnit.assertDeepEq("loadLastUpdates(badFile) should return skeleton data", expect, loaded);
        }),
        configUpdater.loadLastUpdates("/does/not/exist").then(function (loaded) {
            jqUnit.assertDeepEq("loadLastUpdates(non-exisiting file) should return skeleton data", expect, loaded);
        })
    ];

    Promise.all(tests)["catch"](jqUnit.fail).then(jqUnit.start);
});

jqUnit.asyncTest("Test save last updates file (failures)", function () {

    jqUnit.expect(1);
    configUpdater.saveLastUpdates({}, "/").then(function () {
        jqUnit.fail("saveLastUpdates should not resolve on error");
    }, function () {
        jqUnit.assert("saveLastUpdates should reject on error");
    }).then(jqUnit.start);
});

jqUnit.asyncTest("Test applyUpdate", function () {
    jqUnit.expect(configUpdaterTests.applyUpdateTests.length * 3);

    var promises = configUpdaterTests.applyUpdateTests.map(function (test) {
        var sourceFile = configUpdaterTests.getTempFile("applyUpdate-source");
        var destinationFile = configUpdaterTests.getTempFile("applyUpdate-dest");
        var backupFile = destinationFile + ".previous";

        if (test.input.source !== null) {
            fs.writeFileSync(sourceFile, test.input.source);
        }
        if (test.input.destination !== null) {
            fs.writeFileSync(destinationFile, test.input.destination);
        }
        if (test.input.backup !== null) {
            fs.writeFileSync(backupFile, test.input.backup);
        }

        var suffix = " - test.id: " + test.id;
        return configUpdater.applyUpdate(sourceFile, destinationFile).then(function () {

            configUpdaterTests.assertFile("source file should match expected" + suffix, test.expect.source, sourceFile);
            configUpdaterTests.assertFile("destination file should match expected" + suffix,
                test.expect.destination, destinationFile);
            configUpdaterTests.assertFile("backup file should match expected" + suffix, test.expect.backup, backupFile);
        });
    });

    Promise.all(promises)["catch"](jqUnit.fail).then(jqUnit.start);
});

jqUnit.asyncTest("Test updateFile", function () {

    jqUnit.expect(configUpdaterTests.updateFileTests.length * 3);

    // Mock getDateString so it returns a predictable date.
    var getDateStringOrig = configUpdater.getDateString;
    configUpdater.getDateString = function () {
        return configUpdaterTests.testDate;
    };
    teardowns.push(function () {
        configUpdater.getDateString = getDateStringOrig;
    });

    var server = http.createServer();
    server.listen(0, "127.0.0.1");
    var localUrl;

    // Respond to requests using the details from the test's response object.
    server.on("request", function (req, res) {
        var url = new URL(localUrl + req.url.substr(1));
        var index = parseInt(url.pathname.substr(1));
        var test = configUpdaterTests.updateFileTests[index];
        var suffix = " - test.id: " + test.id;

        if (url.search || test.expect.search) {
            jqUnit.expect(1);
            jqUnit.assertEquals("URL search must match the expected value" + suffix,
                test.expect.search, url.search);
        }
        // Check the request headers
        var checkHeaders = {};
        for (var key in test.expect.request) {
            checkHeaders[key] = req.headers[key];
        }

        jqUnit.assertDeepEq("All expected headers must match the requested headers" + suffix,
            test.expect.request, checkHeaders);

        // Set the headers
        if (test.response.headers) {
            for (var header in test.response.headers) {
                res.setHeader(header, test.response.headers[header]);
            }
        }
        console.log(req.url, req.headers);
        res.statusCode = test.response.statusCode;

        res.end(test.response.body);
    });

    server.on("listening", function () {
        localUrl = "http://" + server.address().address + ":" + server.address().port + "/";
        console.log("http server listening on " + localUrl);

        var promises = configUpdaterTests.updateFileTests.map(function (test, index) {
            var suffix = " - test.id: " + test.id;

            /** @type {AutoUpdateFile} */
            var file = {
                path: configUpdaterTests.getTempFile("updateFile"),
                url: test.input.hasOwnProperty("url") ? test.input.url : localUrl + index,
                isJSON: test.input.isJSON,
                always: test.input.always
            };

            if (test.input.urlQuery) {
                if (Array.isArray(test.input.urlQuery)) {
                    var urls = test.input.urlQuery.map(function (query) {
                        return file.url + "?" + query;
                    });
                    file.url = urls;
                } else {
                    file.url += "?" + test.input.urlQuery;
                }
            }

            var lastUpdate = Object.assign({}, test.input.lastUpdate);

            if (test.input.content) {
                // Write the "old" content.
                fs.writeFileSync(file.path, test.input.content);
            }

            return configUpdater.updateFile(file, lastUpdate).then(function (result) {
                // If the previous file is expected, set the field to the filename, otherwise remove it.
                if (test.expect.lastUpdate.previous) {
                    test.expect.lastUpdate.previous = file.path + ".previous";
                } else {
                    delete test.expect.lastUpdate.previous;
                }

                jqUnit.assertDeepEq("resolved value from updateFile should match the expected" + suffix,
                    test.expect.lastUpdate, result);

                // Check the file has been updated as expected.
                configUpdaterTests.assertFile("Target file should contain the expected content",
                    test.expect.content, file.path);

                if (!test.expect.request) {
                    // For tests with no requests, the request assertion isn't hit.
                    jqUnit.assert("no request");
                }

            }, jqUnit.fail);

        });
        Promise.all(promises)["catch"](jqUnit.fail).then(function () {
            jqUnit.start();
        });
    });

    teardowns.push(function () {
        server.close();
        server.unref();
    });

});


jqUnit.asyncTest("Test updateAll", function () {

    var test = configUpdaterTests.updateAllTest;
    jqUnit.expect(2);

    var service = require("../src/service.js");
    service.config.autoUpdate = {
        enabled: true,
        files: test.input.autoUpdateFiles,
        lastUpdatesFile: configUpdaterTests.getTempFile("lastUpdatesFile"),
        retryDelay: 1
    };

    var files = [];

    var updateFileOrig = configUpdater.updateFile;
    teardowns.push(function () {
        configUpdater.updateFile = updateFileOrig;
    });

    var retryCount = {};

    // Check that updateFile gets called for every updated file, with the correct data.
    configUpdater.updateFile = function (update, lastUpdate) {
        files.push(update.path);
        jqUnit.expect(1);
        jqUnit.assertEquals("update argument must correspond with the correct lastUpdate", update.url, lastUpdate.id);

        var error = update.error;

        if (update.retry) {
            var retries = retryCount[update.path] || 0;
            retryCount[update.path] = ++retries;
            error = retries < update.retry;
        }
        return (error)
            ? Promise.reject({isError: true})
            : Promise.resolve(Object.assign({}, lastUpdate, {handled:true}));
    };

    // Save the data, to ensure the file gets loaded in updateAll
    configUpdater.saveLastUpdates(test.input.lastUpdates).then(function () {
        return configUpdater.updateAll(5).then(function () {
            // Check every file was processed.
            jqUnit.assertDeepEq("updateFile should be called for every file",
                test.expect.updateAttempts.sort(), files.sort());

            // Check that the last-updates file had been written to.
            return configUpdater.loadLastUpdates().then(function (lastUpdates) {
                // Remove the lastCheck field
                delete lastUpdates.lastCheck;
                jqUnit.assertDeepEq("lastUpdates should the expected value", test.expect.lastUpdates, lastUpdates);
            });
        });
    })["catch"](jqUnit.fail).then(jqUnit.start);
});
