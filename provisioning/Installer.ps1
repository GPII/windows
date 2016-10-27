<#
  This script create all the installers for Windows GPII.
#>
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'Provisioning.psm1') -Force

$installerRepo = "https://github.com/gpii/gpii-wix-installer"
$installerBranch = "v1.1.0"

$mainDir = Join-Path $env:SystemDrive "vagrant"
$installerDir = Join-Path $env:SystemDrive "installer"
$npm = "npm" -f $env:SystemDrive
$git = "git" -f $env:SystemDrive

if (!(Test-Path -Path $installerDir)){
    Invoke-Command $git "clone --branch $($installerBranch) $($installerRepo) $($installerDir)"
}

$stagingWindowsDir = [io.path]::combine($installerDir, "staging", "windows")
if (Test-Path -Path $stagingWindowsDir) {
    rm $stagingWindowsDir -Recurse -Force
}

# We are exiting with as a successful value if robocopy error is less or equal to 3
# to avoid interruption. http://ss64.com/nt/robocopy-exit.html
Invoke-Command "robocopy" "/job:windows.rcj *.*" (Join-Path $mainDir "provisioning") -errorLevel 3

Invoke-Command $npm "prune --production" $stagingWindowsDir

md (Join-Path $installerDir "output")
md (Join-Path $installerDir "temp")

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
$setupDir = Join-Path $installerDir "setup"
$msbuild = Get-MSBuild "4.0"
Invoke-Command $msbuild "setup.msbuild" $setupDir
