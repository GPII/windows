GPII node.js and executable components for Windows
==================================================

This project is part of the GPII personalisation infrastructure which are specific to Microsoft's "Windows"™ 
operating system, in the desktop configuration. In particular, these components are portable to Win32 at 
and above the level supported by Windows XP.

A description of the GPII project and its aims can be found at [http://www.gpii.net/](http://www.gpii.net/). 
Technical documentation is housed on the wiki at [http://wiki.gpii.net](http://wiki.gpii.net).

Checkout Structure
------------------

In order to make use of the system, this project must be used in conjunction with the GPII "universal" components 
held at [https://github.com/GPII/universal](https://github.com/GPII/universal). 
We recommend the following directory layout for checkouts - a root 
directory (named "`gpii`" or "`gpii-root`") with a subdirectory named `node_modules`. Into `node_modules` should be checked
out as siblings this project as "windows" and the universal project as "universal". 

Having checked out this structure, possibly assisted by the checkout script `checkoutUniversal.cmd` run from Windows
itself, you may build the Windows-specific executables (requiring MinGW) using build.cmd and then finally start
the GPII "Flow Manager" server using the script `start.cmd`.
