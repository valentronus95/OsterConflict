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
echo 2. ПОВНИЙ RUNTIME-ТЕСТ ^(ПАКЕТНИЙ^)
echo 3. SAFE СУМІСНІСТЬ ^(RHI THREAD OFF^)
echo 4. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Для запуску проєкту завжди використовуйте тільки START_HERE.cmd.
echo Інші RUN_*.cmd - внутрішні технічні скрипти, вручну їх запускати не потрібно.
echo.
echo Пункт 1: тільки incremental C++ build + запуск гри. Без strict reimport/fresh-load підготовки.
echo Пункт 2: один пакетний PASS45 preflight ^(усі assets/зброя/транспорт^), один runtime, один звіт усіх проблем.
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
    echo ============================================================
    pause
  )
  goto menu
)
if errorlevel 2 (
  call "%~dp0OsterConflict\PASS45_BATCH_RUNTIME.cmd"
  set "BATCH_RC=!ERRORLEVEL!"
  echo.
  echo ============================================================
  if "!BATCH_RC!"=="0" (
    echo [PASS] Повний пакетний runtime-тест завершено.
  ) else (
    echo [STOP] Пакетний runtime-тест завершився з проблемами. code=!BATCH_RC!
    echo Один зведений звіт: %~dp0Logs\PASS45_BATCH_RUNTIME_REPORT.txt
  )
  echo ============================================================
  pause
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
    echo ============================================================
    pause
  )
  goto menu
)

goto menu

:end
exit /b 0
