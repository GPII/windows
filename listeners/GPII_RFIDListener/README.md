# GPII RFIDListener

Windows executable that listens out for NFC RFID tags and invokes URLs to trigger GPII user log on/off. It listens out for RFID tag events and reads the encoded tag data. The listener does not track the login state and only informs GPII that a tag has been presented, via the GPII RESTful API (e.g http://localhost:8081/user/bert/proximityTriggered).

See the [User listener](http://wiki.gpii.net/index.php/User_Listener) and [NFC](http://wiki.gpii.net/index.php/Using_the_NFC_Listener) pages on the GPII wikifor details of USB and NFC listening and how to encode user IDs.

Known to work with Advanced Card Systems RFID readers including ACR122 with Mifare Classic 1K and NTAG203 tags.

## Use

1. Plug in the USB NFC card reader if required
2. Run the GPII_RFIDListener.exe. A tray icon is created and can be right clicked on to show a menu.
3. Optionally use the menu item "Show Status Window" to view status. 
4. Optionally use the "Show Diagnostic Window" for debugging. The X to close window actually clears the text as edit controll will fill up.
5. Place a tag with the user UID encoded as UTF8 onto the reader. Once the new status has been recognised the tag can be removed.

## Build

Build using Visual studio. The 2015 version is known to work. You can build using the IDE or command line. Build the solution to get the installer.

### libcurl

The static version of libcurl is used to create the http GET actions and while files for libcurl v7.49.1 are included in the source you may wish to regenerate them, for example to update libcurl. To generate these files do the following

See the README in libcur/ for details of generating these files

## Testing 

Apart from normal debugging the Diagnostic window provides plenty of details, including a dump of the TAG memory
