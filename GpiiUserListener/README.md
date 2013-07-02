# GPII User Listener

Windows executable that listens out for NFC RFID tags and raises events to trigger GPII user log on/off. It listens out for RFID tag events and reads the encoded tag data. The listener tracks the login state and generate loginin and logout events on the GPII RESTful API (e.g http://localhost:8081/user/bert/login).

See the [NFC page on the GPII wiki](http://wiki.gpii.net/index.php/Using_the_NFC_Listener) for details of NFC listenrs and how to encode RFID suitable tags.

Known to work with Advanced Card Systems RFID readers including ACR122.

## Use

1. Plug in the USB card reader
2. Run the GpiiUserListener.exe. A tray icon is created and can be right clicked on to show a menu. Use the menu item "Show Window" to view status.
3. Place a tag with the user id encoded onto the reader. 

## Build

Requires Visual Studio >= 2012 should work with the free Express version. 
Open GpiiUserListener.sln and build the Debug and/or Release versions.

### libcurl

The static version of libcurl is used to create the http GET actions and while files for libcurl 7v25 are included in the source you may wish to regenerate them, for example to update libcurl. To generate these files do the following

1. Download curl source from http://curl.haxx.se/libcurl/
2. Open a MSVC command shell and cd to the libcurl/winbuild folder
3. Build both static debug and release libraries using a command line of the form:
     nmake /f Makefile.vc mode=static VC=11 ENABLE_IPV6=yes USE_SSPI=no ENABLE_IDN=no DEBUG=yes
	(IPv6 is not required but the build fails with IPV6=no in 7.25
4. refresh the .h & .lib files in this project's libcurl folder

## Testing 

Apart from normal debugging the [Fiddler HTTP proxy](http://fiddler2.com/) is useful to monitor the URLs being created. Note by default Fiddler won't show Http://localhost URLs so specific code to make it work has been added to the listener in a debug build.
