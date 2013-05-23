SET NODE_ENV=statistical.mm.production

start node.exe gpii.js %1
start UsbUserListener\bin\UsbUserListener.exe

SET NODE_ENV=
