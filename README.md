# GPII for Windows

Contains platform-specific components of the GPII architecture for Windows. See [http://gpii.net/](http://gpii.net/) for
overall details of the GPII project. After checkout out using git, this project will require node.js and npm to be
installed - please consult [Setting Up Your Development Environment](http://wiki.gpii.net/w/Setting_Up_Your_Development_Environment)
for installation instructions.

## Building

This package depends on node 8.x (LTS).

To build the GPII for Windows, perform the following:

    git clone https://github.com/GPII/windows.git
    cd windows
    npm install

## Running the Tests

The preferred way to run the tests is to do so in a standard virtual machine.  Before you can set up the VM, you will
need to ensure you have met [these VM requirements](https://github.com/GPII/qi-development-environments/#requirements).
You can then use the ``vagrant up`` command to create an instance of a [Windows 10 Evaluation VM](https://github.com/idi-ops/packer-windows),
which will boot an instance of the Windows 10 VM, pull in the GPII Framework's npm dependencies, and then build it.

If this is your first time creating this VM an 8 GB download will take place. The downloaded image will be valid for 90
days after which the Windows installation will no longer be useable. To remove an expired image you can use the
``vagrant box remove "inclusivedesign/windows10-eval"`` command.

Now you can open a command prompt window and use the following commands to test the framework:

    refreshenv
    cd c:\vagrant
    npm test

*Note:* The [refreshenv](https://github.com/chocolatey/chocolatey/blob/a09e15896fbc5e790b17b6699cd6b50bc7eb14e4/src/redirects/RefreshEnv.cmd)
command only needs to be used if you are about to issue commands after having just created the VM. If the VM has been
restarted since its creation, the command is not required.

At the end of a test run in either environment, an [Istanbul](https://github.com/gotwarlost/istanbul) coverage report
will be saved in the `coverage` directory.

Note that tests and function involving the High Contrast setting will fail on all current versions of Windows (including
Windows 7 SP1 and Windows 8 SP2) unless the following hotfix from Microsoft is applied:
[http://support.microsoft.com/kb/2516889](http://support.microsoft.com/kb/2516889)

See [GPII-49](http://issues.gpii.net/browse/GPII-49) for more details of this issue.
