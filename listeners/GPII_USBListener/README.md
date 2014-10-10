#GPII USB User Listener for Windows

This folder contains the necessary files for the Windows USB listener (source code and libraries).

## Build

Build using Visual studio. The 2013 Express version is know to work. You can build using the IDE or command line.  Build the solution to get the installer.

### libcurl

The static version of libcurl is used to create the http GET actions and while files for libcurl 7v37 are included in the source you may wish to regenerate them, for example to update libcurl. To generate these files do the following
