/*
GPII Windows Personalization Framework Gruntfile
Copyright 2013-2014 OCAD University

Licensed under the Educational Community License (ECL), Version 2.0 or the New
BSD license. You may not use this file except in compliance with one these
Licenses.

You may obtain a copy of the ECL 2.0 License and BSD License at
https://github.com/fluid-project/infusion/raw/master/Infusion-LICENSE.txt
*/

"use strict";

module.exports = function (grunt) {
    grunt.initConfig({
        lintAll: {
            sources: {
                md:    [ "./*.md","./gpii/*.md", "./settingsHelper/**/*.md"],
                js:    ["./gpii/**/*.js", "./tests/**/*.js", "./*.js", "./service/src/**/*.js", "./service/tests/**/*.js"],
                json:  ["./gpii/**/*.json", "./tests/**/*.json", "./settingsHelper/**/*.json", "./*.json"],
                json5: ["./gpii/**/*.json5", "./tests/**/*.json5", "./*.json5"],
                other: ["./.*"]
            }
        }
    });

    grunt.loadNpmTasks("gpii-grunt-lint-all");
    grunt.registerTask("lint", "Perform all standard lint checks.", ["lint-all"]);
};
