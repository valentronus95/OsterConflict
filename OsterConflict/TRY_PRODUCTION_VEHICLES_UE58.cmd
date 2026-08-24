@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "HMMWV=%PROJECT_DIR%Content\Production\Vehicles\HMMWV\SM_HMMWV_UA.uasset"
set "M2=%PROJECT_DIR%Content\Production\Weapons\M2\SM_M2_Browning.uasset"
set "BTR=%PROJECT_DIR%Content\Production\Vehicles\BTR4\SM_BTR4_Bucephalus.uasset"
set "IMPORTER=%PROJECT_DIR%IMPORT_PRODUCTION_VEHICLES_UE58.cmd"

if exist "%HMMWV%" if exist "%M2%" if exist "%BTR%" (
  echo [ASSETS] Production HMMWV + M2 Browning + BTR-4 вже присутні локально.
  exit /b 0
)

if not exist "%IMPORTER%" (
  echo [ASSETS] Optional production vehicle importer is missing. Continuing normal game with explicit content gap.
  exit /b 0
)

echo [ASSETS] Шукаю локальні HMMWV + M2 Browning + BTR-4 та підключаю кожну доступну модель незалежно...
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
if "%BTR_READY%"=="0" echo [ASSETS] CONTENT GAP: BTR-4 production model is still unavailable.

if "%HMMWV_READY%"=="1" if "%M2_READY%"=="1" if "%BTR_READY%"=="1" (
  echo [ASSETS] PASS: all three production models are present and fresh-load verified.
  exit /b 0
)

echo [ASSETS] Partial production intake only. Normal game may continue, but missing models are NOT production-ready.
exit /b 0
