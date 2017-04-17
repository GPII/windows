<#
    Script that creates that provision the machine
    to be able to fully use the windows morpher.
        - ClassicShell Instalation.
        - Folder structure.
#>

Invoke-Expression ".\ClassicShell-IoD.ps1"

$GPII_Demo_Path = "$env:HOMEPATH\AppData\Local\GPII-Demo"

if (!(Test-Path $GPII_Demo_Path)) {
    New-Item -Path $GPII_Demo_Path -ItemType 'directory' | Out-Null
}

Copy-Item -Path "demo-data\*" -Force -Recurse -Destination $GPII_Demo_Path
