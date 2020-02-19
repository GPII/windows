/* Tests for the GPII Windows Service.
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

// The service tests are ran in a separate process to the rest of GPII. This not only ensures it's isolated from GPII,
// but also prevents having to re-build to be ran under electron for the gpii-app tests.

if (!global.fluid) {
    // In child process.
    require("./service-tests.js");
    require("./windows-tests.js");
    require("./gpii-ipc-tests.js");
    require("./processHandling-tests.js");
    require("./pipe-messaging-tests.js");
    require("./gpii-client-tests.js");
    return;
}

var jqUnit = require("node-jqunit"),
    child_process = require("child_process");

jqUnit.module("GPII service tests");

jqUnit.asyncTest("Test window service", function () {
    console.log("Starting service tests");

    var child = child_process.spawn("node", [__filename]);
    child.stdout.pipe(process.stdout);
    child.stderr.pipe(process.stderr);

    child.on("close", function (code) {
        console.log("Service tests ended:", code);
        jqUnit.assertEquals("Service tests should pass", 0, code);
        jqUnit.start();
    });
});
