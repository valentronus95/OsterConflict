@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%PROJECT_DIR%..\"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "SCRIPT=%REPO_ROOT%PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py"
set "SOURCE=%REPO_ROOT%SOURCE_ASSETS\PASS45\Remington870\remington_870_8siandude_ccby4.glb"
set "LOG=%PROJECT_DIR%Saved\Logs\Pass45Remington870UE58ImportedMotionPilot.log"
set "CANONICAL_BRANCH=fix/pass45-runtime-rejection-material-closure-20260826"
set "EXPECTED_SOURCE_SIZE=20621580"
set "EXPECTED_SOURCE_SHA256=147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
)

if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
)

if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%%B"
    )
)

if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "5.8 UE_5.8"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%%B"
    )
)

if not defined UE_ROOT (
    echo ERROR: Unreal Engine 5.8 installation was not found.
    exit /b 2
)

set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if not exist "%COMMANDLET_UPROJECT%" (
    echo ERROR: isolated Pass45 commandlet project not found: %COMMANDLET_UPROJECT%
    exit /b 3
)
if not exist "%SCRIPT%" (
    echo ERROR: Remington 870 imported-motion pilot script not found: %SCRIPT%
    exit /b 4
)
if not exist "%SOURCE%" (
    echo ERROR: pinned Remington 870 source donor not found: %SOURCE%
    exit /b 5
)

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git is required for current-head proof. No working-tree mutation was attempted.
    exit /b 6
)

set "LOCAL_BRANCH="
for /f "delims=" %%B in ('git -C "%REPO_ROOT%" rev-parse --abbrev-ref HEAD 2^>nul') do set "LOCAL_BRANCH=%%B"
if not defined LOCAL_BRANCH (
    echo ERROR: could not determine the local Git branch.
    exit /b 7
)
if /I not "%LOCAL_BRANCH%"=="%CANONICAL_BRANCH%" (
    echo ERROR: imported-motion pilot must run from canonical branch %CANONICAL_BRANCH%.
    echo Actual local branch: %LOCAL_BRANCH%
    exit /b 8
)

set "LOCAL_HEAD="
for /f "delims=" %%H in ('git -C "%REPO_ROOT%" rev-parse HEAD 2^>nul') do set "LOCAL_HEAD=%%H"
if not defined LOCAL_HEAD (
    echo ERROR: could not determine local HEAD.
    exit /b 9
)

set "REMOTE_HEAD="
for /f "tokens=1" %%H in ('git -C "%REPO_ROOT%" ls-remote origin "refs/heads/%CANONICAL_BRANCH%" 2^>nul') do set "REMOTE_HEAD=%%H"
if not defined REMOTE_HEAD (
    echo ERROR: could not read current remote canonical HEAD. Current-head acceptance is fail-closed.
    echo No checkout, pull, reset, clean, stash or LFS mutation was attempted.
    exit /b 10
)
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
    echo ERROR: local checkout is stale relative to remote canonical HEAD.
    echo Local:  %LOCAL_HEAD%
    echo Remote: %REMOTE_HEAD%
    echo Update the checkout without discarding local Changes before treating this pilot as current-head evidence.
    exit /b 11
)

set "PILOT_DIRTY="
for /f "delims=" %%S in ('git -C "%REPO_ROOT%" status --porcelain --untracked-files=no -- "PASS45_REMINGTON870_UE58_IMPORT_PILOT.py" "PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py" "OsterConflict/OsterConflictPass45Commandlet.uproject" "OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd" 2^>nul') do set "PILOT_DIRTY=1"
if defined PILOT_DIRTY (
    echo ERROR: tracked files that define the isolated imported-motion pilot differ from current HEAD.
    echo Local Changes were not modified. Current-head evidence is fail-closed until those pilot files match HEAD.
    exit /b 12
)

for %%F in ("%SOURCE%") do set "SOURCE_SIZE=%%~zF"
if not "%SOURCE_SIZE%"=="%EXPECTED_SOURCE_SIZE%" (
    echo ERROR: Remington 870 source size drifted. Expected %EXPECTED_SOURCE_SIZE% bytes, got %SOURCE_SIZE%.
    echo This also rejects a Git LFS pointer in place of the exact donor payload.
    exit /b 13
)

set "SOURCE_SHA256="
for /f "delims=" %%H in ('powershell -NoProfile -Command "$h=(Get-FileHash -Algorithm SHA256 -LiteralPath $env:SOURCE).Hash.ToLowerInvariant(); Write-Output $h" 2^>nul') do set "SOURCE_SHA256=%%H"
if not defined SOURCE_SHA256 (
    echo ERROR: could not compute SHA-256 for the Remington 870 donor.
    exit /b 14
)
if /I not "%SOURCE_SHA256%"=="%EXPECTED_SOURCE_SHA256%" (
    echo ERROR: Remington 870 donor SHA-256 drifted.
    echo Expected: %EXPECTED_SOURCE_SHA256%
    echo Actual:   %SOURCE_SHA256%
    exit /b 15
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 REMINGTON 870 UE 5.8 IMPORTED MOTION PILOT
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo Source:  %SOURCE%
echo Branch:  %LOCAL_BRANCH%
echo HEAD:    %LOCAL_HEAD%
echo SHA256:  %SOURCE_SHA256%
echo.
echo [PASS45] CURRENT-HEAD / EXACT-DONOR PREFLIGHT PASS. All Git checks above are read-only.
echo [PASS45] ISOLATED MOTION PROOF ONLY: reuse base import pilot; no package save, no production cutover, no runtime acceptance.
echo [PASS45] Pmag_061 pump identity and standalone pump-clip identity remain UNPROVEN until visual inspection.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo.
    echo ERROR: Remington 870 isolated UE 5.8 imported-motion pilot failed with code !RC!.
    echo Log: %LOG%
    exit /b 16
)

if not exist "%LOG%" (
    echo ERROR: Remington 870 imported-motion pilot log was not created.
    exit /b 17
)

findstr /L /C:"PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT_PASS" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"pbody_track_preserved=1 pbody_motion_preserved=1" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"pmag_track_preserved=1 pmag_motion_preserved=1" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"runtime_acceptance=0 item16_checked=0" "%LOG%" >nul || goto :pilot_failed

echo.
echo PASS: UE 5.8 preserved named Remington weapon-side tracks and non-trivial imported motion on current canonical HEAD with the exact pinned donor payload.
echo STATUS: IMPORTED-MOTION PROOF ONLY. Visual pump identity, standalone pump sequence, production cutover and runtime acceptance remain pending.
echo Log: %LOG%
exit /b 0

:pilot_failed
echo.
echo ERROR: UE commandlet returned success but required imported-motion evidence markers are incomplete.
echo Log: %LOG%
exit /b 18
