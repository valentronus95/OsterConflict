@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_DIR=%~dp0"
set "HMMWV=%PROJECT_DIR%Content\Production\Vehicles\HMMWV\SM_HMMWV_UA.uasset"
set "M2=%PROJECT_DIR%Content\Production\Weapons\M2\SM_M2_Browning.uasset"
set "BTR=%PROJECT_DIR%Content\Production\Vehicles\BTR4\SM_BTR4_Bucephalus.uasset"
set "IMPORTER=%PROJECT_DIR%IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
set "IMPORT_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "FRESH_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_fresh_load_success.txt"
set "REQUIRED_REVISION=PASS45_BTR_GLTF_Y_UP_20260827_R3"

set "ASSETS_PRESENT=0"
if exist "%HMMWV%" if exist "%M2%" if exist "%BTR%" set "ASSETS_PRESENT=1"

set "IMPORT_REVISION_OK=0"
set "FRESH_REVISION_OK=0"
set "FRESH_HMMWV=0"
set "FRESH_M2=0"
set "FRESH_BTR=0"
set "FRESH_BTR_AUTHORED=0"
set "FRESH_BTR_AXIS=0"
set "FRESH_BTR_GLTF_UP=0"
set "FRESH_BTR_INTERNAL_UP=0"

if exist "%IMPORT_SENTINEL%" (
  findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%IMPORT_SENTINEL%" >nul && set "IMPORT_REVISION_OK=1"
)
if exist "%FRESH_SENTINEL%" (
  findstr /L /C:"IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%" "%FRESH_SENTINEL%" >nul && set "FRESH_REVISION_OK=1"
  findstr /L /C:"FRESH_LOADED=/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA" "%FRESH_SENTINEL%" >nul && set "FRESH_HMMWV=1"
  findstr /L /C:"FRESH_LOADED=/Game/Production/Weapons/M2/SM_M2_Browning" "%FRESH_SENTINEL%" >nul && set "FRESH_M2=1"
  findstr /L /C:"FRESH_LOADED=/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" "%FRESH_SENTINEL%" >nul && set "FRESH_BTR=1"
  findstr /L /C:"BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored" "%FRESH_SENTINEL%" >nul && set "FRESH_BTR_AUTHORED=1"
  findstr /L /C:"BTR4_FORWARD_AXIS=+X" "%FRESH_SENTINEL%" >nul && set "FRESH_BTR_AXIS=1"
  findstr /L /C:"BTR4_GLTF_UP_AXIS=+Y" "%FRESH_SENTINEL%" >nul && set "FRESH_BTR_GLTF_UP=1"
  findstr /L /C:"BTR4_INTERNAL_UP_AXIS=+Z" "%FRESH_SENTINEL%" >nul && set "FRESH_BTR_INTERNAL_UP=1"
)

if "%ASSETS_PRESENT%"=="1" if "%IMPORT_REVISION_OK%"=="1" if "%FRESH_REVISION_OK%"=="1" if "%FRESH_HMMWV%"=="1" if "%FRESH_M2%"=="1" if "%FRESH_BTR%"=="1" if "%FRESH_BTR_AUTHORED%"=="1" if "%FRESH_BTR_AXIS%"=="1" if "%FRESH_BTR_GLTF_UP%"=="1" if "%FRESH_BTR_INTERNAL_UP%"=="1" (
  echo [ASSETS] Production HMMWV + M2 + BTR-4 match current Pass45 R3 import revision, authored-material and BTR +X/+Y-up fresh-load checks.
  exit /b 0
)

if not exist "%IMPORTER%" (
  echo [ASSETS] Optional production vehicle importer is missing. Continuing normal game with explicit content gap.
  exit /b 0
)

echo [ASSETS] Existing .uasset files are not sufficient proof of current materials/orientation.
echo [ASSETS] Reimport required: revision=%REQUIRED_REVISION% assets=%ASSETS_PRESENT% import_revision=%IMPORT_REVISION_OK% fresh_revision=%FRESH_REVISION_OK% fresh_hmmwv=%FRESH_HMMWV% fresh_m2=%FRESH_M2% fresh_btr=%FRESH_BTR% btr_material=%FRESH_BTR_AUTHORED% btr_axis=%FRESH_BTR_AXIS% btr_gltf_up=%FRESH_BTR_GLTF_UP% btr_internal_up=%FRESH_BTR_INTERNAL_UP%
echo [ASSETS] Importing HMMWV + M2 + BTR-4 through the current authored-material/axis contract...
call "%IMPORTER%"
set "IMPORT_RC=%ERRORLEVEL%"

set "HMMWV_READY=0"
set "M2_READY=0"
set "BTR_READY=0"
if exist "%HMMWV%" set "HMMWV_READY=1"
if exist "%M2%" set "M2_READY=1"
if exist "%BTR%" set "BTR_READY=1"

echo [ASSETS] Local canonical result: HMMWV=%HMMWV_READY% M2=%M2_READY% BTR4=%BTR_READY% importer_rc=%IMPORT_RC%

if "%HMMWV_READY%"=="1" echo [ASSETS] HMMWV production model is available for runtime.
if "%M2_READY%"=="1" echo [ASSETS] M2 Browning production model is available for runtime.
if "%BTR_READY%"=="1" echo [ASSETS] BTR-4 production model is available for runtime.

if "%HMMWV_READY%"=="0" echo [ASSETS] CONTENT GAP: HMMWV production model is still unavailable.
if "%M2_READY%"=="0" echo [ASSETS] CONTENT GAP: M2 Browning production model is still unavailable.
if "%BTR_READY%"=="0" echo [ASSETS] ERROR: repository-safe BTR-4 fallback should be importable even without a local FBX.

if "%IMPORT_RC%"=="0" if "%HMMWV_READY%"=="1" if "%M2_READY%"=="1" if "%BTR_READY%"=="1" (
  echo [ASSETS] PASS: all three production models were imported/fresh-load validated under the current Pass45 R3 material/axis revision.
  exit /b 0
)

echo [ASSETS] Production intake is incomplete. Normal game may continue for diagnosis, but missing/failed models are NOT production-ready.
exit /b 0
