GPII USB User Listener for Windows

This folder contains the necessary files for the Windows USB listener (source code and libraries).

## Build

Build using Visual studio. The 2013 Express version is know to work. You can build using the IDE or command line.  Build the solution to get the installer.

### libcurl

The static version of libcurl is used to create the http GET actions and while files for libcurl 7v37 are included in the source you may wish to regenerate them, for example to update libcurl. To generate these files do the following

### Alternative build (not used)

See the README in libcur/ for details of generating these files
Dependencies:
1. MinGW - download and install from http://www.mingw.org
2. libcurl - download from http://curl.haxx.se/download.html and follow build/installation instructions for Windows MinGW (http://curl.haxx.se/docs/install.html)

Installation instructions:
1. Make sure you have MinGW installed.
2. Copy lib/libcurl.dll from the curl build to a folder on your system's PATH or this root directory
3. Build and link with MinGW g++ with the build.cmd file or else use the following options:
g++ -I.\include -O0 -g3 -Wall -c -fmessage-length=0 -o src\UsbUserListener.o src\UsbUserListener.cpp
g++ -L.\lib -static-libgcc -o UsbUserListener.exe src\UsbUserListener.o -lcurldll
You need to pay specific attention to the -I, -L, -static-libgcc and -lcurldll flags.
4. When the application is run, you should be able to see UsbUserListener.exe in the task manager. Fire up the Flow Manager to see requests coming upon insertion or removal of USB sticks.

Build flags:
-I, -L and -l specify the include path for curl/curl.h and the libcurldll.a library
-static-libgcc is needed for the application to run. If omitted, the application terminates immediately (http://www.eclipse.org/forums/index.php/mv/tree/156519/#page_top)


In order to run the application, the dll libcurl.dll must be resolvable on the PATH (preferably in the same directory as the .exe) otherwise
the .exe will terminate on startup with error 0xc0000013