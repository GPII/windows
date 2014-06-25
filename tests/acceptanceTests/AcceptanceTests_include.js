/*
 *
 * GPII Acceptance Testing
 *
 * Copyright 2013 Raising the Floor International
 *
 * Licensed under the New BSD license. You may not use this file except in
 * compliance with this License.
 *
 * The research leading to these results has received funding from the European Union's
 * Seventh Framework Programme (FP7/2007-2013)
 * under grant agreement no. 289016.
 *
 * You may obtain a copy of the License at
 * https://github.com/GPII/universal/blob/master/LICENSE.txt
 */

/*global __dirname, require*/

"use strict";
var fluid = require("universal"),
    path = require("path"),
    kettle = fluid.registerNamespace("kettle"),
    gpii = fluid.registerNamespace("gpii");

fluid.registerNamespace("gpii.acceptanceTesting.windows");

fluid.require("../../gpii/node_modules/registrySettingsHandler", require);
fluid.require("../../gpii/node_modules/registryResolver", require);
fluid.require("../../gpii/node_modules/spiSettingsHandler", require);

fluid.require("universal/tests/AcceptanceTests", require);


gpii.acceptanceTesting.windows.runTests = function (configFile, testDefs) {
    var gpiiConfig = {
       nodeEnv: configFile,
       configPath: path.resolve(__dirname, "./configs")
    };
    fluid.each(testDefs, function (testDef) {
        testDef.config = gpiiConfig;
    });
    testDefs = gpii.acceptanceTesting.buildTests(testDefs);
    module.exports = kettle.tests.bootstrap(testDefs);
}
