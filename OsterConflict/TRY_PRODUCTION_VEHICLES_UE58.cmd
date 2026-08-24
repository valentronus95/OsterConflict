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
  echo [ASSETS] Optional production vehicle importer is missing. Continuing normal game.
  exit /b 0
)

echo [ASSETS] Шукаю локальні HMMWV + M2 Browning + BTR-4 та підключаю їх, якщо пакет доступний...
call "%IMPORTER%"
if errorlevel 1 (
  echo [ASSETS] Точний production-пакет ще неповний. Це content gap; normal game не блокується.
  exit /b 0
)

echo [ASSETS] Production HMMWV + M2 Browning + BTR-4 імпортовані та fresh-load перевірені.
exit /b 0