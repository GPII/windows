<#
  This script execute all the needed steps to build all the Windows GPII
  components.

  Requirements:
    * Windows PowerShell 3.0 or greater.
    * PowerShell Core
#>

$VerbosePreference = "continue"

Function Invoke-Command {
  Param (
    [string] $command,
    [string] $arguments,
    [string] $location,
    [int] $errorLevel = 0
  )

  Write-Verbose "Execute-Command Started ($location : $command $arguments)."

  if (!$command) {
    throw "You need to specify a command to Execute-Command."
  }
  if (!$arguments) {
    $arguments = " "
  }
  $originalLocation = Get-Location
  if ($location) {
    Set-Location -Path $location
  }

  $p = Start-Process -FilePath $command -ArgumentList $arguments -PassThru -NoNewWindow
  $handle = $p.Handle
  $p.WaitForExit();

  $exitCode = $p.ExitCode

  Write-Verbose "`'$command`' exited with $($exitCode)"

  Set-Location -Path $originalLocation.path

  if ($exitCode -gt $errorLevel) {
    throw "Installation process returned error code: $($exitCode)"
  }
}

Function Get-MSBuild {
  Param (
    [Parameter(Mandatory=$true)]
    [string] $version
  )

  # TODO: Check version validity.
  # Valid versions are [2.0, 3.5, 4.0]

  $dotNetVersion = $version
  $regKey = "HKLM:\software\Microsoft\MSBuild\ToolsVersions\$dotNetVersion"
  $regProperty = "MSBuildToolsPath"

  $msbuild = Join-Path -path (Get-ItemProperty $regKey).$regProperty -childpath "msbuild.exe"

  return $msbuild
}

Function Invoke-Environment {
  Param (
        [Parameter(Mandatory=$true)]
        [string] $Command
    )

    $Command = "`"" + $Command + "`""
    cmd /c "$Command > nul 2>&1 && set" | . { process {
        if ($_ -match '^([^=]+)=(.*)') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }}
}

# TODO: Currently this function is adding the directory location even if this
# is present in the path. Implement an additional function to check existence.
Function Add-Path {
  Param (
    [Parameter(Mandatory=$true)]
    [string] $Directory,
    [bool] $Permanent = $false
  )

  $regKey = "HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
  $normalizedDirectory = [System.IO.Path]::GetFullPath($Directory)

  # Obtain and set local PATH.
  $currentLocalPath = $env:Path
  $newLocalPath = "$($currentLocalPath);$($normalizedDirectory)"
  $env:Path = $newLocalPath
  Write-Verbose "Add-Path Local $($normalizedDirectory) -> $($newLocalPath)"

  if ($Permanent) {
    # Obtain and set system PATH.
    $currentSystemPath = (Get-ItemProperty -Path $regKey -Name PATH).Path
    $newSystemPath = "$($currentSystemPath);$($normalizedDirectory)"
    Write-Verbose "Add-Path System $($normalizedDirectory) in the registry: $($newSystemPath)."
    Set-ItemProperty -Path $regKey -Value $newSystemPath -Name PATH
  }

  return $newLocalPath
}

Export-ModuleMember -Function Invoke-Command
Export-ModuleMember -Function Get-MSBuild
Export-ModuleMember -Function Invoke-Environment
Export-ModuleMember -Function Add-Path
