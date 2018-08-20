<#
  This script install and setup all the needed npm packages.

  If the script is copied and run from a temporary folder (like when running via vagrant)
    the -originalBuildScriptPath parameter should be passed with the path to the original
    "provisioning" folder
#>
param ( # default to script path if no parameter is given
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath)
)

Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force

$npm = "npm" -f $env:SystemDrive

Invoke-Command $npm "config set msvs_version 2015 --global"
Invoke-Command $npm "install grunt-cli -g"
Invoke-Command $npm "install testem -g"
refreshenv

exit 0