/* Entry point for when the GPII windows service starts.
 * This should be executed when the service has already started, otherwise a delay in the loading may cause the system
 * to think it's not responding to the start signal.
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
var service = require("./service.js");

require("./windows.js");
require("./gpii-ipc.js");
require("./processHandling.js");
require("./gpiiClient.js");

service.start();
