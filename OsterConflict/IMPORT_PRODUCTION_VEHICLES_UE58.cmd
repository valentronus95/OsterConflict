@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
set "SOURCE_RECOVERY=%PROJECT_DIR%Scripts\prepare_local_production_sources.ps1"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleImport.log"
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

rem The public repo intentionally does not carry every production source byte. BTR-4 is local-only for
rem license reasons, and the HMMWV/M2 LFS sources were not present in current main. Recover the user's
rem previously downloaded model files/ZIP locally before invoking Unreal instead of producing a vague
rem "exit code 0 but sentinel missing" failure.
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
if exist "%IMPORT_LOG%" del /q "%IMPORT_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PRODUCTION VEHICLE IMPORT
echo ============================================================
echo UE:      %UE_CMD%
echo Project: %UPROJECT%
echo Script:  %PY_SCRIPT%
echo Log:     %IMPORT_LOG%
echo.

rem UE 5.8 documents two Python launch routes. The old full-editor -ExecutePythonScript route also
rem requires Editor Scripting Utilities and in the failing playtest returned 0 after platform validation
rem without ever executing our Python script. Asset import does not need a loaded level, so use the
rem PythonScript commandlet directly. This route requires only PythonScriptPlugin and is deterministic headless CI/runtime tooling.
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%PY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo ERROR: Unreal production asset import failed with code %RESULT%.
    echo Log: %IMPORT_LOG%
    exit /b %RESULT%
)

rem Do not trust the process exit code alone. Python writes this file only after all three import
rem tasks report the canonical assets as created/updated and the assets are saved successfully.
if not exist "%SUCCESS_SENTINEL%" (
    echo.
    echo ERROR: Unreal exited with code 0 but the production import success sentinel is missing.
    echo The Python commandlet did not complete all three canonical asset imports.
    echo Log: %IMPORT_LOG%
    exit /b 5
)

findstr /L /C:"/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Weapons/M2/SM_M2_Browning" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel

echo.
echo PASS: HMMWV, M2 Browning and BTR-4 production assets imported, verified and saved.
echo Log: %IMPORT_LOG%
exit /b 0

:bad_sentinel
echo.
echo ERROR: production import success sentinel is incomplete or invalid.
echo File: %SUCCESS_SENTINEL%
echo Log:  %IMPORT_LOG%
exit /b 6
