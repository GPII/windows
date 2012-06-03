rem This script should be run from its own directory
rem It requires MinGW to be installed - see instructions in USBUserListener readme

cd UsbUserListener
mkdir bin
copy lib\libcurl.dll bin
start build.cmd
cd ..
