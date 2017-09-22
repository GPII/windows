<#
  .
#>

$VerbosePreference = "continue"

$GPII_Data_Path = Join-Path $env:LOCALAPPDATA "GPII"

Write-Verbose("Starting $PSCommandPath and set GPII_Data_Path: $GPII_Data_Path")

$TabletModeScript = Join-Path $GPII_Data_Path "scripts\TabletView.ahk"
Write-Verbose("Launching tabletModeScript : $TabletModeScript")
Start-Process -FilePath $TabletModeScript 0
