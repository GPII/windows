choco install nodejs.install -version 4.4.3 -y
choco install python2 -y
setx /M PATH "%PATH%;C:\Program Files\nodejs;C:\tools\python2"
call npm config set msvs_version 2015 --global
