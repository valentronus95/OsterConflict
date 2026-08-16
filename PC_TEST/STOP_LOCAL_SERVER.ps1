$ErrorActionPreference='Stop'
$pidFile=Join-Path $PSScriptRoot 'LAST_SERVER_PID.txt'
if(!(Test-Path $pidFile)){Write-Host 'No remembered local server PID.';exit 0}
$raw=(Get-Content $pidFile -Raw).Trim()
$serverPid=0
if(-not [int]::TryParse($raw,[ref]$serverPid)){
    Write-Warning "Invalid remembered PID '$raw'; removing stale PID file without stopping any process."
    Remove-Item $pidFile -Force -ErrorAction SilentlyContinue
    exit 1
}
$p=Get-Process -Id $serverPid -ErrorAction SilentlyContinue
if($p){
    if($p.ProcessName -like 'OsterConflict*'){
        Stop-Process -Id $serverPid -Force
        Write-Host "Stopped Oster local server PID $serverPid ($($p.ProcessName))"
    }else{
        Write-Warning "PID $serverPid now belongs to '$($p.ProcessName)', not OsterConflict. Refusing to stop it."
        Remove-Item $pidFile -Force -ErrorAction SilentlyContinue
        exit 1
    }
}else{
    Write-Host 'Remembered server is not running.'
}
Remove-Item $pidFile -Force -ErrorAction SilentlyContinue
exit 0
