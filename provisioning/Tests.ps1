<#
  This script builds the artifacts required in order to run the tests of gpii-windows

  If run via a tool (like vagrant) which moves this script to somewhere different
  than its original location within the gpii-app repository, the parameter
  "-originalBuildScriptPath" should be provided, with the original location of the
  script
#>

param (
    [string]$originalBuildScriptPath = (Split-Path -parent $PSCommandPath) # Default to script path.
)

Import-Module "$($originalBuildScriptPath)/Provisioning.psm1" -Force

Write-OutPut "originalBuildScriptPath set to: $($originalBuildScriptPath)"

# Turn verbose on, change to "SilentlyContinue" for default behaviour.
$VerbosePreference = "continue"

Invoke-Environment "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
$testProcessHandlingDir = Join-Path $originalBuildScriptPath "..\gpii\node_modules\processHandling\test"
Invoke-Command "cl" "test-window.c" $testProcessHandlingDir
rm (Join-Path $testProcessHandlingDir "test-window.obj")
