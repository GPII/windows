/* Tests for service.js
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
    os = require("os"),
    path = require("path"),
    fs = require("fs"),
    service = require("../src/service.js");

var teardowns = [];

jqUnit.module("GPII service tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

jqUnit.test("Test config loader", function () {
    // service.js should already have called service.config.
    jqUnit.assertNotNull("service.config is called on startup", service.config);

    // Create a temporary config file.
    var testDir = path.join(os.tmpdir(), "gpii-service-test" + Math.random());
    var testFile = path.join(testDir, "service.json5");
    teardowns.push(function () {
        try {
            fs.unlinkSync(testFile);
            fs.rmdirSync(testDir);
        } catch (e) {
            // ignore
        }
    });

    fs.mkdirSync(testDir);
    fs.writeFileSync(testFile, "{testLoaded: true}");


    var origConfig = service.config;
    // Check a config file will be loaded if the process is running as a service
    try {
        service.config = null;
        service.isService = true;
        service.loadConfig(testDir);
        jqUnit.assertTrue("config should be loaded when running as a service",
            service.config && service.config.testLoaded);
    } finally {
        service.isService = false;
        service.config = origConfig;
    }
});

jqUnit.test("Test secret loading", function () {
    var secret = service.getSecrets();
    jqUnit.assertTrue("Secret should have been loaded", !!secret);

    var origFile = service.config.secretFile;
    try {
        // Try a secret file that does not exist
        service.config.secretFile = "does/not/exist";
        var secret2 = service.getSecrets();
        jqUnit.assertNull("No secret should have been loaded", secret2);
    } finally {
        service.config.secretFile = origFile;
    }
});
