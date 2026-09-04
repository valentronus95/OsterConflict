@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_DIR=%~dp0"
rem %~dp0 ends in a backslash. Use a dot-qualified directory so PowerShell receives a clean path.
set "RECOVERY_PROJECT_DIR=%~dp0."
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
set "VERIFY_SCRIPT=%PROJECT_DIR%Scripts\verify_production_vehicle_fresh_load.py"
set "SOURCE_RECOVERY=%PROJECT_DIR%Scripts\prepare_local_production_sources.ps1"
set "WEAPON_SOURCE_RECOVERY=%PROJECT_DIR%Scripts\prepare_local_weapon_sources.ps1"
set "WEAPON_IMPORT_SCRIPT=%PROJECT_DIR%Scripts\import_local_production_weapon_assets.py"
set "WEAPON_VERIFY_SCRIPT=%PROJECT_DIR%Scripts\verify_local_production_weapon_fresh_load.py"
set "ALL_SOURCE_PREP=%PROJECT_DIR%Scripts\prepare_all_local_inbox_assets.ps1"
set "ALL_IMPORT_SCRIPT=%PROJECT_DIR%Scripts\import_all_local_inbox_assets.py"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_fresh_load_success.txt"
set "WEAPON_IMPORT_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_weapon_import_result.txt"
set "WEAPON_FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_weapon_fresh_load_result.txt"
set "ALL_BINDING_SENTINEL=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings_success.txt"
set "ALL_BINDING_MANIFEST=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings.json"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleImport.log"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\ProductionVehicleFreshLoad.log"
set "WEAPON_IMPORT_LOG=%PROJECT_DIR%Saved\Logs\ProductionWeaponImport.log"
set "WEAPON_FRESH_LOG=%PROJECT_DIR%Saved\Logs\ProductionWeaponFreshLoad.log"
set "ALL_IMPORT_LOG=%PROJECT_DIR%Saved\Logs\AllLocalInboxImport.log"
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
    echo ERROR: vehicle import script not found: %PY_SCRIPT%
    exit /b 4
)
if not exist "%VERIFY_SCRIPT%" (
    echo ERROR: vehicle fresh-load verification script not found: %VERIFY_SCRIPT%
    exit /b 5
)
if not exist "%WEAPON_SOURCE_RECOVERY%" (
    echo ERROR: local weapon source intake script not found: %WEAPON_SOURCE_RECOVERY%
    exit /b 8
)
if not exist "%WEAPON_IMPORT_SCRIPT%" (
    echo ERROR: local production weapon import script not found: %WEAPON_IMPORT_SCRIPT%
    exit /b 9
)
if not exist "%WEAPON_VERIFY_SCRIPT%" (
    echo ERROR: local production weapon fresh-load verifier not found: %WEAPON_VERIFY_SCRIPT%
    exit /b 10
)
if not exist "%ALL_SOURCE_PREP%" (
    echo ERROR: all-inbox source preparation script not found: %ALL_SOURCE_PREP%
    exit /b 11
)
if not exist "%ALL_IMPORT_SCRIPT%" (
    echo ERROR: all-inbox Unreal import/binding script not found: %ALL_IMPORT_SCRIPT%
    exit /b 12
)

rem First prepare EVERY model/UI pack in models_game_OC. UE-ready uasset packs keep their original /Game path;
rem raw FBX/GLB/OBJ and HUD images remain staged for the Unreal import pass below.
echo [ALL INBOX] Preparing every user-supplied ZIP/model/HUD source...
powershell -NoProfile -ExecutionPolicy Bypass -File "%ALL_SOURCE_PREP%" -ProjectDir "%RECOVERY_PROJECT_DIR%"
set "ALL_PREP_RC=!ERRORLEVEL!"
if not "!ALL_PREP_RC!"=="0" (
    echo ERROR: all-inbox preparation failed with code !ALL_PREP_RC!.
    exit /b !ALL_PREP_RC!
)

rem Vehicle source recovery is independent: usable sources import even when another exact vehicle remains a gap.
if exist "%SOURCE_RECOVERY%" (
    echo [SOURCE] Recovery project directory: %RECOVERY_PROJECT_DIR%
    powershell -NoProfile -ExecutionPolicy Bypass -File "%SOURCE_RECOVERY%" -ProjectDir "%RECOVERY_PROJECT_DIR%"
    set "SOURCE_RC=!ERRORLEVEL!"
    if not "!SOURCE_RC!"=="0" (
        echo [SOURCE] Vehicle recovery returned code !SOURCE_RC!. Continuing independent intake for available source files.
    )
)

rem Exact production M249 and Remington 870 are staged only from the explicit local models_game_OC inbox.
powershell -NoProfile -ExecutionPolicy Bypass -File "%WEAPON_SOURCE_RECOVERY%" -ProjectDir "%RECOVERY_PROJECT_DIR%"
set "WEAPON_SOURCE_RC=!ERRORLEVEL!"
if not "!WEAPON_SOURCE_RC!"=="0" (
    echo ERROR: local production weapon source staging failed with code !WEAPON_SOURCE_RC!.
    exit /b !WEAPON_SOURCE_RC!
)

for %%F in ("%SUCCESS_SENTINEL%" "%FRESH_SENTINEL%" "%WEAPON_IMPORT_SENTINEL%" "%WEAPON_FRESH_SENTINEL%" "%ALL_BINDING_SENTINEL%" "%IMPORT_LOG%" "%FRESH_LOG%" "%WEAPON_IMPORT_LOG%" "%WEAPON_FRESH_LOG%" "%ALL_IMPORT_LOG%") do (
    if exist "%%~F" del /q "%%~F" >nul 2>nul
)

echo ============================================================
echo OSTER CONFLICT - STRICT PRODUCTION MODEL INTAKE
echo ============================================================
echo UE:      %UE_CMD%
echo Project: %UPROJECT%
echo.

echo [ALL INBOX] Importing/binding all user models, humans, UE packs and HUD assets...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%ALL_IMPORT_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%ALL_IMPORT_LOG%"
set "ALL_IMPORT_RC=!ERRORLEVEL!"
if not "!ALL_IMPORT_RC!"=="0" (
    echo ERROR: all-inbox Unreal import process failed. code=!ALL_IMPORT_RC!
    echo Log: %ALL_IMPORT_LOG%
    exit /b !ALL_IMPORT_RC!
)
if not exist "%ALL_BINDING_SENTINEL%" (
    echo [ALL INBOX] CONTENT GAP: at least one supplied model/HUD could not be bound to runtime.
    if exist "%ALL_BINDING_MANIFEST%" type "%ALL_BINDING_MANIFEST%"
    echo Log: %ALL_IMPORT_LOG%
    exit /b 34
)

echo [VEHICLES] Importing HMMWV + M2 Browning + BTR-4 candidates...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%PY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "RESULT=!ERRORLEVEL!"
if not "!RESULT!"=="0" (
    echo ERROR: no usable production vehicle import completed. code=!RESULT!
    echo Log: %IMPORT_LOG%
    exit /b !RESULT!
)

if not exist "%SUCCESS_SENTINEL%" (
    echo ERROR: Unreal exited with code 0 but the vehicle production import result sentinel is missing.
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
    echo ERROR: importer produced no canonical production vehicle asset marker.
    type "%SUCCESS_SENTINEL%"
    exit /b 7
)

echo [VEHICLES] Reopening imported production assets in a fresh UE process...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%VERIFY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "VERIFY_RC=!ERRORLEVEL!"
if not "!VERIFY_RC!"=="0" (
    echo ERROR: fresh UE process could not validate imported production vehicles. code=!VERIFY_RC!
    echo Log: %FRESH_LOG%
    exit /b !VERIFY_RC!
)
if not exist "%FRESH_SENTINEL%" (
    echo ERROR: fresh-load production vehicle sentinel is missing.
    echo Log: %FRESH_LOG%
    exit /b 24
)

if "!HMMWV_IMPORTED!"=="1" findstr /L /C:"%HMMWV_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
if "!M2_IMPORTED!"=="1" findstr /L /C:"%M2_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
if "!BTR_IMPORTED!"=="1" findstr /L /C:"%BTR_ASSET%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh

echo [WEAPONS] Importing exact local M249 + Remington 870 candidates from models_game_OC...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%WEAPON_IMPORT_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%WEAPON_IMPORT_LOG%"
set "WEAPON_IMPORT_RC=!ERRORLEVEL!"
if not "!WEAPON_IMPORT_RC!"=="0" (
    echo ERROR: production weapon import process failed. code=!WEAPON_IMPORT_RC!
    echo Log: %WEAPON_IMPORT_LOG%
    exit /b !WEAPON_IMPORT_RC!
)
if not exist "%WEAPON_IMPORT_SENTINEL%" (
    echo ERROR: production weapon import result is missing.
    echo Log: %WEAPON_IMPORT_LOG%
    exit /b 26
)

"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%WEAPON_VERIFY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%WEAPON_FRESH_LOG%"
set "WEAPON_VERIFY_RC=!ERRORLEVEL!"
if not "!WEAPON_VERIFY_RC!"=="0" (
    echo ERROR: exact production weapon fresh-load process failed. code=!WEAPON_VERIFY_RC!
    echo Log: %WEAPON_FRESH_LOG%
    exit /b !WEAPON_VERIFY_RC!
)
if not exist "%WEAPON_FRESH_SENTINEL%" (
    echo ERROR: production weapon fresh-load result is missing.
    echo Log: %WEAPON_FRESH_LOG%
    exit /b 27
)

findstr /L /C:"STATUS=PASS" "%WEAPON_FRESH_SENTINEL%" >nul
if errorlevel 1 (
    echo [WEAPONS] CONTENT GAP: exact M249 and/or Remington 870 did not pass authored material+texture fresh-load validation.
    type "%WEAPON_FRESH_SENTINEL%"
    echo Log: %WEAPON_FRESH_LOG%
    exit /b 33
)

echo.
echo [ASSETS] ALL supplied inbox models/HUD are imported and assigned to a runtime binding.
echo [ASSETS] Vehicle result: HMMWV=!HMMWV_IMPORTED! M2=!M2_IMPORTED! BTR4=!BTR_IMPORTED!
if "!HMMWV_IMPORTED!"=="1" echo [ASSETS] HMMWV canonical production mesh imported and fresh-load verified.
if "!M2_IMPORTED!"=="1" echo [ASSETS] M2 Browning canonical production mesh imported and fresh-load verified.
if "!BTR_IMPORTED!"=="1" echo [ASSETS] BTR-4 canonical production mesh imported and fresh-load verified.
echo [ASSETS] Exact M249 + Remington 870 authored material/texture fresh-load validation PASS.

if "!HMMWV_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: HMMWV production source/import is still unavailable.
if "!M2_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: M2 Browning production source/import is still unavailable.
if "!BTR_IMPORTED!"=="0" echo [ASSETS] CONTENT GAP: BTR-4 production source/import is still unavailable.

rem Full runtime acceptance is fail-closed for every exact production vehicle required by Pass45.
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
