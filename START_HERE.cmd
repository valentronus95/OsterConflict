@echo off
setlocal EnableExtensions EnableDelayedExpansion
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
echo 3. SAFE СУМІСНІСТЬ ^(RHI THREAD OFF^)
echo 4. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Для запуску проєкту завжди використовуйте тільки START_HERE.cmd.
echo Інші RUN_*.cmd - внутрішні технічні скрипти, вручну їх запускати не потрібно.
echo.
echo Пункт 1: тільки incremental C++ build + запуск гри. Без strict reimport/fresh-load підготовки.
echo Пункт 2: повний PASS45 runtime acceptance з усіма strict import/verification gates.
echo.
echo Pass 45 normal renderer: DirectX 11 + Shader Model 5 + HDR off, normal RHI threading.
echo Compatibility route adds -norhithread only for A/B crash/performance diagnosis.
echo D3D12/SM6 тимчасово не використовується після підтверджених startup renderer crashes.
echo.
choice /C 12340 /N /M "Оберіть: "

if errorlevel 5 goto end
if errorlevel 4 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject" -d3d11 -sm5 -nohdr
  goto menu
)
if errorlevel 3 (
  set "OC_QUICK_NORMAL=1"
  set "OC_RHI_COMPAT=1"
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  set "QUICK_RC=!ERRORLEVEL!"
  set "OC_RHI_COMPAT="
  set "OC_QUICK_NORMAL="
  if not "!QUICK_RC!"=="0" (
    echo.
    echo ============================================================
    echo [STOP] SAFE запуск завершився помилкою. code=!QUICK_RC!
    echo Лог: %~dp0Logs\R14_CURRENT_GAMEPLAY.log
    echo Вікно залишено відкритим спеціально, щоб помилка не зникала.
    echo ============================================================
    pause
  )
  goto menu
)
if errorlevel 2 (
  call :prepare_materials_strict
  set "STRICT_PREP_RC=!ERRORLEVEL!"
  if not "!STRICT_PREP_RC!"=="0" (
    echo.
    echo [STOP] Повний runtime-тест зупинено. code=!STRICT_PREP_RC!
    pause
    goto menu
  )
  set "OC_RHI_COMPAT=0"
  call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
  set "RUNTIME_RC=!ERRORLEVEL!"
  set "OC_RHI_COMPAT="
  if not "!RUNTIME_RC!"=="0" (
    echo.
    echo ============================================================
    echo [STOP] Повний runtime-тест завершився помилкою. code=!RUNTIME_RC!
    echo Вікно залишено відкритим спеціально, щоб результат не зникав.
    echo ============================================================
    pause
  )
  goto menu
)
if errorlevel 1 (
  set "OC_QUICK_NORMAL=1"
  set "OC_RHI_COMPAT=0"
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  set "QUICK_RC=!ERRORLEVEL!"
  set "OC_RHI_COMPAT="
  set "OC_QUICK_NORMAL="
  if not "!QUICK_RC!"=="0" (
    echo.
    echo ============================================================
    echo [STOP] Звичайний запуск завершився помилкою. code=!QUICK_RC!
    echo Лог: %~dp0Logs\R14_CURRENT_GAMEPLAY.log
    echo Вікно залишено відкритим спеціально, щоб помилка не зникала.
    echo ============================================================
    pause
  )
  goto menu
)

goto menu

:prepare_materials_strict
echo.
echo [PASS45] STRICT: Stein materials + manual-action audio + Remington 870 skeletal pump production intake...
rem Production HMMWV/M2/BTR import is intentionally NOT duplicated here. RUN_R14_CURRENT_GAMEPLAY.cmd,
rem called once through the full acceptance wrapper, owns the strict production vehicle intake.
if not exist "%~dp0OsterConflict\PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd" (
  echo [STOP] Stein authored-material importer відсутній.
  exit /b 22
)
call "%~dp0OsterConflict\PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
set "STEIN_STRICT_RC=!ERRORLEVEL!"
if not "!STEIN_STRICT_RC!"=="0" (
  echo [STOP] Stein authored-material import/fresh-load не пройшов. code=!STEIN_STRICT_RC!
  exit /b 23
)

if not exist "%~dp0OsterConflict\PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd" (
  echo [STOP] Manual-action audio importer/fresh-load wrapper відсутній.
  exit /b 24
)
call "%~dp0OsterConflict\PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd"
set "MANUAL_AUDIO_RC=!ERRORLEVEL!"
if not "!MANUAL_AUDIO_RC!"=="0" (
  echo [STOP] Manual-action Bolt/Lever SoundWave import/fresh-load не пройшов. code=!MANUAL_AUDIO_RC!
  exit /b 25
)

if not exist "%~dp0OsterConflict\PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd" (
  echo [STOP] Remington 870 production pump importer/fresh-load wrapper відсутній.
  exit /b 26
)
call "%~dp0OsterConflict\PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd"
set "REMINGTON_STRICT_RC=!ERRORLEVEL!"
if not "!REMINGTON_STRICT_RC!"=="0" (
  echo [STOP] Remington 870 skeletal pump production intake не пройшов. code=!REMINGTON_STRICT_RC!
  exit /b 27
)

echo [PASS45] STRICT content preparation PASS. Stein materials, manual-action audio and Remington 870 skeletal PumpCycle are fresh-load validated; vehicle intake, gameplay presentation, audibility/timing and rendered runtime are validated downstream.
exit /b 0

:end
exit /b 0
