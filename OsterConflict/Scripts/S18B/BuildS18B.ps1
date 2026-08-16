param(
    [string]$UERoot = $env:UE_ROOT,
    [string]$Configuration = "Development",
    [switch]$SkipProjectFiles,
    [switch]$SkipAutomation,
    [switch]$SkipPackage
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

Write-Warning "Historical S18B helper. In the R6 launch kit, START_HERE.cmd / PC_TEST\RUN_UE58_PC_VALIDATION.ps1 is the authoritative first-build workflow."

$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = (Resolve-Path (Join-Path $ScriptRoot "..\..")).Path
$ProjectFile = Join-Path $ProjectRoot "OsterConflict.uproject"
$BuildRoot = Join-Path $ProjectRoot "Build\S18B"
$LogsRoot = Join-Path $BuildRoot "Logs"
$ReportsRoot = Join-Path $BuildRoot "AutomationReports"
$ClientArchive = Join-Path $BuildRoot "Client"
$ServerArchive = Join-Path $BuildRoot "Server"
$ReleaseMap = "/Game/Maps/OsterConflict_Runtime"

function Fail([string]$Message) { throw "[S18B] $Message" }
function Step([string]$Name, [scriptblock]$Body) {
    Write-Host ""
    Write-Host "========== $Name ==========" -ForegroundColor Cyan
    $global:LASTEXITCODE = 0
    & $Body
    $rc=$LASTEXITCODE
    if ($null -ne $rc -and $rc -ne 0) { Fail "$Name failed with exit code $rc" }
}

if ([string]::IsNullOrWhiteSpace($UERoot)) { Fail "Set UE_ROOT to the Unreal Engine 5.8 source-build root or pass -UERoot." }
$UERoot = (Resolve-Path $UERoot).Path
$RunUBT = Join-Path $UERoot "Engine\Build\BatchFiles\RunUBT.bat"
$RunUAT = Join-Path $UERoot "Engine\Build\BatchFiles\RunUAT.bat"
$EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$GenerateProjectFiles = Join-Path $UERoot "GenerateProjectFiles.bat"

foreach ($Required in @($RunUBT,$RunUAT,$EditorCmd)) {
    if (-not (Test-Path $Required)) { Fail "Required UE tool missing: $Required" }
}
if (-not (Test-Path $ProjectFile)) { Fail "Project file missing: $ProjectFile" }

New-Item -ItemType Directory -Force -Path $BuildRoot,$LogsRoot,$ReportsRoot,$ClientArchive,$ServerArchive | Out-Null

Step "0/7 Toolchain fingerprint" {
    $VersionInfo = (Get-Item $EditorCmd).VersionInfo
    $Lines = @(
        "UE_ROOT: $UERoot",
        "Project: $ProjectFile",
        "Configuration: $Configuration",
        "Release map: $ReleaseMap",
        "UnrealEditor-Cmd FileVersion: $($VersionInfo.FileVersion)",
        "UnrealEditor-Cmd ProductVersion: $($VersionInfo.ProductVersion)"
    )
    $Lines | Tee-Object -FilePath (Join-Path $LogsRoot "00_ToolchainFingerprint.log") | Write-Host
}

if (-not $SkipProjectFiles -and (Test-Path $GenerateProjectFiles)) {
    Step "1/7 Generate project files" {
        & $GenerateProjectFiles -project="$ProjectFile" -game -engine 2>&1 | Tee-Object -FilePath (Join-Path $LogsRoot "01_GenerateProjectFiles.log")
    }
} else {
    Write-Host "[S18B] GenerateProjectFiles skipped."
}

Step "2/7 Build Development Editor" {
    & $RunUBT OsterConflictEditor Win64 Development -Project="$ProjectFile" -WaitMutex -NoHotReloadFromIDE 2>&1 |
        Tee-Object -FilePath (Join-Path $LogsRoot "02_BuildEditor.log")
}

Step "3/7 Create project-owned release map" {
    $MapScript = Join-Path $ProjectRoot "Scripts\S18B\CREATE_RELEASE_MAP.py"
    & $EditorCmd "$ProjectFile" -run=pythonscript -script="$MapScript" -unattended -nop4 -NullRHI -NoSplash -UTF8Output 2>&1 |
        Tee-Object -FilePath (Join-Path $LogsRoot "03_CreateReleaseMap.log")
    $MapFile = Join-Path $ProjectRoot "Content\Maps\OsterConflict_Runtime.umap"
    if (-not (Test-Path $MapFile)) { Fail "Release map was not created: $MapFile" }
}

if (-not $SkipAutomation) {
    Step "4/7 Run OsterConflict.Release automation tests" {
        & $EditorCmd "$ProjectFile" -unattended -nop4 -NullRHI -NoSplash -UTF8Output `
            '-ExecCmds=Automation RunTest OsterConflict.Release;Quit' `
            -ReportExportPath="$ReportsRoot" 2>&1 | Tee-Object -FilePath (Join-Path $LogsRoot "04_Automation.log")
        $Index = Get-ChildItem -Path $ReportsRoot -Recurse -File -ErrorAction SilentlyContinue | Select-Object -First 1
        if (-not $Index) { Fail "Automation produced no report files in $ReportsRoot" }
    }
} else {
    Write-Host "[S18B] Automation tests skipped."
}

if (-not $SkipPackage) {
    Step "5/7 BuildCookRun Windows Client" {
        $Args = @(
            'BuildCookRun', "-project=$ProjectFile", '-noP4', '-platform=Win64', "-clientconfig=$Configuration",
            '-target=OsterConflictClient', '-build', '-cook', '-stage', '-pak', '-package', '-prereqs', '-archive',
            "-archivedirectory=$ClientArchive", "-map=$ReleaseMap", '-utf8output'
        )
        & $RunUAT @Args 2>&1 | Tee-Object -FilePath (Join-Path $LogsRoot "05_PackageClient.log")
    }

    Step "6/7 BuildCookRun Dedicated Server" {
        $Args = @(
            'BuildCookRun', "-project=$ProjectFile", '-noP4', '-server', '-noclient', '-serverplatform=Win64',
            "-serverconfig=$Configuration", '-target=OsterConflictServer', '-build', '-cook', '-stage', '-pak', '-package',
            '-archive', "-archivedirectory=$ServerArchive", "-map=$ReleaseMap", '-utf8output'
        )
        & $RunUAT @Args 2>&1 | Tee-Object -FilePath (Join-Path $LogsRoot "06_PackageServer.log")
    }

    Step "7/7 Post-build artifact audit" {
        $Audit = Join-Path $ProjectRoot "Scripts\S18B\POST_BUILD_AUDIT.py"
        python "$Audit" --client "$ClientArchive" --server "$ServerArchive" --out "$BuildRoot"
    }
}

Write-Host ""
Write-Host "S18B BUILD PIPELINE COMPLETE" -ForegroundColor Green
Write-Host "Build output: $BuildRoot"
Write-Host "Next: Scripts\S18B\SMOKE_LOCAL.ps1 -BuildRoot `"$BuildRoot`""
