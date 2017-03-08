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

if (-Not $skipNpm) {
    Invoke-Command "npm" "install" $mainDir
}

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools_msbuild.bat"
$msbuild = Get-MSBuild "4.0"
$listenersDir = Join-Path $mainDir "listeners"
Invoke-Command $msbuild "listeners.sln /nodeReuse:false /p:Configuration=Release /p:FrameworkPathOverride=`"C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.1`"" $listenersDir

Invoke-Expression (Join-Path $originalBuildScriptPath "Tests.ps1")
