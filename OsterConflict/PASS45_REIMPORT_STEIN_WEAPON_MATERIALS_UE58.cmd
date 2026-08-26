@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "SCRIPT=%PROJECT_DIR%Scripts\pass45_reimport_stein_weapon_materials.py"
set "SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_reimport_success.txt"
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
    echo ERROR: Pass45 Stein reimport script not found: %SCRIPT%
    exit /b 4
)
if exist "%SENTINEL%" del /q "%SENTINEL%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 STEIN AUTHORED MATERIAL REIMPORT - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Project: %UPROJECT%
echo.

"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout
set "IMPORT_RC=%ERRORLEVEL%"

if not "%IMPORT_RC%"=="0" (
    echo.
    echo ERROR: Stein authored-material reimport failed with code %IMPORT_RC%.
    exit /b %IMPORT_RC%
)

if not exist "%SENTINEL%" (
    echo.
    echo ERROR: Pass45 Stein authored-dependency sentinel is missing.
    exit /b 10
)

findstr /L /C:"PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS" "%SENTINEL%" >nul || goto :sentinel_failed
findstr /L /C:"STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING" "%SENTINEL%" >nul || goto :sentinel_failed

echo.
echo PASS: Stein authored texture/material dependency import validated in UE editor commandlet.
echo STATUS: EDITOR IMPORT VALIDATED; RUNTIME VISUAL ACCEPTANCE IS STILL PENDING.
echo Sentinel: %SENTINEL%
exit /b 0

:sentinel_failed
echo.
echo ERROR: Stein material reimport sentinel is incomplete.
type "%SENTINEL%"
exit /b 11
