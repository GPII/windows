/*
GPII Acceptance Testing for Windows

Copyright 2013 Raising the Floor International
Copyright 2014 Lucendo Development Ltd.

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

The research leading to these results has received funding from the European Union's
Seventh Framework Programme (FP7/2007-2013) under grant agreement no. 289016.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

"use strict";

var fluid = require("universal"),
    gpii = fluid.registerNamespace("gpii");

gpii.loadTestingSupport();

fluid.registerNamespace("gpii.acceptanceTesting.windows");

require("../index.js");

var baseDir = fluid.module.resolvePath("%universal/tests/");
var windowsFiles = fluid.require("%universal/tests/platform/index-windows.js");

gpii.test.runSuitesWithFiltering(windowsFiles, baseDir, ["gpii.test.acceptance.testCaseHolder"]);
