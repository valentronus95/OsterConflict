@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_DIR=%~dp0"
rem %~dp0 ends in a backslash. Use a dot-qualified directory so PowerShell receives a clean path.
set "RECOVERY_PROJECT_DIR=%~dp0."
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
set "VERIFY_SCRIPT=%PROJECT_DIR%Scripts\verify_production_vehicle_fresh_load.py"
set "SOURCE_RECOVERY=%PROJECT_DIR%Scripts\prepare_local_production_sources.ps1"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_fresh_load_success.txt"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleImport.log"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleFreshLoad.log"
set "UE_CMD="

set "HMMWV_ASSET=/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA"
set "M2_ASSET=/Game/Production/Weapons/M2/SM_M2_Browning"
set "BTR_ASSET=/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus"

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_CMD=%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)
if not defined UE_CMD if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_CMD=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)
if not defined UE_CMD (
    for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    )
)
if not defined UE_CMD (
    for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "UE_5.8"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    )
)

if not defined UE_CMD (
    echo ERROR: Unreal Engine 5.8 UnrealEditor-Cmd.exe was not found.
    exit /b 2
)
if not exist "%UPROJECT%" (
    echo ERROR: Project not found: %UPROJECT%
    exit /b 3
)
if not exist "%PY_SCRIPT%" (
    echo ERROR: Import script not found: %PY_SCRIPT%
    exit /b 4
)
if not exist "%VERIFY_SCRIPT%" (
    echo ERROR: fresh-load verification script not found: %VERIFY_SCRIPT%
    exit /b 5
)

rem Pass 44: recovery may report an unresolved BTR gap while HMMWV/M2 are already usable.
rem Do not abort before independently importing the sources that actually exist.
if exist "%SOURCE_RECOVERY%" (
    echo [SOURCE] Recovery project directory: %RECOVERY_PROJECT_DIR%
    powershell -NoProfile -ExecutionPolicy Bypass -File "%SOURCE_RECOVERY%" -ProjectDir "%RECOVERY_PROJECT_DIR%"
    set "SOURCE_RC=!ERRORLEVEL!"
    if not "!SOURCE_RC!"=="0" (
        echo [SOURCE] Recovery returned code !SOURCE_RC!. Continuing independent intake for any available source files.
    )
)

if exist "%SUCCESS_SENTINEL%" del /q "%SUCCESS_SENTINEL%" >nul 2>nul
if exist "%FRESH_SENTINEL%" del /q "%FRESH_SENTINEL%" >nul 2>nul
if exist "%IMPORT_LOG%" del /q "%IMPORT_LOG%" >nul 2>nul
if exist "%FRESH_LOG%" del /q "%FRESH_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - INDEPENDENT PRODUCTION VEHICLE IMPORT
echo ============================================================
echo UE:      %UE_CMD%
echo Project: %UPROJECT%
echo Script:  %PY_SCRIPT%
echo Log:     %IMPORT_LOG%
echo.

"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%PY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "RESULT=!ERRORLEVEL!"
if not "!RESULT!"=="0" (
    echo.
    echo ERROR: no usable production import completed. code=!RESULT!
    echo Log: %IMPORT_LOG%
    exit /b !RESULT!
)

if not exist "%SUCCESS_SENTINEL%" (
    echo.
    echo ERROR: Unreal exited with code 0 but the production import result sentinel is missing.
    echo Log: %IMPORT_LOG%
    exit /b 6
)

set "HMMWV_IMPORTED=0"
set "M2_IMPORTED=0"
set "BTR_IMPORTED=0"
findstr /L /C:"IMPORTED=%HMMWV_ASSET%" "%SUCCESS_SENTINEL%" >nul && set "HMMWV_IMPORTED=1"
findstr /L /C:"IMPORTED=%M2_ASSET%" "%SUCCESS_SENTINEL%" >nul && set "M2_IMPORTED=1"
findstr /L /C:"IMPORTED=%BTR_ASSET%" "%SUCCESS_SENTINEL%" >nul && set "BTR_IMPORTED=1"

if "!HMMWV_IMPORTED!"=="0" if "!M2_IMPORTED!"=="0" if "!BTR_IMPORTED!"=="0" (
    echo ERROR: importer produced no canonical production asset marker.
    type "%SUCCESS_SENTINEL%"
    exit /b 7
)

echo.
echo [VERIFY] Reopening imported production assets in a fresh UE process...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%VERIFY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "VERIFY_RC=!ERRORLEVEL!"
if not "!VERIFY_RC!"=="0" (
    echo ERROR: fresh UE process could not validate imported production models. code=!VERIFY_RC!
    echo Log: %FRESH_LOG%
    exit /b !VERIFY_RC!
)
if not exist "%FRESH_SENTINEL%" (
    echo ERROR: fresh-load production model sentinel is missing.
    echo Log: %FRESH_LOG%
    exit /b 24
)

if "!HMMWV_IMPORTED!"=="1" findstr /L /C:"%HMMWV_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
if "!M2_IMPORTED!"=="1" findstr /L /C:"%M2_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
if "!BTR_IMPORTED!"=="1" findstr /L /C:"%BTR_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh

echo.
echo [ASSETS] Import result: HMMWV=!HMMWV_IMPORTED! M2=!M2_IMPORTED! BTR4=!BTR_IMPORTED!
if "!HMMWV_IMPORTED!"=="1" echo [ASSETS] HMMWV canonical production mesh imported and fresh-load verified.
if "!M2_IMPORTED!"=="1" echo [ASSETS] M2 Browning canonical production mesh imported and fresh-load verified.
if "!BTR_IMPORTED!"=="1" echo [ASSETS] BTR-4 canonical production mesh imported and fresh-load verified.

if "!HMMWV_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: HMMWV production source/import is still unavailable.
if "!M2_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: M2 Browning production source/import is still unavailable.
if "!BTR_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: BTR-4 production source/import is still unavailable.

rem Return non-zero for strict acceptance if any requested final vehicle is missing. START_HERE's normal
rem optional intake catches this as a content gap and continues, but it can no longer print a false all-three success.
if "!HMMWV_IMPORTED!"=="0" exit /b 30
if "!M2_IMPORTED!"=="0" exit /b 31
if "!BTR_IMPORTED!"=="0" exit /b 32

exit /b 0

:bad_fresh
echo.
echo ERROR: a model reported as imported failed fresh-load verification.
echo File: %FRESH_SENTINEL%
echo Log:  %FRESH_LOG%
exit /b 25
