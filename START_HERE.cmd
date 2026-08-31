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
echo Пункт 1: тільки incremental C++ build + запуск гри. Без reimport HMMWV/M2/BTR/Stein.
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
  set "OC_RHI_COMPAT="
  set "OC_QUICK_NORMAL="
  goto menu
)
if errorlevel 2 (
  call :prepare_materials_strict
  set "STRICT_PREP_RC=!ERRORLEVEL!"
  if not "!STRICT_PREP_RC!"=="0" goto menu
  set "OC_RHI_COMPAT=0"
  call "%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
  set "OC_RHI_COMPAT="
  goto menu
)
if errorlevel 1 (
  set "OC_QUICK_NORMAL=1"
  set "OC_RHI_COMPAT=0"
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  set "OC_RHI_COMPAT="
  set "OC_QUICK_NORMAL="
  goto menu
)

goto menu

:prepare_materials_strict
echo.
echo [PASS45] STRICT: підготовка Stein authored materials перед єдиним runtime acceptance route...
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
echo [PASS45] STRICT Stein material preparation PASS. Vehicle intake, playflow, dependencies and rendered runtime are validated downstream.
exit /b 0

:end
exit /b 0
