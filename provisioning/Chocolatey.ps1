<#
  This script install all the chocolatey packages and configure them.

  If the script is copied and run from a temporary folder (like when running via vagrant)
    the -originalBuildScriptPath parameter should be passed with the path to the original
    "provisioning" folder
#>

param ( # default to script path if no parameter is given
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath)
)

Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force

$chocolatey = "$env:ChocolateyInstall\bin\choco.exe" -f $env:SystemDrive

$nodePath = "C:\Program Files (x86)\nodejs"
$nodeVersion = "12.16.3"
Invoke-Command $chocolatey "install nodejs.install --version $($nodeVersion) --forcex86 -y"
# TODO: Correct path and automatically added is this one
# C:\Users\vagrant\AppData\Roaming\npm review it.
#Add-Path $nodePath $true
# Call nodevars.bat through Invoke-Environment to have available all the environment vars
# setted by the script in the PS1 environment.
Invoke-Environment (Join-Path $nodePath "nodevars.bat")
refreshenv

$python2Path = "C:\tools\python2"
Invoke-Command $chocolatey "install python2 -y"
Add-Path $python2Path $true
refreshenv

Invoke-Command $chocolatey "install nuget.commandline"
refreshenv

exit 0
