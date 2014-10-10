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

require("./gpii/index.js");

kettle.config.makeConfigLoader({
    nodeEnv: kettle.config.getNodeEnv("fm.ps.sr.dr.mm.os.lms.development"),
// TODO: it would be nice to not have to hardcode this path.
    configPath: kettle.config.getConfigPath() || "../node_modules/universal/gpii/configs"
});
