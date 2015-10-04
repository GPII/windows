# GPII for Windows

Contains platform-specific components of the GPII architecture for Windows. See http://gpii.net/ for overall details of the GPII
project. After checkout out using git, this project will require node.js and npm to be installed - please consult
http://wiki.gpii.net/w/Setting_Up_Your_Development_Environment for installation instructions.

Note that tests and function involving the High Contrast setting will fail on all current versions of Windows (including 
Windows 7 SP1 and Windows 8 SP2) unless the following hotfix from Microsoft is applied: http://support.microsoft.com/kb/2516889

See http://issues.gpii.net/browse/GPII-49 for more details of this issue. 

# Grunt Builds

We are in the process of adding support for building and running utility tasks 
with grunt.js. At some point these will replace the current build scripts.

To use these, you must be running a recent version of npm, necessarily greater 
than 1.4.  If you need to upgrade npm you can issue the following command:

    npm install -g npm

To build the GPII for Windows using grunt, perform the following:

    mkdir gpii
    cd gpii
    git clone https://github.com/GPII/windows.git
    cd windows
    npm install --ignore-scripts=true
    grunt build

Note that whenever you run the `npm install` task for this project, you must use the option appearing above.

# Old Builds

This project is still bundled with command-line build scripts (with the same effect as the above grunt builds) - support for these
will be withdrawn soon. You can operate these as follows:


* `build.cmd` will check out GPII's universal project https://github.com/GPII/universal in a subdirectory. 
* `start.cmd` will run the GPII personalisation system


