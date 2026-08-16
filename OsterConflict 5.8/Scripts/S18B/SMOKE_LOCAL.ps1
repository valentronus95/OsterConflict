param(
    [string]$BuildRoot = "",
    [int]$Port = 7777,
    [int]$WarmupSeconds = 10,
    [int]$PlaySeconds = 20
)
$ErrorActionPreference="Stop"
$ScriptRoot=Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot=(Resolve-Path (Join-Path $ScriptRoot "..\..")).Path
if ([string]::IsNullOrWhiteSpace($BuildRoot)) { $BuildRoot=Join-Path $ProjectRoot "Build\S18B" }
$BuildRoot=(Resolve-Path $BuildRoot).Path
$ClientRoot=Join-Path $BuildRoot "Client"
$ServerRoot=Join-Path $BuildRoot "Server"
$ServerExe=Get-ChildItem $ServerRoot -Recurse -Filter "OsterConflictServer.exe" -ErrorAction Stop | Select-Object -First 1
$ClientExe=Get-ChildItem $ClientRoot -Recurse -Filter "OsterConflictClient.exe" -ErrorAction Stop | Select-Object -First 1
if (-not $ServerExe -or -not $ClientExe) { throw "Packaged client/server executables not found under $BuildRoot" }

$SmokeLogRoot=Join-Path $BuildRoot "SmokeLogs"
if(Test-Path $SmokeLogRoot){ Remove-Item $SmokeLogRoot -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory -Force -Path $SmokeLogRoot | Out-Null
$ServerLog=Join-Path $SmokeLogRoot "Server.log"
$ClientALog=Join-Path $SmokeLogRoot "Client_SmokeAlpha.log"
$ClientBLog=Join-Path $SmokeLogRoot "Client_SmokeBravo.log"

function Get-LogText([string]$Path){
    if(!(Test-Path $Path)){ return "" }
    try { return (Get-Content $Path -Raw -ErrorAction Stop) } catch { return "" }
}
function Wait-LogContains([string]$Path,[string]$Needle,[datetime]$Deadline,[string]$Label,$Process){
    do {
        if($Process -and $Process.HasExited){ throw "$Label process exited with code $($Process.ExitCode) while waiting for '$Needle'. Log: $Path" }
        $text=Get-LogText $Path
        if($text.Contains($Needle)){ return }
        Start-Sleep -Milliseconds 500
    } until((Get-Date) -ge $Deadline)
    throw "$Label did not emit required marker '$Needle' before timeout. Log: $Path"
}
function Assert-NoFatal([string]$Path,[string]$Label){
    $text=Get-LogText $Path
    if([string]::IsNullOrEmpty($text)){ throw "$Label log was not created: $Path" }
    if($text -match 'Fatal error:|Assertion failed:|LowLevelFatalError|Unhandled Exception'){ throw "$Label contains a fatal/assert marker: $Path" }
}

$ServerArgs="/Game/Maps/OsterConflict_Runtime?Bots=4?Population=6?BotDifficulty=Normal?PerfProfile=Balanced -log -port=$Port -culture=uk-UA -ABSLOG=`"$ServerLog`""
$ClientA="127.0.0.1:$Port?Protocol=18?Name=SmokeAlpha?Role=Medic?AutoDeploy=1 -windowed -ResX=960 -ResY=540 -log -culture=uk-UA -ABSLOG=`"$ClientALog`""
$ClientB="127.0.0.1:$Port?Protocol=18?Name=SmokeBravo?Role=Support?AutoDeploy=1 -windowed -ResX=960 -ResY=540 -WinX=980 -WinY=0 -log -culture=uk-UA -ABSLOG=`"$ClientBLog`""
$procs=@()
try {
    Write-Host "[R6] Starting dedicated server: $($ServerExe.FullName)"
    $server=Start-Process -FilePath $ServerExe.FullName -ArgumentList $ServerArgs -PassThru
    $procs += $server

    $serverDeadline=(Get-Date).AddSeconds([Math]::Max(45,$WarmupSeconds))
    $bound=$false
    do {
        Start-Sleep -Milliseconds 500
        if ($server.HasExited) { throw "Server exited during startup with code $($server.ExitCode). Log: $ServerLog" }
        $bound=[bool](Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue)
    } until($bound -or (Get-Date) -ge $serverDeadline)
    if(-not $bound){ throw "Server stayed alive but did not bind UDP port $Port before smoke clients. Log: $ServerLog" }
    Wait-LogContains $ServerLog 'OC_SERVER_READY' $serverDeadline 'Server' $server

    Write-Host "[R6] Server ready on UDP $Port; starting two protocol-18 auto-deploy clients"
    $a=Start-Process -FilePath $ClientExe.FullName -ArgumentList $ClientA -PassThru; $procs += $a
    Start-Sleep -Seconds 1
    $b=Start-Process -FilePath $ClientExe.FullName -ArgumentList $ClientB -PassThru; $procs += $b

    $joinDeadline=(Get-Date).AddSeconds(35)
    Wait-LogContains $ServerLog 'Human joined: SmokeAlpha' $joinDeadline 'Client SmokeAlpha/server join' $a
    Wait-LogContains $ServerLog 'Human joined: SmokeBravo' $joinDeadline 'Client SmokeBravo/server join' $b

    # After proving both clients actually joined, keep the session alive briefly to catch immediate runtime failures.
    Start-Sleep -Seconds $PlaySeconds
    foreach($p in $procs) { if ($p.HasExited) { throw "Process $($p.Id) exited unexpectedly with code $($p.ExitCode)" } }

    Assert-NoFatal $ServerLog 'Server'
    Assert-NoFatal $ClientALog 'Client SmokeAlpha'
    Assert-NoFatal $ClientBLog 'Client SmokeBravo'
    $serverText=Get-LogText $ServerLog
    if($serverText -match 'VERSION_MISMATCH|SERVER_FULL_HUMANS'){ throw "Unexpected connection rejection marker found in server smoke log: $ServerLog" }

    @(
        'R6 PACKAGED LOCAL SMOKE: PASS',
        'Required evidence:',
        '  - server emitted OC_SERVER_READY',
        '  - SmokeAlpha joined',
        '  - SmokeBravo joined',
        '  - all three processes remained alive',
        '  - no fatal/assert marker in explicit ABSLOG files'
    ) | Set-Content -Encoding UTF8 (Join-Path $SmokeLogRoot 'SMOKE_RESULT.txt')
    Write-Host "R6 PACKAGED LOCAL SMOKE: PASS" -ForegroundColor Green
}
finally {
    foreach($p in $procs) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue } }
}
