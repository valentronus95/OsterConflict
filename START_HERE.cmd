@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

:menu
cls
echo ============================================================
echo OSTER CONFLICT - ГОЛОВНИЙ ЗАПУСК
echo ============================================================
echo.
echo 1. ЗВИЧАЙНА ГРА
echo 2. ПОВНИЙ RUNTIME-ТЕСТ
echo 3. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Для запуску проєкту завжди використовуйте тільки START_HERE.cmd.
echo Інші RUN_*.cmd - внутрішні технічні скрипти, вручну їх запускати не потрібно.
echo.
echo Поточний safe renderer: DirectX 11 + Shader Model 5.
echo Safe-start flags: -d3d11 -sm5 -nohdr -norhithread.
echo D3D12/SM6 тимчасово не використовується після підтверджених startup renderer crashes.
echo.
choice /C 1230 /N /M "Оберіть: "

if errorlevel 4 goto end
if errorlevel 3 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject" -d3d11 -sm5 -nohdr -norhithread
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
  goto menu
)
if errorlevel 1 (
  rem Pass 42: if the real HMMWV / M2 / BTR source package is already on this PC, import it before
  rem normal gameplay too. Do not make a missing optional local source package block the normal frontend.
  set "PRODUCTION_HMMWV=%~dp0OsterConflict\Content\Production\Vehicles\HMMWV\SM_HMMWV_UA.uasset"
  set "PRODUCTION_M2=%~dp0OsterConflict\Content\Production\Weapons\M2\SM_M2_Browning.uasset"
  set "PRODUCTION_BTR=%~dp0OsterConflict\Content\Production\Vehicles\BTR4\SM_BTR4_Bucephalus.uasset"
  if not exist "%PRODUCTION_HMMWV%" goto try_production_intake
  if not exist "%PRODUCTION_M2%" goto try_production_intake
  if not exist "%PRODUCTION_BTR%" goto try_production_intake
  goto production_ready

  :try_production_intake
  if exist "%~dp0OsterConflict\IMPORT_PRODUCTION_VEHICLES_UE58.cmd" (
    echo.
    echo [ASSETS] Шукаю та підключаю локальні HMMWV + M2 Browning + BTR-4...
    call "%~dp0OsterConflict\IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
    if errorlevel 1 (
      echo [ASSETS] Точний production-пакет ще неповний. Звичайна гра продовжить запуск із доступними моделями.
    ) else (
      echo [ASSETS] Production HMMWV + M2 Browning + BTR-4 підключені.
    )
  )

  :production_ready
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  goto menu
)

:end
exit /b 0