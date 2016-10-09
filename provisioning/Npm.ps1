<#
  This script install and setup all the needed npm packages.
#>
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'Provisioning.psm1') -Force

$npm = "npm" -f $env:SystemDrive

Invoke-Command $npm "config set msvs_version 2015 --global"
Invoke-Command $npm "install grunt-cli -g"
Invoke-Command $npm "install testem -g"
refreshenv

exit 0
