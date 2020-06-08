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
    fs = require("fs");

var teardowns = [];

jqUnit.module("GPII service tests", {
    teardown: function () {
        while (teardowns.length) {
            teardowns.pop()();
        }
    }
});

jqUnit.test("Test config loader", function () {
    var service = require("../src/service.js");
    // service.js should already have called service.config.
    jqUnit.assertNotNull("service.config is called on startup", service.config);

    // Check the package.json was read from
    var packageJson = require("./test-package.json");
    jqUnit.assertEquals("Version from package.json should be in the config",
        packageJson.version, service.config.morphicVersion);

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


    // Check a config file will be loaded if the process is running as a service
    try {
        service.isService = true;
        var config = service.loadConfig(testDir);
        jqUnit.assertTrue("config should be loaded when running as a service",
            config && config.testLoaded);
    } finally {
        // Ensure the next user of service.js gets a clean one
        delete require.cache[require.resolve("../src/service.js")];
    }
});

jqUnit.test("Test secret loading", function () {
    try {
        var service = require("../src/service.js");
        var secret = service.getSecrets();
        jqUnit.assertTrue("Secret should have been loaded", !!secret);

        // Try a secret file that does not exist
        service.config.secretFile = "does/not/exist";
        var secret2 = service.getSecrets();
        jqUnit.assertNull("No secret should have been loaded", secret2);
    } finally {
        // Ensure the next user of service.js gets a clean one
        delete require.cache[require.resolve("../src/service.js")];
    }
});
