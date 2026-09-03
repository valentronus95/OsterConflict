@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%PROJECT_DIR%..\"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "BASE_SCRIPT=%REPO_ROOT%PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.py"
set "SCRIPT=%REPO_ROOT%PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_COMPAT.py"
set "SOURCE=%PROJECT_DIR%Content\Raw\R13\Weapons\SteinClassicWeapons\WeaponsPack\LeverAction\SKM_LeverAction.fbx"
set "LOG=%PROJECT_DIR%Saved\Logs\Pass45LeverActionDerivedLeverUE58Pilot.log"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
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
if not exist "%BASE_SCRIPT%" (
    echo ERROR: canonical Lever Action UE 5.8 pilot script not found: %BASE_SCRIPT%
    exit /b 4
)
if not exist "%SCRIPT%" (
    echo ERROR: Lever Action UE 5.8 frame-rate compatibility shim not found: %SCRIPT%
    exit /b 10
)
if not exist "%SOURCE%" (
    echo ERROR: exact Stein Lever Action donor not found: %SOURCE%
    exit /b 5
)
for %%F in ("%SOURCE%") do set "SOURCE_SIZE=%%~zF"
if %SOURCE_SIZE% LSS 100000 (
    echo ERROR: Stein Lever Action donor looks like a Git LFS pointer rather than the exact payload.
    echo Use the repository's normal Git LFS path first. No automatic working-tree mutation is performed here.
    exit /b 6
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 LEVER ACTION DERIVED LEVER UE 5.8 PILOT
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo Source:  %SOURCE%
echo.
echo [PASS45] ISOLATED LEVER MOVING-PART PROOF ONLY.
echo [PASS45] UE 5.8 transient AnimSequence cadence is adapted from legacy 20 fps to compatible 60 fps only.
echo [PASS45] Pilot angle is calibration-only, not a source-authored endpoint.
echo [PASS45] No package save, no production profile cutover, no item-16 acceptance.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo.
    echo ERROR: Lever Action UE 5.8 pilot failed with code !RC!.
    echo Log: %LOG%
    exit /b 7
)

if not exist "%LOG%" (
    echo ERROR: Lever Action pilot log was not created.
    exit /b 8
)

findstr /L /C:"PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"source_authored_endpoint=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"pilot_angle_accepted=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"saved_packages=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"production_profile_changed=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"production_cutover=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"runtime_visual_acceptance=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"runtime_acceptance=0" "%LOG%" >nul || goto :pilot_failed
findstr /L /C:"item16_checked=0" "%LOG%" >nul || goto :pilot_failed

echo.
echo PASS: UE 5.8 preserved the isolated Lever Action LEVER bone and non-trivial pilot motion.
echo STATUS: MOVING-PART PROOF ONLY. Pilot-angle calibration, production cutover and runtime acceptance remain pending.
echo Log: %LOG%
exit /b 0

:pilot_failed
echo.
echo ERROR: UE commandlet returned success but required Lever Action proof markers are incomplete.
echo Log: %LOG%
exit /b 9
