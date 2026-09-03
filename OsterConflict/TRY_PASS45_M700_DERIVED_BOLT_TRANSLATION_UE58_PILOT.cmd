@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
for %%I in ("%PROJECT_DIR%..") do set "REPO_ROOT=%%~fI"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "SCRIPT=%REPO_ROOT%\PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.py"
set "SOURCE=%PROJECT_DIR%Content\Raw\R13\Weapons\SteinClassicWeapons\WeaponsPack\M700\SKM_M700.fbx"
set "LOG=%PROJECT_DIR%Saved\Logs\Pass45M700DerivedBoltTranslationUE58Pilot.log"
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
    exit /b 31
)

set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not exist "%COMMANDLET_UPROJECT%" (
    echo ERROR: isolated PASS45 commandlet project not found: %COMMANDLET_UPROJECT%
    exit /b 32
)
if not exist "%SCRIPT%" (
    echo ERROR: M700 UE58 pilot script not found: %SCRIPT%
    exit /b 33
)
if not exist "%SOURCE%" (
    echo ERROR: pinned M700 source FBX not found: %SOURCE%
    exit /b 34
)

for %%I in ("%SOURCE%") do set "SOURCE_SIZE=%%~zI"
if not "!SOURCE_SIZE!"=="638732" (
    echo ERROR: M700 source is not the hydrated pinned payload. Expected 638732 bytes, got !SOURCE_SIZE!.
    echo No automatic Git LFS or working-tree mutation is performed here.
    exit /b 35
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 M700 BOLT TRANSLATION UE 5.8 PILOT
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo Source:  %SOURCE%
echo.
echo [PASS45] This is an isolated unsaved calibration proof only.
echo [PASS45] BOLT_STOP is not used as a travel endpoint; bolt rotation remains pending.
echo [PASS45] No automatic working-tree mutation is performed here.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo ERROR: M700 UE58 bolt translation pilot failed with code !RC!.
    echo Log: %LOG%
    exit /b 36
)
if not exist "%LOG%" (
    echo ERROR: M700 pilot log was not created.
    exit /b 37
)

findstr /L /C:"PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_PASS" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"source_authored_endpoint=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"bolt_stop_used_as_endpoint=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"pilot_travel_accepted=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"rotation_calibration_pending=1" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"saved_packages=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"production_profile_changed=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"production_cutover=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"runtime_visual_acceptance=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"runtime_acceptance=0" "%LOG%" >nul || goto :proof_failed
findstr /L /C:"item16_checked=0" "%LOG%" >nul || goto :proof_failed

echo.
echo PASS: UE 5.8 preserved the M700 BOLT joint and non-trivial bounded translation pilot.
echo STATUS: TRANSLATION MOTION PROOF ONLY. Travel calibration, bolt rotation, production cutover and runtime acceptance remain pending.
echo Log: %LOG%
exit /b 0

:proof_failed
echo.
echo ERROR: UE commandlet returned success but required M700 proof markers are incomplete.
echo Log: %LOG%
exit /b 38
