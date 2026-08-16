param([int]$Port=7777)
$ErrorActionPreference='Stop'
$TestRoot=Join-Path $PSScriptRoot 'TEST_RESULTS'
if(!(Test-Path $TestRoot)){ throw 'No TEST_RESULTS directory yet. Run a successful Full validation first.' }
$candidates=Get-ChildItem $TestRoot -Directory | Sort-Object Name -Descending
$chosen=$null;$serverExe=$null;$clientExe=$null
foreach($c in $candidates){
    $pkg=Join-Path $c.FullName 'Package'
    if(!(Test-Path $pkg)){continue}
    $s=Get-ChildItem (Join-Path $pkg 'Server') -Recurse -Filter 'OsterConflictServer.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
    $cl=Get-ChildItem (Join-Path $pkg 'Client') -Recurse -Filter 'OsterConflictClient.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
    if($s -and $cl){$chosen=$c;$serverExe=$s;$clientExe=$cl;break}
}
if(!$chosen){throw 'No packaged Client+Server pair found. Full validation must reach Package Client and Package Server first.'}
if(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue){throw "UDP port $Port is already in use."}
$launchDir=Join-Path $chosen.FullName 'ManualLaunchLogs'; New-Item -ItemType Directory -Force -Path $launchDir|Out-Null
$serverLog=Join-Path $launchDir 'Server.log'
$clientLog=Join-Path $launchDir 'Client.log'
$serverArgs="/Game/Maps/OsterConflict_Runtime?Bots=8?Population=8?BotDifficulty=Normal?PerfProfile=Balanced -log -port=$Port -culture=uk-UA -ABSLOG=`"$serverLog`""
$clientArgs="/Game/Maps/OsterConflict_Runtime -Frontend -log -windowed -ResX=1280 -ResY=720 -culture=uk-UA -ABSLOG=`"$clientLog`""
Write-Host "Starting server: $($serverExe.FullName)" -ForegroundColor Cyan
$server=Start-Process $serverExe.FullName -ArgumentList $serverArgs -PassThru
$deadline=(Get-Date).AddSeconds(30)
$bound=$false
do {
    Start-Sleep 1
    if($server.HasExited){throw "Server exited during startup with code $($server.ExitCode)"}
    $bound = [bool](Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue)
} until($bound -or (Get-Date) -ge $deadline)
if(-not $bound){throw "Server process stayed alive but did not bind UDP port $Port within 30 seconds. Check $serverLog"}
$readyDeadline=(Get-Date).AddSeconds(15)
$ready=$false
do {
    if($server.HasExited){throw "Server exited while waiting for OC_SERVER_READY. Check $serverLog"}
    if(Test-Path $serverLog){
        try { $ready=(Get-Content $serverLog -Raw -ErrorAction Stop).Contains('OC_SERVER_READY') } catch {}
    }
    if(-not $ready){Start-Sleep -Milliseconds 500}
} until($ready -or (Get-Date) -ge $readyDeadline)
if(-not $ready){throw "Server bound UDP $Port but did not emit OC_SERVER_READY. Check $serverLog"}
Write-Host "Server is ready on UDP $Port. Starting client with Frontend menu. Click LOCAL SERVER 127.0.0.1:$Port in the UI." -ForegroundColor Green
$client=Start-Process $clientExe.FullName -ArgumentList $clientArgs -PassThru
@("ServerPID=$($server.Id)","ClientPID=$($client.Id)","Results=$($chosen.FullName)") | Set-Content -Encoding UTF8 (Join-Path $launchDir 'SESSION.txt')
Write-Host 'Local session started. Close the game window when finished; the dedicated server may remain running until closed in Task Manager or with STOP_LOCAL_SERVER.cmd.' -ForegroundColor Yellow
$server.Id | Set-Content -Encoding ASCII (Join-Path $PSScriptRoot 'LAST_SERVER_PID.txt')
