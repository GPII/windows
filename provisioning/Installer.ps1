<#
  This script create all the installers for Windows GPII.
#>
Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) 'Provisioning.psm1') -Force

$installerRepo = "https://github.com/GPII/gpii-wix-installer"
$installerBranch = "master"

$mainDir = Join-Path $env:SystemDrive "vagrant"
$installerDir = Join-Path $env:SystemDrive "installer"
$npm = "npm" -f $env:SystemDrive
$git = "git" -f $env:SystemDrive
$node = Get-Command "node.exe" | Select -expandproperty Path

if (!(Test-Path -Path $installerDir)){
    Invoke-Command $git "clone --branch $($installerBranch) $($installerRepo) $($installerDir)"
}

# Integrate into installer the current version of Node.js in the host machine.
$nodeDestinationDir = Join-Path $installerDir "staging"
Write-Verbose "Copying $($node) to $($nodeDestinationDir) directory for including it into the installer."
cp $node -Destination $nodeDestinationDir -Force

$stagingWindowsDir = [io.path]::combine($installerDir, "staging", "windows")
if (Test-Path -Path $stagingWindowsDir) {
    rm $stagingWindowsDir -Recurse -Force
}
md $stagingWindowsDir

# We are exiting with as a successful value if robocopy error is less or equal to 3
# to avoid interruption. http://ss64.com/nt/robocopy-exit.html
Invoke-Command "robocopy" "..\gpii         $(Join-Path $stagingWindowsDir "gpii")         /job:windows.rcj *.*" (Join-Path $mainDir "provisioning") -errorLevel 3
Invoke-Command "robocopy" "..\node_modules $(Join-Path $stagingWindowsDir "node_modules") /job:windows.rcj *.*" (Join-Path $mainDir "provisioning") -errorLevel 3
Invoke-Command "robocopy" "..\tests        $(Join-Path $stagingWindowsDir "tests")        /job:windows.rcj *.*" (Join-Path $mainDir "provisioning") -errorLevel 3
Invoke-Command "robocopy" ".. $($stagingWindowsDir) gpii.js index.js package.json package-lock.json README.md LICENSE.txt /NFL /NDL" (Join-Path $mainDir "provisioning") -errorLevel 3

Invoke-Command $npm "prune --production" $stagingWindowsDir

# Create the dir for holding data for GPII
Invoke-Expression "$env:SystemDrive\vagrant\provisioning\envChanges\CreateDataDir.ps1"

md (Join-Path $installerDir "output")
md (Join-Path $installerDir "temp")

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
$setupDir = Join-Path $installerDir "setup"
$msbuild = Get-MSBuild "4.0"
Invoke-Command $msbuild "setup.msbuild" $setupDir

# Install MSVC common tools for VC++ development
$tempDir = Join-Path $installerDir "temp"
$msvcInstaller = Join-Path $tempDir "msvc-inst.exe"
wget "https://download.microsoft.com/download/0/B/C/0BC321A4-013F-479C-84E6-4A2F90B11269/vs_community.exe" -OutFile $msvcInstaller

# Visual studio parameters
$select = '/InstallSelectableItems'
$quiet = '/quiet'

# Visual studio components
$ATL = 'NativeLanguageSupport_VCV1'
$ATLMF = 'NativeLanguageSupport_MFCV1'

Invoke-Command $msvcInstaller "$select `"$ATL;$ATLMF`" $quiet"
