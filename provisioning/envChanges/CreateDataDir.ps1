<#
    Script that createss the folder for holding the necessary data for GPII.
#>

$GPII_Data_Path = "$env:LOCALAPPDATA\GPII"

if (!(Test-Path $GPII_Data_Path)) {
    New-Item -Path $GPII_Data_Path -ItemType 'directory' | Out-Null
}

$mainDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$dataDir = Join-Path $mainDir "..\data\*"

Write-Verbose("Deleting: '$GPII_Data_Path'")
Remove-Item "$GPII_Data_Path\*" -Force -Recurse
Write-Verbose("Copying: '$dataDir' to '$GPII_Data_Path'")
Copy-Item -Path $dataDir -Force -Recurse -Destination $GPII_Data_Path -ErrorAction Stop

exit 0
