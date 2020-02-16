<#
  This script is executed by `npm install`, and builds things required by gpii-windows.
#>
# Turn verbose on, change to "SilentlyContinue" for default behaviour.
$VerbosePreference = "continue"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir = Split-Path -Parent $scriptDir

if (${Env:ProgramFiles(x86)}) {
  $programFilesX86Path = ${Env:ProgramFiles(x86)}
} else {
  # This is required for compatibility with Intel 32-bit systems (since the ProgramFiles(x86) environment variable does not exist on 32-bit Windows)
  $programFilesX86Path = ${Env:ProgramFiles}
}
$programFilesPath = ${Env:ProgramFiles}

# Include main Provisioning module.
Import-Module (Join-Path $scriptDir 'Provisioning.psm1') -Force -Verbose

# For Microsoft's build tools, we will accept any version >=15.0 and <16.0 (i.e. VS2017)
$visualStudioVersion = "[15.0,16.0)"

# Capture the full file path of MSBuild 
$msbuild = Get-MSBuild $visualStudioVersion

# Capture the full file path of the C# compiler
$csc = Get-CSharpCompiler $visualStudioVersion

# Build the settings helper
$settingsHelperDir = Join-Path $rootDir "settingsHelper"
Invoke-Command $msbuild "SettingsHelper.sln /p:Configuration=Release /p:Platform=`"Any CPU`" /p:FrameworkPathOverride=`"$($programFilesX86Path)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1`"" $settingsHelperDir

$volumeControlDir = Join-Path $rootDir "gpii\node_modules\nativeSettingsHandler\nativeSolutions\VolumeControl"
Invoke-Command $msbuild "VolumeControl.sln /p:Configuration=Release /p:Platform=`"x86`" /p:FrameworkPathOverride=`"$($programFilesX86Path)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1`"" $volumeControlDir

# Build the process test helper
$testProcessHandlingDir = Join-Path $rootDir "gpii\node_modules\processHandling\test"
Invoke-Command $csc "/target:exe /out:test-window.exe test-window.cs" $testProcessHandlingDir

# Build the Windows Service
$serviceDir = Join-Path $rootDir "gpii-service"
Invoke-Command "npm" "install" $serviceDir
