<#
    Script that provision the machine for the Windows 10 morpher.
#>

Import-Module (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) '../Provisioning.psm1') -Force

$VerbosePreference = "continue"

$chocolatey = "$env:ChocolateyInstall\bin\choco.exe" -f $env:SystemDrive
$classicShellVers = "4.3.0.0"
$classicShellDir = "C:\Program Files\Classic Shell\"
$classicShellPath = "C:\Program Files\Classic Shell\ClassicStartMenu.exe"
$classicShellProcessName = "ClassicStartMenu"
$classicShellStopScript = Join-Path $(Split-Path -Parent $MyInvocation.MyCommand.Path) "ClassicShellStop.ps1"

function installClassicShell()
{
    try
    {
        <# Classic-Shell installation #>
        Invoke-Command $chocolatey "install classic-shell --version $($classicShellVers) -y" "" 0
    }
    catch
    {
        <# We should notify that IoD have fail #>
        $ErrorMessage = $_.Except0on.Message
        Write-Verbose "$ErrorMessage"
    }
}

function selectVal($regPath, $val)
{
    if ($val.type -eq "REG_DWORD") {
        New-ItemProperty -Path $regpath -Name $val.name -Value $val.value `
        -PropertyType DWORD -Force | Out-Null
    } elseif ($val.type -eq "REG_SZ") {
        New-ItemProperty -Path $regpath -Name $val.name -Value $val.value `
        -PropertyType String -Force | Out-Null
    } else {
        Write-Error ("Unvalid type: " + $val.type)
    }
}

function createNewKey($regPath, $vals) {
    if(!(Test-Path $regPath)) {
        New-Item -Path $regPath -Force | Out-Null
    }

    if ($vals -ne $null) {
        foreach($val in $vals) {
            selectVal $regPath $val
        }
    }
}

function setClassicShellRegistryKeys() {
    $registryPath = "HKCU:\Software\IvoSoft"
    $Name = "Version"
    $value = "1"

    # Disable first run menu selection
    $settingsPath = "$registryPath\ClassicStartMenu"
    $vals = @((newVal "ShowedStyle2" "REG_DWORD" 1))

    createNewKey $settingsPath $vals

    $settingsPath = "$registryPath\ClassicStartMenu\Settings"
    $vals = @((newVal "EnableStartButton" "REG_DWORD" 1),
              (newVal "StartButtonType" "REG_SZ" "CustomButton"),
              (newVal "StartButtonPath" "REG_SZ" "$env:LOCALAPPDATA\GPII\logo\win-logo.png"),
              (newVal "StartButtonSize" "REG_DWORD" 42),
              (newVal "SkipMetro" "REG_DWORD" 1),
              (newVal "MenuStyle" "REG_SZ" "Win7"),
              (newVal "SkinW7" "REG_SZ" "Windows Aero"),
              (newVal "AutoStart" "REG_DWORD" 0))

    createNewKey $settingsPath $vals
}

function newVal($name, $type, $val)
{
    $keyobj = New-Object -TypeName PSObject
    $keyobj | Add-Member -MemberType NoteProperty -Name name -Value $name
    $keyobj | Add-Member -MemberType NoteProperty -Name type -Value $type
    $keyobj | Add-Member -MemberType NoteProperty -Name value -Value $val

    return $keyobj
}

function stopClassicShell() {
    try
    {
        <# Currently there is no way of being sure classicShell has exit #>
        $process = Get-Process $ClassicShellProcessName -ea SilentlyContinue

        if ($process)
        {
            Invoke-Command $classicShellPath "-exit" "" 0
            Write-Verbose "Classic-Shell stopped"
        }
    }
    catch
    {
        <# ($_.FullyQualifiedErrorId).split(',')[0] #>
        Write-Verbose ("Failed to restore correct Windows Menu " + $_.FullyQualifiedErrorId)
    }
}

installClassicShell
setClassicShellRegistryKeys
stopClassicShell

refreshenv

exit 0
