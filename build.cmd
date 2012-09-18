cd UsbUserListener
mkdir bin
copy lib\libcurl.dll bin
start build.cmd
cd ..

if not exist ..\node_modules (
    mkdir ..\node_modules
)

if not exist ..\node_modules\universal (
    cd ..\node_modules
    git clone git://github.com/GPII/universal.git
    cd ..\windows
)
