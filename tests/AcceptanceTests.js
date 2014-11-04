/*

GPII Acceptance Testing for Windows

Copyright 2013 Raising the Floor International
Copyright 2014 Lucendo Development Ltd.

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

"use strict";

var fluid = require("universal"),
    gpii = fluid.registerNamespace("gpii");

gpii.loadTestingSupport();

fluid.registerNamespace("gpii.acceptanceTesting.windows");

require("../index.js");

var baseDir = fluid.module.resolvePath("${universal}/tests/");
var windowsFiles = fluid.require("${universal}/tests/platform/index-windows.js");

gpii.test.runSuitesWithFiltering(windowsFiles, baseDir, ["gpii.test.acceptance.testCaseHolder"]);
