/*!
GPII Windows Personalization Framework Node.js Bootstrap

Copyright 2012 OCAD University

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

var fluid = require("universal"),
    kettle = fluid.registerNamespace("kettle");

// TODO: it would be nice to not have to hardcode this path. A different module layout
// for the platform packages, perhaps?
fluid.require("./gpii/node_modules/registrySettingsHandler", require);
fluid.require("./gpii/node_modules/registryResolver", require);
fluid.require("./gpii/node_modules/spiSettingsHandler", require);

kettle.config.makeConfigLoader({
    nodeEnv: kettle.config.getNodeEnv("fm.ps.sr.dr.mm.os.lms.development"),
    configPath: kettle.config.getConfigPath() || "../node_modules/universal/gpii/configs"
});
