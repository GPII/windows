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
    [string] $location
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

  $p = Start-Process $command -ArgumentList $arguments -PassThru -NoNewWindow -Wait

  if (!($p.HasExited)) {
    Write-Verbose "Waiting for `'$command $arguments`' to finish, please wait...."
  } else {
    Write-Verbose "Process `'$command $arguments`' finished."
  }
  Write-Verbose "`'$command`' exited with $($p.ExitCode)"

  Set-Location -Path $originalLocation.path

  if($p.ExitCode -ne 0) {
    throw "Installation process returned error code: $($p.ExitCode)"
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

function Invoke-Environment {
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

Export-ModuleMember -Function Invoke-Command
Export-ModuleMember -Function Get-MSBuild
Export-ModuleMember -Function Invoke-Environment