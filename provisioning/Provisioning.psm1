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
    [string] $visualStudioVersion
  )

  # Find the path to vswhere (which installed with Visual Studio 2017 or Build Tools 2017 or later)
  $vswhere = Get-VSWhere

  if (!$vswhere) {
    throw "Visual Studio $($visualStudioVersion) or Build Tools $($visualStudioVersion) were not found."
  }

  # courtesy of Microsoft: https://github.com/microsoft/vswhere/wiki/Find-MSBuild/62adac8eb22431fa91d94e03503d76d48a74939c
  $path = &$vswhere -version $visualStudioVersion -products * -requires Microsoft.Component.MSBuild -property installationPath
  if ($path) {
    $msbuild = Join-Path $path 'MSBuild\Current\Bin\MSBuild.exe'
    if (-not (test-path $msbuild)) {
      $msbuild = join-path $path 'MSBuild\15.0\Bin\MSBuild.exe'
      if (-not (test-path $msbuild)) {
        throw "MSBuild from Visual Studio $($visualStudioVersion) or Build Tools $($visualStudioVersion) could not be found"
      }
    }
  }

  return $msbuild
}

Function Get-CSharpCompiler {
  Param (
    [Parameter(Mandatory=$true)]
    [string] $visualStudioVersion
  )

  # Find the path to vswhere (which installed with Visual Studio 2017 or Build Tools 2017 or later)
  $vswhere = Get-VSWhere

  if (!$vswhere) {
    throw "Visual Studio $($visualStudioVersion) or Build Tools $($visualStudioVersion) were not found."
  }

  # adapted from: https://github.com/microsoft/vswhere/wiki/Find-MSBuild/62adac8eb22431fa91d94e03503d76d48a74939c
  $path = &$vswhere -version $visualStudioVersion -products * -requires Microsoft.VisualStudio.Component.Roslyn.Compiler -property installationPath
  if ($path) {
    $csc = Join-Path $path 'MSBuild\Current\Bin\Roslyn\csc.exe'
    if (-not (test-path $csc)) {
      $csc = join-path $path 'MSBuild\15.0\Bin\Roslyn\csc.exe'
      if (-not (test-path $csc)) {
        throw "csc from Visual Studio $($visualStudioVersion) or Build Tools $($visualStudioVersion) could not be found"
      }
    }
  }

  return $csc
}

Function Get-VSWhere {
  # NOTE: this function is compatible with Visual Studio 2017+

  # Valid Visual Studio versions options are [15.0, "[15.0,16.0)", etc.]

  # Find the path to vswhere (which installed with Visual Studio 2017 or Build Tools 2017 or later)
  if (${Env:ProgramFiles(x86)}) {
    $vswhere = Join-Path ${Env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
  } else {
    $vswhere = Join-Path ${Env:ProgramFiles} "Microsoft Visual Studio\Installer\vswhere.exe"
  }

  return $vswhere
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
Export-ModuleMember -Function Get-CSharpCompiler
Export-ModuleMember -Function Invoke-Environment
Export-ModuleMember -Function Add-Path
