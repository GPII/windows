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
$nodeVersion = "6.9.1"
Invoke-Command $chocolatey "install nodejs.install --version $($nodeVersion) --forcex86 -y"
# TODO: Correct path and automatically added is this one
# C:\Users\vagrant\AppData\Roaming\npm review it.
#Add-Path $nodePath $true
refreshenv

Invoke-Command "$($nodePath)\npm.cmd" "install node-gyp@3.4.0" "$($nodePath)\node_modules\npm"

$python2Path = "C:\tools\python2"
Invoke-Command $chocolatey "install python2 -y"
Add-Path $python2Path $true
refreshenv

# This seems weak. We should track bin path automatically and avoid version
# declaration in the path.
$wixSetupPath = "C:\Program Files (x86)\WiX Toolset v3.10\bin"
Invoke-Command $chocolatey "install wixtoolset -y"
Add-Path $wixSetupPath $true
refreshenv

Invoke-Command $chocolatey "install msbuild.extensionpack -y"
refreshenv

exit 0
