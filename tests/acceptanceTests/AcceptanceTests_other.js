/*

GPII Acceptance Testing

Copyright 2014 Raising the Floor - International

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

/*global require,process*/

"use strict";
var fluid = require("universal"),
    path = require("path"),
    gpii = fluid.registerNamespace("gpii");

fluid.require("./AcceptanceTests_include", require);

var testDefs = [
    {
        name: "Testing that the system doesn't crashes on login with empty NP set",
        token: "empty",
        settingsHandlers: {},
        processes: []
    }
];

gpii.acceptanceTesting.windows.runTests("builtIn_config", testDefs);