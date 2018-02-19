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
        eslint: {
            src: ["./gpii/**/*.js", "./tests/**/*.js", "./*.js"]
        },
        jsonlint: {
            src: ["gpii/**/*.json", "tests/**/*.json", "examples/**/*.json", "./*.json"]
        }
    });

    grunt.loadNpmTasks("grunt-jsonlint");
    grunt.loadNpmTasks("fluid-grunt-eslint");

    grunt.registerTask("lint", "Apply jshint and jsonlint", ["eslint", "jsonlint"]);
};
