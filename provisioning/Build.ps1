<#
  This script execute all the needed steps to build all the Windows GPII
  components.

  If the -skipNpm flag is provided, no npm install will be run
  If the script is copied and run from a temporary folder (like when running via vagrant)
     the -originalBuildScriptPath parameter should be passed with the path to the original
     "provisioning" folder
#>
param ( # default to script path if no parameter is given
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath),
    [switch]$skipNpm # defaults to false
)

# Turn verbose on, change to "SilentlyContinue" for default behaviour.
$VerbosePreference = "continue"

# Include main Provisioning module.
Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force -Verbose
Import-Module "$env:ChocolateyInstall\helpers\chocolateyInstaller.psm1" -Force -Verbose

# Obtain some useful paths.
$mainDir = (get-item $originalBuildScriptPath).parent.FullName

# Acquire information about the system and environment.
$winVersion = [System.Environment]::OSVersion
$OSBitness = Get-OSBitness
$processorBits = Get-ProcessorBits

Write-Verbose "Calling build in $($winVersion.VersionString) - OS $($OSBitness)bits - Processor $($processorBits)bits"
Write-Verbose "PSModulePath is $($env:PSModulePath)"
Write-Verbose "mainDir is $($mainDir)"

Invoke-Command "npm" "install" $mainDir
