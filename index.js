/*!
GPII Universal Personalization Framework GPII Windows Index

Copyright 2014 Lucendo Development Ltd.

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

The research leading to these results has received funding from the European Union's
Seventh Framework Programme (FP7/2007-2013) under grant agreement no. 289016.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

"use strict";

var fluid = require("gpii-universal"),
    path = require("path");

fluid.module.register("gpii-windows", __dirname, require);

fluid.contextAware.makeChecks({
    "gpii.contexts.windows": {
        value: true
    }
});

// Add ./bin to the path.
var binPath = path.join(__dirname, "bin");
process.env.path = binPath + ";" + process.env.path;

require("./gpii/node_modules/WindowsUtilities/WindowsUtilities.js");
require("./gpii/node_modules/processHandling/processHandling.js");
require("./gpii/node_modules/displaySettingsHandler");
require("./gpii/node_modules/registrySettingsHandler");
require("./gpii/node_modules/registryResolver");
require("./gpii/node_modules/spiSettingsHandler");
require("./gpii/node_modules/registeredAT/registeredAT.js");
require("./gpii/node_modules/windowsMetrics");
require("./gpii/node_modules/processReporter");
require("./gpii/node_modules/windowMessages");
require("./gpii/node_modules/userListeners");
require("./gpii/node_modules/systemSettingsHandler");
require("./gpii/node_modules/nativeSettingsHandler");
require("./gpii/node_modules/gpii-app-zoom");
require("./gpii/node_modules/wmiSettingsHandler");
require("./gpii/node_modules/gpii-localisation");
require("./gpii/node_modules/gpii-service-handler");

module.exports = fluid;
