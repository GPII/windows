<#
  This script install all the chocolatey packages and configure them.
#>
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'Provisioning.psm1') -Force

$chocolatey = "$env:ChocolateyInstall\bin\choco.exe" -f $env:SystemDrive;

Invoke-Command $chocolatey "install nodejs.install -version 6.3.1 --forcex86 -y"

refreshenv
Invoke-Command "npm" "install node-gyp@3.4.0" "C:\Program Files (x86)\nodejs\node_modules\npm"

Invoke-Command $chocolatey "install python2 -y"
Invoke-Command $chocolatey "install innosetup -y"

refreshenv
Write-Verbose $env:Path

# TODO: Review that the PATH in this session and others are set.
setx /M PATH "%PATH%;C:\Program Files\nodejs;C:\tools\python2;C:\Program Files (x86)\Inno Setup 5"
