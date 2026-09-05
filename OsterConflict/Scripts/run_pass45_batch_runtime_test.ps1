$ErrorActionPreference = "Continue"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$ProjectDir = Join-Path $RepoRoot "OsterConflict"
$UProject = Join-Path $ProjectDir "OsterConflict.uproject"
$AllAssetImport = Join-Path $ProjectDir "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
$MaterialGate = Join-Path $ProjectDir "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
$CurrentGameplay = Join-Path $RepoRoot "RUN_R14_CURRENT_GAMEPLAY.cmd"
$EvidenceVerify = Join-Path $RepoRoot "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
$Finalizer = Join-Path $ProjectDir "Scripts\finalize_asset_acceptance.py"
$BuildBat = "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
$GameplayLog = Join-Path $RepoRoot "Logs\R14_CURRENT_GAMEPLAY.log"
$MaterialLog = Join-Path $RepoRoot "Logs\PASS45_STRICT_MATERIAL_GATE.log"
$WeaponReport = Join-Path $ProjectDir "Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
$LocalInboxRuntimeReport = Join-Path $ProjectDir "Saved\AutomationReports\ProductionModels\local_inbox_runtime_validation.txt"
$LocalWorldRuntimeReport = Join-Path $ProjectDir "Saved\AutomationReports\ProductionModels\local_world_runtime_validation.txt"
$LogDir = Join-Path $RepoRoot "Logs"
$StageDir = Join-Path $LogDir "Pass45Batch"
$Report = Join-Path $LogDir "PASS45_BATCH_RUNTIME_REPORT.txt"

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

$StageResults = @()
$Failures = @()

function Add-ReportLine([string]$Line = "") {
    Add-Content -LiteralPath $Report -Value $Line -Encoding UTF8
}

function Add-Result([string]$Name, [string]$Status, [int]$Code, [string]$LogPath, [bool]$Failure) {
    $item = [pscustomobject]@{ Name = $Name; Status = $Status; Code = $Code; Log = $LogPath }
    $script:StageResults += $item
    if ($Failure) { $script:Failures += $item }
}

function Invoke-Stage([string]$Name, [string]$Command, [string[]]$Arguments = @()) {
    $safeName = ($Name -replace '[^A-Za-z0-9_.-]', '_')
    $stageLog = Join-Path $StageDir ($safeName + ".log")
    if (Test-Path -LiteralPath $stageLog) { Remove-Item -LiteralPath $stageLog -Force }

    Write-Host "[RUN ] $Name"
    if (-not (Test-Path -LiteralPath $Command)) {
        $message = "Missing command: $Command"
        $message | Tee-Object -FilePath $stageLog
        Write-Host "[FAIL] $Name code=127"
        Add-Result $Name "FAIL" 127 $stageLog $true
        return 127
    }

    $global:LASTEXITCODE = 0
    & $Command @Arguments 2>&1 | Tee-Object -FilePath $stageLog
    $rc = $LASTEXITCODE
    if ($null -eq $rc) { $rc = 0 }

    if ($rc -eq 0) {
        Write-Host "[PASS] $Name code=0"
        Add-Result $Name "PASS" 0 $stageLog $false
    } else {
        Write-Host "[FAIL] $Name code=$rc"
        Add-Result $Name "FAIL" ([int]$rc) $stageLog $true
    }
    return [int]$rc
}

function Add-Skipped([string]$Name, [string]$Reason) {
    Write-Host "[SKIP] $Name - $Reason"
    Add-Result $Name "SKIP" 0 $Reason $false
}

function Write-Summary {
    Add-ReportLine ""
    Add-ReportLine "STAGES:"
    foreach ($item in $StageResults) {
        Add-ReportLine ("[{0}] {1} code={2}" -f $item.Status, $item.Name, $item.Code)
        if ($item.Log) { Add-ReportLine ("  log={0}" -f $item.Log) }
    }
    if ($Failures.Count -gt 0) {
        Add-ReportLine ""
        Add-ReportLine "FAILURE TAILS:"
        foreach ($item in $Failures) {
            Add-ReportLine ("--- {0} / code={1} ---" -f $item.Name, $item.Code)
            if (Test-Path -LiteralPath $item.Log) {
                Get-Content -LiteralPath $item.Log -Tail 35 -ErrorAction SilentlyContinue | ForEach-Object { Add-ReportLine ("  " + $_) }
            }
        }
    }
}

Set-Content -LiteralPath $Report -Value @(
    "OSTER CONFLICT - PASS45 BATCH RUNTIME REPORT",
    ("Generated: {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss")),
    ("Repo: {0}" -f $RepoRoot)
) -Encoding UTF8

Write-Host "===================================================================="
Write-Host "OSTER CONFLICT - PASS45 ПАКЕТНИЙ RUNTIME-ТЕСТ"
Write-Host "===================================================================="
Write-Host "Один запуск збирає проблеми без старих дубльованих preflight-ланцюгів."
Write-Host "Локальні Changes не видаляються, не stash і не reset."
Write-Host ""

$branch = (& git -C $RepoRoot branch --show-current 2>$null | Select-Object -First 1).Trim()
$head = (& git -C $RepoRoot rev-parse HEAD 2>$null | Select-Object -First 1).Trim()
$remoteHead = ""
$fetchOk = $false
if ($branch) {
    & git -C $RepoRoot fetch origin $branch 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        $remoteHead = (& git -C $RepoRoot rev-parse ("origin/" + $branch) 2>$null | Select-Object -First 1).Trim()
        $fetchOk = [bool]$remoteHead
    }
}
$trackedChanges = @(& git -C $RepoRoot status --porcelain --untracked-files=no 2>$null)
$trackedCount = $trackedChanges.Count
$exactHead = $fetchOk -and $head -and ($head -eq $remoteHead)
$formalBlocked = ($trackedCount -gt 0) -or (-not $exactHead)

Add-ReportLine ("BRANCH={0}" -f $branch)
Add-ReportLine ("HEAD={0}" -f $head)
Add-ReportLine ("REMOTE_HEAD={0}" -f $remoteHead)
Add-ReportLine ("TRACKED_CHANGES={0}" -f $trackedCount)
Add-ReportLine ("EXACT_REMOTE_HEAD={0}" -f $exactHead)

if ($trackedCount -gt 0) {
    Write-Host "[WARN] Tracked Changes: $trackedCount. Не чіпаю їх; formal acceptance буде BLOCKED."
}
if (-not $exactHead) {
    Write-Host "[WARN] Local HEAD не підтверджений як exact origin/$branch; formal acceptance буде BLOCKED."
}

Write-Host ""
Write-Host "[PREFLIGHT] Канонічні етапи без історичних Stein/Remington/audio wrapper-ів..."

if ($formalBlocked) {
    Add-Skipped "ALL local/Fab assets: prepare + import + runtime bindings" "FORMAL_ACCEPTANCE_BLOCKED_BY_DIRTY_OR_NONEXACT_SOURCE"
} else {
    [void](Invoke-Stage "ALL local/Fab assets: prepare + import + runtime bindings" $AllAssetImport)
}

$buildArgs = @("OsterConflictEditor", "Win64", "Development", ("-Project=" + $UProject), "-WaitMutex")
[void](Invoke-Stage "Final OsterConflictEditor C++ build" $BuildBat $buildArgs)
[void](Invoke-Stage "Strict authored material/dependency gate" $MaterialGate)

if ($Failures.Count -gt 0) {
    Write-Host ""
    Write-Host "===================================================================="
    Write-Host "[STOP] Знайдено $($Failures.Count) реальних preflight проблем. Гру поки не запускаю."
    Write-Host "ЄДИНИЙ ЗВІТ: $Report"
    Add-ReportLine ""
    Add-ReportLine ("PREFLIGHT_RESULT=FAIL count={0}" -f $Failures.Count)
    if ($formalBlocked) { Add-ReportLine "FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE" }
    Write-Summary
    exit 20
}

Write-Host ""
if ($formalBlocked) {
    Write-Host "[RUNTIME] Preflight для запуску пройшов. Запускаю ОДИН diagnostic gameplay без formal acceptance."
    $env:OC_FORCE_ACCEPTANCE = "0"
    $env:OC_VALIDATE_LOCAL_INBOX = "0"
} else {
    Write-Host "[RUNTIME] Clean exact-head preflight пройшов. Запускаю ОДИН formal acceptance gameplay."
    $env:OC_FORCE_ACCEPTANCE = "1"
    $env:OC_VALIDATE_LOCAL_INBOX = "1"
}
$env:OC_RHI_COMPAT = "0"
$runtimeRc = Invoke-Stage "Single gameplay runtime" $CurrentGameplay
Remove-Item Env:OC_FORCE_ACCEPTANCE -ErrorAction SilentlyContinue
Remove-Item Env:OC_VALIDATE_LOCAL_INBOX -ErrorAction SilentlyContinue
Remove-Item Env:OC_RHI_COMPAT -ErrorAction SilentlyContinue

if ($runtimeRc -ne 0) {
    Add-ReportLine ""
    Add-ReportLine ("RUNTIME_RESULT=FAIL code={0}" -f $runtimeRc)
    if ($formalBlocked) { Add-ReportLine "FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE" }
    Write-Summary
    Write-Host "[STOP] Gameplay runtime завершився з code=$runtimeRc"
    Write-Host "ЄДИНИЙ ЗВІТ: $Report"
    exit 21
}

if ($formalBlocked) {
    Add-ReportLine ""
    Add-ReportLine "DIAGNOSTIC_RUNTIME=PASS"
    Add-ReportLine "FORMAL_ACCEPTANCE=BLOCKED_DIRTY_OR_NONEXACT_SOURCE"
    Write-Summary
    Write-Host "===================================================================="
    Write-Host "[PASS] Diagnostic gameplay запустився і завершився без launcher/runtime exit failure."
    Write-Host "[BLOCKED] Formal acceptance не рахується через tracked Changes або non-exact HEAD."
    Write-Host "ЄДИНИЙ ЗВІТ: $Report"
    exit 0
}

$postFailures = @()
if (-not (Test-Path -LiteralPath $LocalInboxRuntimeReport) -or -not (Select-String -LiteralPath $LocalInboxRuntimeReport -SimpleMatch "PASS45_LOCAL_INBOX_RUNTIME=PASS" -Quiet)) {
    $postFailures += "models_game_OC live runtime proof missing/failed"
}
if (-not (Test-Path -LiteralPath $LocalWorldRuntimeReport) -or -not (Select-String -LiteralPath $LocalWorldRuntimeReport -SimpleMatch "PASS45_LOCAL_WORLD_RUNTIME=PASS" -Quiet)) {
    $postFailures += "world asset live runtime proof missing/failed"
}

$pyCommand = Get-Command py -ErrorAction SilentlyContinue
$pythonCommand = Get-Command python -ErrorAction SilentlyContinue
$pythonExe = $null
$pythonPrefix = @()
if ($pyCommand) {
    $pythonExe = $pyCommand.Source
    $pythonPrefix = @("-3")
} elseif ($pythonCommand) {
    $pythonExe = $pythonCommand.Source
}

if (-not $pythonExe) {
    $postFailures += "Python 3 not found"
} else {
    $env:PASS45_SOURCE_SHA = $head
    $evidenceArgs = $pythonPrefix + @($EvidenceVerify, $GameplayLog, $MaterialLog, $WeaponReport)
    $global:LASTEXITCODE = 0
    & $pythonExe @evidenceArgs 2>&1 | Tee-Object -FilePath (Join-Path $StageDir "runtime_evidence.log")
    $evidenceRc = $LASTEXITCODE
    if ($evidenceRc -ne 0) { $postFailures += "runtime evidence verifier failed code=$evidenceRc" }
    Remove-Item Env:PASS45_SOURCE_SHA -ErrorAction SilentlyContinue
}

if ($postFailures.Count -gt 0) {
    Add-ReportLine ""
    Add-ReportLine "POST_RUNTIME_RESULT=FAIL"
    foreach ($problem in $postFailures) { Add-ReportLine (" - " + $problem) }
    Write-Summary
    Write-Host "[STOP] Runtime завершився, але post-runtime evidence має $($postFailures.Count) проблем."
    Write-Host "ЄДИНИЙ ЗВІТ: $Report"
    exit 22
}

if (-not $pythonExe) { exit 22 }
$finalPreflightArgs = $pythonPrefix + @($Finalizer, "--preflight")
& $pythonExe @finalPreflightArgs
$finalPreflightRc = $LASTEXITCODE
if ($finalPreflightRc -ne 0) {
    Add-ReportLine "AUTOMATED_RUNTIME=PASS"
    Add-ReportLine ("FINALIZER_PREFLIGHT=PENDING code={0}" -f $finalPreflightRc)
    Write-Summary
    Write-Host "[FINALIZE PENDING] Automated runtime PASS, але visual/cleanup preflight ще не закрито."
    Write-Host "ЄДИНИЙ ЗВІТ: $Report"
    exit 0
}

Write-Host ""
Write-Host "Перед підтвердженням перевірте HMMWV/M2/BTR-4, зброю, world assets, skins та HUD/UI."
$answer = Read-Host "Ви реально оглянули assets і приймаєте їх візуальний стан? [Y/N]"
if ($answer -notmatch '^[Yy]$') {
    Add-ReportLine "AUTOMATED_RUNTIME=PASS"
    Add-ReportLine "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION"
    Write-Summary
    Write-Host "[FINALIZE PENDING] Visual acceptance не записано. ZIP не видалялись."
    exit 0
}

$acceptArgs = $pythonPrefix + @($Finalizer, "--accept-visual")
& $pythonExe @acceptArgs
$acceptRc = $LASTEXITCODE
if ($acceptRc -ne 0) {
    Add-ReportLine ("FINALIZER_ACCEPT=FAIL code={0}" -f $acceptRc)
    Write-Summary
    exit $acceptRc
}

Add-ReportLine "AUTOMATED_RUNTIME=PASS"
Add-ReportLine "VISUAL_ACCEPTANCE=PASS"
Add-ReportLine "SOURCE_ZIP_CLEANUP=PASS"
Write-Summary
Write-Host "===================================================================="
Write-Host "PASS45 FULL ASSET LIFECYCLE ACCEPTED."
Write-Host "ЄДИНИЙ ЗВІТ: $Report"
exit 0
