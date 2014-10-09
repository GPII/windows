The installer is created using the unicode vesion of Inno Setup and it's preprocessor'
http://www.jrsoftware.org/isinfo.php

Be sure to leave the preprocessor option checked and you will need to add the directory containing iscc.exe to the path before you can build.

The installer is created in ${SolutionDir}bin\Release\
It's version is included in it's filename and is set to be the greater of the version numbers of the 2 listeners.
The version number of one or both Listeners must be increased as part of the release process 