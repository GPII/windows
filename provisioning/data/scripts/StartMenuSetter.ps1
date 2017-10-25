<#
  .
#>

#############################################################################
#  If Powershell is running the 32-bit version on a 64-bit machine, we
#           need to force powershell to run in 64-bit mode .
#############################################################################

if ($env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
    write-warning "Not running in 64bits, relaunching script in 64 bit mode"
    if ($myInvocation.Line) {
        &"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile $myInvocation.Line
    }else{
        &"$env:WINDIR\sysnative\windowspowershell\v1.0\powershell.exe" -NonInteractive -NoProfile -file "$($myInvocation.InvocationName)" $args
    }
exit $lastexitcode
}

# Get the ID and security principal of the current user account
$myWindowsID=[System.Security.Principal.WindowsIdentity]::GetCurrent()
$myWindowsPrincipal=new-object System.Security.Principal.WindowsPrincipal($myWindowsID)

# Get the security principal for the Administrator role
$adminRole=[System.Security.Principal.WindowsBuiltInRole]::Administrator

# Check to see if we are currently running "as Administrator"
if ($myWindowsPrincipal.IsInRole($adminRole)) {
  # We are running "as Administrator" - so change the title and background color to indicate this
  $Host.UI.RawUI.WindowTitle = $myInvocation.MyCommand.Definition + "(Elevated)"
  $Host.UI.RawUI.BackgroundColor = "DarkBlue"
  clear-host
} else {
  # We are not running "as Administrator" - so relaunch as administrator

  # Create a new process object that starts PowerShell
  $newProcess = new-object System.Diagnostics.ProcessStartInfo "PowerShell";
  # Specify the current script path and name as a parameter
  $newProcess.Arguments = $myInvocation.MyCommand.Definition;
  # Indicate that the process should be elevated
  $newProcess.Verb = "runas";
  # Avoid the display of the window.
  $newProcess.WindowStyle =  [System.Diagnostics.ProcessWindowStyle]::Hidden;
  # Start the new process
  [System.Diagnostics.Process]::Start($newProcess);

  # Exit from the current, unelevated, process
  exit
}

$VerbosePreference = "continue"
$ErrorActionPreference = "Stop"

$GPII_Data_Path = Join-Path $env:LOCALAPPDATA "GPII"

Write-Verbose("Starting $PSCommandPath and set GPII_Data_Path : $GPII_Data_Path ")

Write-Verbose("Exporting current layout.")
$CurrentLayout = Join-Path $GPII_Data_Path "menuLayouts\current_layout.xml"
$CurError = Join-Path $GPII_Data_Path "menuLayouts\Error.txt"

Try {
    Export-StartLayout -Path $CurrentLayout -ErrorAction Stop
} Catch {
    $ErrorMessage = $_.Exception.Message
    New-Item -Path $CurError -type file -force -value $ErrorMessage
    Break
}

$NewLayout = Join-Path $GPII_Data_Path "menuLayouts\StartMenuLayout.xml"
Write-Verbose("Writing layout from $NewLayout")
reg add "HKCU\Software\Policies\Microsoft\Windows\Explorer" /f /v "StartLayoutFile" /t REG_SZ /d "$NewLayout"
reg add "HKCU\Software\Policies\Microsoft\Windows\Explorer" /f /v "LockedStartLayout" /t REG_DWORD /d "1"

taskkill /f /im "ShellExperienceHost.exe"
