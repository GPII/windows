choco install nodejs.install -version 6.3.1 --forcex86 -y
call refreshenv
cd "C:\Program Files (x86)\nodejs\node_modules\npm"
call npm install node-gyp@3.4.0
choco install python2 -y
choco install innosetup -y
setx /M PATH "%PATH%;C:\Program Files (x86)\nodejs;C:\tools\python2;C:\Program Files (x86)\Inno Setup 5"
