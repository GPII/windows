<#
  This script execute all the needed steps to build all the Windows GPII
  components.
#>
# Turn verbose on, change to "SilentlyContinue" for default behaviour.
$VerbosePreference = "continue"

# Include main Provisioning module.
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Import-Module (Join-Path $scriptDir 'Provisioning.psm1') -Force -Verbose
Import-Module "$env:ChocolateyInstall\helpers\chocolateyInstaller.psm1" -Force -Verbose

# Obtain some useful paths.
$systemDrive = $env:SystemDrive
$mainDir = "$systemDrive\vagrant"

# Acquire information about the system and environment.
$winVersion = [System.Environment]::OSVersion
$OSBitness = Get-OSBitness
$processorBits = Get-ProcessorBits

Write-Verbose "Calling build in $($winVersion.VersionString) - OS $($OSBitness)bits - Processor $($processorBits)bits"
Write-Verbose "PSModulePath is $($env:PSModulePath)"
Write-Verbose "systemDrive is $($systemDrive)"
Write-Verbose "mainDir is $($mainDir)"

Invoke-Command "npm" "install" $mainDir

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
$msbuild = Get-MSBuild "4.0"
$listenersDir = Join-Path $mainDir "listeners"
Invoke-Command $msbuild "listeners.sln /nodeReuse:false /p:Configuration=Release /p:FrameworkPathOverride=`"C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1`"" $listenersDir

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
$testProcessHandlingDir = Join-Path $mainDir "gpii\node_modules\processHandling\test"
Invoke-Command "cl" "test-window.c" $testProcessHandlingDir
rm (Join-Path $testProcessHandlingDir "test-window.obj")
