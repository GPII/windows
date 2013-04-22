SET NODE_ENV=deployment.statistical.mm

start node.exe gpii.js %1
start UsbUserListener\bin\UsbUserListener.exe

SET NODE_ENV=
