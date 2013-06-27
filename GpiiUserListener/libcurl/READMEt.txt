This folder contains files from the libcurl project

To generate these files do the following

1: download curl source eg 7.25 from http://curl.haxx.se/libcurl/
2: open <curl>\lib\vc6libcurl.dsw in Visual studio
3: build both DLL debug and DLL release
4: refresh the .h, .lib & .dll files in this project's libcurl folder
5: be sure to distribute the.dll with the user listener built exe