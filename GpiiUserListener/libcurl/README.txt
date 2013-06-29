This folder contains files from the libcurl project

To generate these files do the following

1: download curl source eg 7.25 from http://curl.haxx.se/libcurl/
2: cd to the winbuild folder
3: build both static debug and release libraries using following command line
     nmake /f Makefile.vc mode=static VC=11 ENABLE_IPV6=yes USE_SSPI=no ENABLE_IDN=no DEBUG=yes
	note the build fails with IPV6=no
4: refresh the .h & .lib files in this project's libcurl folder
