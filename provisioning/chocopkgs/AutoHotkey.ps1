<#
  This script install everything needed by Win10 Simplification module.

  If the script is copied and run from a temporary folder (like when running via vagrant)
  the -originalBuildScriptPath parameter should be passed with the path to the original
  "provisioning" folder
#>

param ( # default to script path if no parameter is given
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath)
)

if ($originalBuildScriptPath -eq (Split-Path -parent $PSCommandPath)) {
  Import-Module "$($originalBuildScriptPath)/../Provisioning.psm1" -Force
} else {
  Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force
}

$chocolatey = "$env:ChocolateyInstall\bin\choco.exe" -f $env:SystemDrive

# Install AutoHotKey package.
Invoke-Command $chocolatey "install autohotkey --yes --force"
refreshenv

exit 0
