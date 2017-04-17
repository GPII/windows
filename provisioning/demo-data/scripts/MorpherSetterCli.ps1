param([String]$pld="None")

$VerbosePreference = "continue"

$ComputerName = '.'
$pipe = new-object System.IO.Pipes.NamedPipeClientStream($ComputerName, 'Morpher', [System.IO.Pipes.PipeDirection]::Out,
                                                        [System.IO.Pipes.PipeOptions]::None,
                                                        [System.Security.Principal.TokenImpersonationLevel]::Impersonation)

$GPII_Demo_Path = "$env:HOMEPATH\AppData\Local\GPII-Demo"
$scripts = "$GPII_Demo_Path\scripts"

try {
    $pipe.Connect(1000);
    break
} catch  {
    Start-Job -FilePath "$scripts\MorpherSetterServ.ps1"
    $pipe.Connect();
}

$sw = new-object System.IO.StreamWriter($pipe);
$sw.WriteLine($command);
$sw.WriteLine('exit');

$sw.Dispose();
$pipe.Dispose();