# GPII RFIDListener

Windows executable that listens out for NFC RFID tags and invokes URLs to trigger GPII user log on/off. It listens out for RFID tag events and reads the encoded tag data. The listener tracks the login state and generate login and logout events on the GPII RESTful API (e.g http://localhost:8081/user/bert/login).

See the [User listener](http://wiki.gpii.net/index.php/User_Listener) and [NFC](http://wiki.gpii.net/index.php/Using_the_NFC_Listener) pages on the GPII wikifor details of USB and NFC listening and how to encode user IDs.

Known to work with Advanced Card Systems RFID readers including ACR122.

## Use

1. Plug in the USB NFC card reader if required
2. Run the GPII_RFIDListener.exe. A tray icon is created and can be right clicked on to show a menu. Use the menu item "Show Window" to view status.
3. Place a tag with the user id encoded onto the reader. Once the ne status has been recognised the tag can be removed.

## Build

Build using build.sh in Mingw base installation with g++ installed. Set COPTS in build.sh to change between release and debug builds. Apart form debugging in gdb DEBUG also
* Displays the monitor window on startup
* Closing the monitor window exits the listener
* Flow manger URLs are logged to debug output and are shown in gdb

### libcurl

The static version of libcurl is used to create the http GET actions and while files for libcurl 7v25 are included in the source you may wish to regenerate them, for example to update libcurl. To generate these files do the following

See the README in libcur/ for details of generating these files

## Testing 

Apart from normal debugging the [Fiddler HTTP proxy](http://fiddler2.com/) is useful to monitor the URLs being created. Note by default Fiddler won't show Http://localhost URLs so specific code to make it work has been added to the listener in a debug build.
