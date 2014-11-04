/*!
GPII Windows Personalization Framework Gruntfile

Copyright 2014 RTF-US

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

"use strict";

module.exports = function (grunt) {
    grunt.loadNpmTasks("grunt-contrib-jshint");
    grunt.loadNpmTasks("grunt-jsonlint");
    grunt.loadNpmTasks("grunt-shell");
    grunt.loadNpmTasks("grunt-gpii");
        
    var gpiiGrunt = grunt.config.getRaw("gpiiGruntGlobal");
    grunt.initConfig({
        jshint: {
            src: ["gpii/**/*.js", "tests/**/*.js", "index.js", "gpii.js"],
            buildScripts: ["Gruntfile.js"],
            options: {
                jshintrc: true
            }
        },
        jsonlint: {
            src: ["gpii/**/*.json", "tests/**/*.json"]
        }
    });

    grunt.registerTask("build", "Build the entire GPII", function () {
        grunt.task.run("gpii-universal");
    });

    grunt.registerTask("start", "Start the GPII", function () {
        gpiiGrunt.shellImmediate({
            name: "start",
            command: "node gpii.js"
        });
    });
};
