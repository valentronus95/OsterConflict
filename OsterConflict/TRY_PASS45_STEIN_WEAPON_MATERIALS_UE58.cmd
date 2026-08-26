@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_reimport_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_fresh_load_success.txt"
set "REIMPORT=%PROJECT_DIR%PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
set "REQUIRED_REVISION=PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"

if exist "%SENTINEL%" if exist "%FRESH_SENTINEL%" (
  findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%SENTINEL%" >nul && ^
  findstr /L /C:"PASS45_STEIN_AUTHORED_GRAPH=PASS" "%SENTINEL%" >nul && ^
  findstr /L /C:"PASS45_STEIN_UE58_EXPLICIT_BINDING=READY" "%SENTINEL%" >nul && ^
  findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%FRESH_SENTINEL%" >nul && ^
  findstr /L /C:"PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS" "%FRESH_SENTINEL%" >nul && ^
  findstr /L /C:"PASS45_STEIN_FRESH_LOAD=READY" "%FRESH_SENTINEL%" >nul && ^
  findstr /L /C:"STATUS=FRESH_LOAD_VALIDATED_RUNTIME_VISUAL_PENDING" "%FRESH_SENTINEL%" >nul && (
    echo [WEAPONS] Stein authored material cache already matches current Pass45 R3 fresh-load revision.
    exit /b 0
  )
)

if not exist "%REIMPORT%" (
  echo [WEAPONS] ERROR: Stein authored-material reimport command is missing: %REIMPORT%
  exit /b 2
)

echo [WEAPONS] Stein material cache is missing or stale. Running R3 explicit authoring + independent fresh-load verification...
call "%REIMPORT%"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo [WEAPONS] ERROR: Stein R3 authored-material validation failed with code %RC%.
  exit /b 3
)

echo [WEAPONS] PASS: current Stein R3 authored material revision is cached and fresh-load validated locally.
exit /b 0
