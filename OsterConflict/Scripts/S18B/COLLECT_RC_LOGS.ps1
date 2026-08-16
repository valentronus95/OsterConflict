param([string]$BuildRoot="")
$ErrorActionPreference="Stop"
$ScriptRoot=Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot=(Resolve-Path (Join-Path $ScriptRoot "..\..")).Path
if ([string]::IsNullOrWhiteSpace($BuildRoot)) { $BuildRoot=Join-Path $ProjectRoot "Build\S18B" }
$Stamp=Get-Date -Format "yyyyMMdd_HHmmss"
$Dest=Join-Path $ProjectRoot "Build\S18B\CollectedLogs_$Stamp"
New-Item -ItemType Directory -Force -Path $Dest | Out-Null
$sets=@(
    @{ Label='BuildRoot'; Root=$BuildRoot },
    @{ Label='SavedLogs'; Root=(Join-Path $ProjectRoot 'Saved\Logs') },
    @{ Label='SavedCrashes'; Root=(Join-Path $ProjectRoot 'Saved\Crashes') }
)
$Copied=0
foreach($set in $sets) {
    $root=$set.Root
    if (Test-Path $root) {
        $resolved=(Resolve-Path $root).Path.TrimEnd('\')
        $bucket=Join-Path $Dest $set.Label
        Get-ChildItem $resolved -Recurse -File -Include *.log,*.dmp,*.xml,*.json,*.utrace -ErrorAction SilentlyContinue | ForEach-Object {
            $relative=$_.FullName.Substring($resolved.Length).TrimStart('\')
            $target=Join-Path $bucket $relative
            $targetDir=Split-Path -Parent $target
            New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
            Copy-Item $_.FullName -Destination $target -Force
            $Copied++
        }
    }
}
Write-Host "Collected RC diagnostics: $Dest ($Copied files)"
