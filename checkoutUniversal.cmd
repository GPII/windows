if not exist checkoutUniversal.cmd (
    echo This command should be run from the current directory.
    echo It checks out the GPII Universal project to a node_modules directory 
    echo where both the Windows and Universal projects may mutually reference each other.
    echo A prerequisite is that this directory itself (windows) must be checked out as a subdirectory
    echo of a directory named node_modules.
    exit /b
)

if not exist ..\..\node_modules (
    echo The GPII windows project must be checked out into a directory named node_modules in order to be resolvable from the Universal project.
    echo That is, there should be a directory structure of the form xxxx\node_modules\windows <-- this directory
    exit /b
)

if not exist ..\..\node_modules\universal (
    cd ..\..\node_modules
    git clone git://github.com/GPII/universal.git
    cd windows
)