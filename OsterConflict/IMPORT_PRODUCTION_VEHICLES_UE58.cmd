@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
set "VERIFY_SCRIPT=%PROJECT_DIR%Scripts\verify_production_vehicle_fresh_load.py"
set "SOURCE_RECOVERY=%PROJECT_DIR%Scripts\prepare_local_production_sources.ps1"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_fresh_load_success.txt"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleImport.log"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleFreshLoad.log"
set "UE_CMD="

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
    echo Expected a Launcher install such as C:\Program Files\Epic Games\UE_5.8.
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

rem Recover the user's previously downloaded real source files locally. BTR remains local-only.
if exist "%SOURCE_RECOVERY%" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%SOURCE_RECOVERY%" -ProjectDir "%PROJECT_DIR%"
    set "SOURCE_RC=%ERRORLEVEL%"
    if not "%SOURCE_RC%"=="0" (
        echo.
        echo ERROR: required local production model sources are unavailable.
        echo The importer did not start Unreal because the real HMMWV/M2/BTR source files are missing.
        exit /b %SOURCE_RC%
    )
)

if not exist "%PROJECT_DIR%SourceAssets\Production\Vehicles\HMMWV\ukrainian_hmmwv_mk_19.glb" (
    echo ERROR: missing HMMWV production source after local recovery.
    exit /b 21
)
if not exist "%PROJECT_DIR%SourceAssets\Production\Weapons\M2\m2_50cal_machinegun_cc0.glb" (
    echo ERROR: missing M2 Browning production source after local recovery.
    exit /b 22
)
if not exist "%PROJECT_DIR%SourceAssets\Production\Vehicles\BTR4\BTR4_Bucephalus.fbx" (
    echo ERROR: missing BTR-4 production source after local recovery.
    exit /b 23
)

if exist "%SUCCESS_SENTINEL%" del /q "%SUCCESS_SENTINEL%" >nul 2>nul
if exist "%FRESH_SENTINEL%" del /q "%FRESH_SENTINEL%" >nul 2>nul
if exist "%IMPORT_LOG%" del /q "%IMPORT_LOG%" >nul 2>nul
if exist "%FRESH_LOG%" del /q "%FRESH_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PRODUCTION VEHICLE IMPORT
echo ============================================================
echo UE:      %UE_CMD%
echo Project: %UPROJECT%
echo Script:  %PY_SCRIPT%
echo Log:     %IMPORT_LOG%
echo.

"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%PY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo ERROR: Unreal production asset import failed with code %RESULT%.
    echo Log: %IMPORT_LOG%
    exit /b %RESULT%
)

if not exist "%SUCCESS_SENTINEL%" (
    echo.
    echo ERROR: Unreal exited with code 0 but the production import success sentinel is missing.
    echo The Python commandlet did not complete all three canonical asset imports.
    echo Log: %IMPORT_LOG%
    exit /b 6
)

findstr /L /C:"/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Weapons/M2/SM_M2_Browning" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel

echo.
echo [VERIFY] Reopening production assets in a fresh UE process...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%VERIFY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "VERIFY_RC=%ERRORLEVEL%"
if not "%VERIFY_RC%"=="0" (
    echo ERROR: fresh UE process could not load the imported production models. code=%VERIFY_RC%
    echo Log: %FRESH_LOG%
    exit /b %VERIFY_RC%
)
if not exist "%FRESH_SENTINEL%" (
    echo ERROR: fresh-load production model sentinel is missing.
    echo The game will not start with proxy BTR/HMMWV/M2 visuals.
    echo Log: %FRESH_LOG%
    exit /b 24
)
findstr /L /C:"/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"/Game/Production/Weapons/M2/SM_M2_Browning" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" "%FRESH_SENTINEL%" >nul || goto :bad_fresh

echo.
echo PASS: HMMWV, M2 Browning and BTR-4 imported and load successfully in a fresh UE process.
echo Import log: %IMPORT_LOG%
echo Verify log: %FRESH_LOG%
exit /b 0

:bad_sentinel
echo.
echo ERROR: production import success sentinel is incomplete or invalid.
echo File: %SUCCESS_SENTINEL%
echo Log:  %IMPORT_LOG%
exit /b 7

:bad_fresh
echo.
echo ERROR: fresh-load production model sentinel is incomplete or invalid.
echo File: %FRESH_SENTINEL%
echo Log:  %FRESH_LOG%
exit /b 25
