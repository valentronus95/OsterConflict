@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%PROJECT_DIR%..\"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "SCRIPT=%REPO_ROOT%PASS45_REMINGTON870_UE58_IMPORT_PILOT.py"
set "SOURCE=%REPO_ROOT%SOURCE_ASSETS\PASS45\Remington870\remington_870_8siandude_ccby4.glb"
set "LOG=%PROJECT_DIR%Saved\Logs\Pass45Remington870UE58ImportPilot.log"
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
    echo ERROR: Remington 870 UE 5.8 import pilot script not found: %SCRIPT%
    exit /b 4
)
if not exist "%SOURCE%" (
    echo ERROR: pinned Remington 870 source donor not found: %SOURCE%
    exit /b 5
)

for %%F in ("%SOURCE%") do set "SOURCE_SIZE=%%~zF"
if "%SOURCE_SIZE%"=="0" (
    echo ERROR: pinned Remington 870 source donor is empty.
    exit /b 6
)
if %SOURCE_SIZE% LSS 1000000 (
    echo ERROR: Remington 870 source looks like a Git LFS pointer, not the 20 MB donor payload.
    echo Run the repository's normal Git LFS fetch/pull path before this pilot. No automatic working-tree mutation is performed here.
    exit /b 7
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 REMINGTON 870 UE 5.8 IMPORT PILOT
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo Source:  %SOURCE%
echo.
echo [PASS45] ISOLATED IMPORT ONLY: no package save, no production cutover, no runtime acceptance.
echo [PASS45] The commandlet host avoids loading OsterConflict runtime/world-sector startup owners.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo.
    echo ERROR: Remington 870 isolated UE 5.8 import pilot failed with code !RC!.
    echo Log: %LOG%
    exit /b 8
)

if not exist "%LOG%" (
    echo ERROR: Remington 870 pilot log was not created.
    exit /b 9
)

findstr /L /C:"PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"donor_action_channels=71/71/72" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"imported_animation_set_preserved=1" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"production_cutover=0 runtime_acceptance=0 item16_checked=0" "%LOG%" >nul || goto :pilot_failed

echo.
echo PASS: pinned Remington 870 donor preserved its animated action set through isolated UE 5.8 import.
echo STATUS: IMPORT PILOT ONLY. Production package, first-person pump mapping, fresh-load and runtime acceptance remain pending.
echo Log: %LOG%
exit /b 0

:pilot_failed
echo.
echo ERROR: UE commandlet returned success but the required Remington pilot evidence markers are incomplete.
echo Log: %LOG%
exit /b 10
