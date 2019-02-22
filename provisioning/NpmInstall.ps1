<#
  This script is executed by `npm install`, and builds things required by gpii-windows.
#>
# Turn verbose on, change to "SilentlyContinue" for default behaviour.
$VerbosePreference = "continue"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir = Split-Path -Parent $scriptDir

# Include main Provisioning module.
Import-Module (Join-Path $scriptDir 'Provisioning.psm1') -Force -Verbose

$msbuild = Get-MSBuild "4.0"

# Build the settings helper
$settingsHelperDir = Join-Path $rootDir "settingsHelper"
Invoke-Command $msbuild "SettingsHelper.sln /p:Configuration=Release /p:Platform=`"Any CPU`" /p:FrameworkPathOverride=`"C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1`"" $settingsHelperDir

# Build the process test helper
$testProcessHandlingDir = Join-Path $rootDir "gpii\node_modules\processHandling\test"
$csc = Join-Path -Path (Split-Path -Parent $msbuild) csc.exe
Invoke-Command $csc "/target:exe /out:test-window.exe test-window.cs" $testProcessHandlingDir

# Build the Windows Service
$serviceDir = Join-Path $rootDir "gpii-service"
Invoke-Command "npm" "install" $serviceDir
