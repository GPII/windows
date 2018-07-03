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

# Build the Windows Service
$serviceDir = "$mainDir\service"
Invoke-Command "npm" "install" $serviceDir
