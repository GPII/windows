/*!
GPII Windows Personalization Framework Gruntfile

Copyright 2014 RFT-US

Licensed under the New BSD license. You may not use this file except in
compliance with this License.

You may obtain a copy of the License at
https://github.com/gpii/universal/LICENSE.txt
*/

module.exports = function(grunt) {
    grunt.initConfig({
        pkg: grunt.file.readJSON("package.json"),
        shell: {
            startGPII: {
                options: {
                    stdout: true,
                    stderr: true
                },
                command: function() {
                    return "node.exe gpii.js";
                }
            }
        }
    });

    grunt.loadNpmTasks("grunt-gpii");

    grunt.registerTask("build", "Build the entire GPII", function() {
        grunt.task.run("gpiiUniversal"); 
    });

    grunt.registerTask("start", "Start the GPII", function() {
        grunt.task.run("shell:startGPII");
    });
};
