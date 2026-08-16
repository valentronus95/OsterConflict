param(
    [Parameter(Mandatory=$true)][string]$ClientExe,
    [string]$Address = "127.0.0.1:7777",
    [string]$Username = "NetQA",
    [switch]$DryRun
)
$ErrorActionPreference = "Stop"
$profiles = @(
    @{ Id="NET-00"; Lag=0;   Loss=0;  Jitter=0  },
    @{ Id="NET-01"; Lag=80;  Loss=1;  Jitter=10 },
    @{ Id="NET-02"; Lag=150; Loss=3;  Jitter=20 },
    @{ Id="NET-03"; Lag=250; Loss=5;  Jitter=35 },
    @{ Id="NET-04"; Lag=500; Loss=10; Jitter=50 }
)
if (!(Test-Path $ClientExe)) { throw "Client exe not found: $ClientExe" }
foreach ($p in $profiles) {
    $url = "$Address`?Protocol=18`?Name=$Username-$($p.Id)"
    $cmds = "NetEmulation.PktLag $($p.Lag),NetEmulation.PktLoss $($p.Loss),NetEmulation.PktJitter $($p.Jitter)"
    $args = @($url, "-log", "-ExecCmds=`"$cmds`"")
    Write-Host "[$($p.Id)] $ClientExe $($args -join ' ')"
    if (!$DryRun) {
        Start-Process -FilePath $ClientExe -ArgumentList $args
        Write-Host "Run the NET scenario, capture logs/evidence, close the client, then continue."
        Read-Host "Press Enter for next profile"
    }
}
