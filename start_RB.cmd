SET NODE_ENV=rule.based.mm.production

start node.exe gpii.js %1
start UsbUserListener\bin\UsbUserListener.exe

SET NODE_ENV=
