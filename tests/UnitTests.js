/*

GPII Unit Testing for Windows

Copyright 2014 Lucendo Development Ltd.
Copyright 2016 Raising the Floor - US

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

"use strict";

require("../index.js");

require("../gpii/node_modules/displaySettingsHandler/test/testDisplaySettingsHandler.js");
// Commenting out App Zoom tests since these fail fairly often in CI and vagrant in electron - GPII-3802
// require("../gpii/node_modules/gpii-app-zoom/test/testAppZoom.js");
require("../gpii/node_modules/gpii-localisation/test/testLanguage.js");
require("../gpii/node_modules/nativeSettingsHandler/test/testNativeSettingsHandler.js");
require("../gpii/node_modules/processHandling/test/testProcessHandling");
require("../gpii/node_modules/processReporter/test/all-tests.js");
require("../gpii/node_modules/registeredAT/test/testRegisteredAT.js");
require("../gpii/node_modules/registrySettingsHandler/test/testRegistrySettingsHandler.js");
require("../gpii/node_modules/registryResolver/test/testRegistryResolver.js");
require("../gpii/node_modules/spiSettingsHandler/test/testSpiSettingsHandler.js");
require("../gpii/node_modules/systemSettingsHandler/test/testSystemSettingsHandler.js");
require("../gpii/node_modules/userListeners/test/all-tests.js");
require("../gpii/node_modules/windowMessages/test/windowMessagesTest.js");
require("../gpii/node_modules/windowsMetrics/test/WindowsMetricsTests.js");
require("../gpii/node_modules/windowsUtilities/test/WindowsUtilitiesTests.js");
require("../gpii/node_modules/wmiSettingsHandler/test/testWmiSettingsHandler.js");
require("../gpii-service/tests/all-tests.js");
