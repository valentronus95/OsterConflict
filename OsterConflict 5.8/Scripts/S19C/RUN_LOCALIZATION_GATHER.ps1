param(
    [Parameter(Mandatory=$true)][string]$UE_ROOT
)
$ErrorActionPreference = 'Stop'
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot '../..')
$Project = Join-Path $ProjectRoot 'OsterConflict.uproject'
$EditorCmd = Join-Path $UE_ROOT 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$Config = Join-Path $ProjectRoot 'Config/Localization/Game_Gather.ini'
if (!(Test-Path $EditorCmd)) { throw "UnrealEditor-Cmd.exe not found: $EditorCmd" }
if (!(Test-Path $Project)) { throw "Project not found: $Project" }
if (!(Test-Path $Config)) { throw "Localization config not found: $Config" }
& $EditorCmd $Project -run=GatherText "-config=$Config" -unattended -nop4 -UTF8Output
if ($LASTEXITCODE -ne 0) { throw "GatherText failed with exit code $LASTEXITCODE" }
$LocRoot=Join-Path $ProjectRoot 'Content/Localization/Game'
$NativeLocRes=Join-Path $LocRoot 'uk-UA/Game.locres'
if(!(Test-Path $NativeLocRes)){ throw "GatherText returned success but native LocRes was not generated: $NativeLocRes" }
$EnglishLocRes=Join-Path $LocRoot 'en/Game.locres'
if(!(Test-Path $EnglishLocRes)){ Write-Warning "English LocRes not generated yet: $EnglishLocRes (translation/import coverage remains pending)." }
Write-Host 'S19C localization gather/resource generation finished. Native uk-UA LocRes exists; English translation coverage still requires catalog review.'
