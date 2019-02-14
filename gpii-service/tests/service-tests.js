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

    var origConfig = service.config;
    // Check a config file will be loaded if the process is running as a service
    try {
        service.config = null;
        service.isService = true;
        service.loadConfig(process.cwd());
        jqUnit.assertNotNull("config should be loaded when running as a service", service.config);
    } finally {
        service.isService = false;
        service.config = origConfig;
    }
});
