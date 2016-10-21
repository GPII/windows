pushd .

if not exist C:\installer\\. (
    git clone https://github.com/gpii/gpii-wix-installer C:\installer

    cd C:\installer
    git checkout tags/v1.1.0
)

rem Use local copy of node
copy "%ProgramFiles(x86)%\nodejs\node.exe" C:\installer\staging /y

rmdir /s /q C:\installer\staging\windows

cd C:\vagrant\provisioning
robocopy /job:windows.rcj

cd C:\installer\staging\windows
call npm prune --production

cd C:\installer\setup

pushd ..\
    if not exist output\\. mkdir output
    if not exist temp\\. mkdir temp
popd

call "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
msbuild setup.msbuild

popd
