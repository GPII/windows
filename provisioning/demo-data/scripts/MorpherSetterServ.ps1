$VerbosePreference = "continue"

$classicShellProc = "ClassicStartMenu"

# 'Created server side of "\\.\pipe\Wulf"'
$pipe=new-object System.IO.Pipes.NamedPipeServerStream "Morpher", "In";
$pipe.WaitForConnection();

$sr = new-object System.IO.StreamReader($pipe);
while (($command= $sr.ReadLine()) -ne 'exit')
{
    if ($command -eq "In") {
        $classicShell = $null
        Start-Process -FilePath "C:\Program Files\Classic Shell\ClassicStartMenu.exe"

        while($classicShell -eq $null)
        {
            Start-Sleep 1
            $classicShell = Get-Process -Name $classicShellProc -ErrorAction SilentlyContinue
        }

        Stop-Process -Name "explorer"
    } elseif ($command -eq "Out") {
        Start-Process -FilePath "C:\Program Files\Classic Shell\ClassicStartMenu.exe" -ArgumentList "-exit"
        Wait-Process -Name $classicShellProc
        Stop-Process -Name "explorer"
        break
    }
    Start-Sleep 1
};

$sr.Dispose();
$pipe.Dispose();

