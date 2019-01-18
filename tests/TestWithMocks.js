/*
 * Application Zoom
 *
 *
 * Copyright 2019 Raising the Floor - International
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

"use strict";

var fluid = require("gpii-universal"),
    path = require("path");

var gpii = fluid.registerNamespace("gpii");
var windows = fluid.registerNamespace("gpii.windows");
fluid.registerNamespace("gpii.windows.tests");
var _ = require("lodash");

windows.tests.testWithMocks = function (mockedMethods, test, fail) {
    var restore = [];
    for(var i=0; i < mockedMethods.length; i++) {
        restore[i] = _.get(mockedMethods[i].original.obj, mockedMethods[i].original.path);
        _.set(mockedMethods[i].original.obj, mockedMethods[i].original.path, mockedMethods[i].mock);
    }
    try {
        test();
    } catch(e) {
        fail(e);
    } finally {
        for(var i=0; i < mockedMethods.length; i++) {
            _.set(mockedMethods[i].original.obj, mockedMethods[i].original.path, restore[i]);
        }
    }
}
