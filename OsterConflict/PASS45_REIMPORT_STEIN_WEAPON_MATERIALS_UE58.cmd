@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "SCRIPT=%PROJECT_DIR%Scripts\pass45_reimport_stein_weapon_materials.py"
set "FRESH_SCRIPT=%PROJECT_DIR%Scripts\verify_stein_weapon_materials_fresh_load.py"
set "SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_reimport_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_fresh_load_success.txt"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\Pass45SteinFreshLoad.log"
set "REQUIRED_REVISION=PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"
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

if not exist "%UPROJECT%" (
    echo ERROR: project not found: %UPROJECT%
    exit /b 3
)
if not exist "%SCRIPT%" (
    echo ERROR: Pass45 Stein R3 authoring script not found: %SCRIPT%
    exit /b 4
)
if not exist "%FRESH_SCRIPT%" (
    echo ERROR: Pass45 Stein R3 fresh-load verifier not found: %FRESH_SCRIPT%
    exit /b 5
)
if exist "%SENTINEL%" del /q "%SENTINEL%" >nul 2>nul
if exist "%FRESH_SENTINEL%" del /q "%FRESH_SENTINEL%" >nul 2>nul
if exist "%FRESH_LOG%" del /q "%FRESH_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 STEIN AUTHORED MATERIAL R3 - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Project: %UPROJECT%
echo Revision: %REQUIRED_REVISION%
echo.

"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout
set "IMPORT_RC=!ERRORLEVEL!"

rem UnrealEditor-Cmd can return a negative value such as -1. Never propagate that raw value to a parent
rem that uses IF ERRORLEVEL, because cmd.exe treats IF ERRORLEVEL 1 as a >=1 test and would miss -1.
if not "!IMPORT_RC!"=="0" (
    echo.
    echo ERROR: Stein R3 authored-material commandlet failed with code !IMPORT_RC!.
    exit /b 9
)

if not exist "%SENTINEL%" (
    echo.
    echo ERROR: Pass45 Stein R3 authoring sentinel is missing.
    exit /b 10
)

findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%SENTINEL%" >nul || goto :authoring_failed
findstr /L /C:"PASS45_STEIN_AUTHORED_GRAPH=PASS" "%SENTINEL%" >nul || goto :authoring_failed
findstr /L /C:"PASS45_STEIN_UE58_EXPLICIT_BINDING=READY" "%SENTINEL%" >nul || goto :authoring_failed
findstr /L /C:"STATUS=EDITOR_GRAPH_AUTHORED_FRESH_LOAD_PENDING" "%SENTINEL%" >nul || goto :authoring_failed

echo.
echo [VERIFY] Reopening every R3 Stein mesh/material in a fresh UE 5.8 process...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%FRESH_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "FRESH_RC=!ERRORLEVEL!"
if not "!FRESH_RC!"=="0" (
    echo ERROR: Stein R3 fresh-load verifier failed with code !FRESH_RC!.
    echo Log: %FRESH_LOG%
    exit /b 12
)
if not exist "%FRESH_SENTINEL%" (
    echo ERROR: Stein R3 fresh-load sentinel is missing.
    echo Log: %FRESH_LOG%
    exit /b 13
)

findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%FRESH_SENTINEL%" >nul || goto :fresh_failed
findstr /L /C:"PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS" "%FRESH_SENTINEL%" >nul || goto :fresh_failed
findstr /L /C:"PASS45_STEIN_FRESH_LOAD=READY" "%FRESH_SENTINEL%" >nul || goto :fresh_failed
findstr /L /C:"PASS45_STEIN_UE58_EXPLICIT_BINDING=READY" "%FRESH_SENTINEL%" >nul || goto :fresh_failed
findstr /L /C:"STATUS=FRESH_LOAD_VALIDATED_RUNTIME_VISUAL_PENDING" "%FRESH_SENTINEL%" >nul || goto :fresh_failed

echo.
echo PASS: Stein R3 authored material graph was saved and independently fresh-loaded with real texture dependencies.
echo STATUS: FRESH-LOAD VALIDATED; RUNTIME VISUAL ACCEPTANCE IS STILL PENDING.
echo Authoring sentinel: %SENTINEL%
echo Fresh-load sentinel: %FRESH_SENTINEL%
exit /b 0

:authoring_failed
echo.
echo ERROR: Stein R3 authoring sentinel is incomplete or stale.
type "%SENTINEL%"
exit /b 11

:fresh_failed
echo.
echo ERROR: Stein R3 fresh-load sentinel is incomplete or stale.
type "%FRESH_SENTINEL%"
echo Log: %FRESH_LOG%
exit /b 14
