<#
  This script install all the chocolatey packages and configure them.
#>
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'Provisioning.psm1') -Force

$chocolatey = "$env:ChocolateyInstall\bin\choco.exe" -f $env:SystemDrive

$nodePath = "C:\Program Files (x86)\nodejs"
Invoke-Command $chocolatey "install nodejs.install -version 6.3.1 --forcex86 -y"
# TODO: Correct path and automatically added is this one
# C:\Users\vagrant\AppData\Roaming\npm review it.
#Add-Path $nodePath $true
refreshenv

Invoke-Command "$($nodePath)\npm.cmd" "install node-gyp@3.4.0 -verbose"

$python2Path = "C:\tools\python2"
Invoke-Command $chocolatey "install python2 -y"
Add-Path $python2Path $true
refreshenv

$innoSetupPath = "C:\Program Files (x86)\Inno Setup 5"
Invoke-Command $chocolatey "install innosetup -y"
Add-Path $innoSetupPath $true
refreshenv

exit 0
