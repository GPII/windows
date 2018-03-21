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
    grunt.loadNpmTasks("grunt-jsonlint");
    grunt.loadNpmTasks("grunt-shell");
    grunt.loadNpmTasks("fluid-grunt-eslint");

    var gypCompileCmd = "node-gyp configure build";
    var gypCleanCmd = "node-gyp clean";

    function nodeGypShell(cmd, cwd) {
        return {
            options: {
                execOptions: {
                    cwd: cwd
                }
            },
            command: cmd
        };
    }

    grunt.registerTask("lint", "Apply jshint and jsonlint", ["eslint", "jsonlint"]);

    grunt.initConfig({
        eslint: {
            src: ["./gpii/**/*.js", "./tests/**/*.js", "./*.js"]
        },
        jsonlint: {
            src: ["gpii/**/*.json", "tests/**/*.json", "examples/**/*.json", "./*.json"]
        },
        shell: {
            options: {
                stdout: true,
                stderr: true,
                failOnError: true,
                // A large maxBuffer value is required for the 'runAcceptanceTests' task otherwise
                // a 'stdout maxBuffer exceeded' warning is generated.
                execOptions: {
                    maxBuffer: 1000 * 1024
                }
            },
            compileTabletMode: nodeGypShell(gypCompileCmd, "gpii/node_modules/tabletMode/src"),
            cleanTabletMode: nodeGypShell(gypCleanCmd, "gpii/node_modules/tabletMode/src")
        }
    });

    grunt.registerTask("build-addons", "Build the native addons", function () {
        grunt.task.run("shell:compileTabletMode");
    });

    grunt.registerTask("build", "Build the entire GPII", function () {
        grunt.task.run("build-addons");
    });

    grunt.registerTask("clean-addons", "Clean the native addons", function () {
        grunt.task.run("shell:cleanTabletMode");
    });

    grunt.registerTask("clean", "Clean the GPII binaries and uninstall", function () {
        grunt.task.run("clean-addons");
    });

};
