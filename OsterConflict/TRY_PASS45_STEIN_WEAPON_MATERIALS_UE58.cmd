@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\SteinWeapons\pass45_stein_material_reimport_success.txt"
set "REIMPORT=%PROJECT_DIR%PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
set "REQUIRED_REVISION=PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1"

if exist "%SENTINEL%" (
  findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%SENTINEL%" >nul && ^
  findstr /L /C:"PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS" "%SENTINEL%" >nul && ^
  findstr /L /C:"STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING" "%SENTINEL%" >nul && (
    echo [WEAPONS] Stein authored material import already matches current Pass45 revision.
    exit /b 0
  )
)

if not exist "%REIMPORT%" (
  echo [WEAPONS] ERROR: Stein authored-material reimport command is missing: %REIMPORT%
  exit /b 2
)

echo [WEAPONS] Stein material cache is missing or stale. Running texture-first authored reimport...
call "%REIMPORT%"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo [WEAPONS] ERROR: Stein authored-material reimport failed with code %RC%.
  exit /b %RC%
)

echo [WEAPONS] PASS: current Stein authored material import revision is cached locally.
exit /b 0
