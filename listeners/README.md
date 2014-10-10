# Listeners

The GPII listeners for windows are currently USB and RFID (NFC). See individual README.md files for details
The executables can be found in ./bin/{Debug,Release} though the installer is only in Release

# Building

## In VisualStudio

Open listener.sln in VS, sleect Debug or Release and right click on Solution in the explorer and build.
Has been tested in VS2013 (12) express

## On command line with MSBuild

Open a command window and run

"%ProgramFiles(x86)%\MSBuild\12.0\bin\msbuild" listeners.sln /property:Configuration=Release

## Making a release

* Make sure the listeners version numbers in GPII_{USB,RFID}UserListener.rc are correct
* Build the Release solution (See above)
* Note the version number for the installer .exe will be the greater of the 2 listeners versions
* Checkin the code to GitHub (or make a pull request)
* Make a binary GitHub release of the installer using a Tag of the form "Listeners_V1.3.0" (using the installer version)
* Include the MD5 hash in the release text. Both Cygwin and the Msys shell of GitHub for Windows include md5sum