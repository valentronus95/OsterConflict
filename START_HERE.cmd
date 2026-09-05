@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul
rem Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.
cd /d "%~dp0"

set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "BATCH_RUNTIME=%~dp0RUN_PASS45_BATCH_RUNTIME_TEST.cmd"
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set "UPROJECT=%~dp0OsterConflict\OsterConflict.uproject"

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
echo Пункт 2: один пакетний PASS45 preflight, один runtime, один звіт усіх проблем.
echo.
echo Pass 45 normal renderer: DirectX 11 + Shader Model 5 + HDR off, normal RHI threading.
echo Compatibility route adds -norhithread only for A/B crash/performance diagnosis.
echo D3D12/SM6 тимчасово не використовується після підтверджених startup renderer crashes.
echo.
choice /C 12340 /N /M "Оберіть: "

if errorlevel 5 goto end
if errorlevel 4 (
  if not exist "%UE_EDITOR%" (
    echo [STOP] Unreal Editor 5.8 не знайдено: %UE_EDITOR%
    pause
    goto menu
  )
  if not exist "%UPROJECT%" (
    echo [STOP] Проєкт не знайдено: %UPROJECT%
    pause
    goto menu
  )
  start "" "%UE_EDITOR%" "%UPROJECT%" -d3d11 -sm5 -nohdr
  goto menu
)
if errorlevel 3 (
  if not exist "%CURRENT_GAMEPLAY%" (
    echo [STOP] Відсутній основний gameplay launcher: %CURRENT_GAMEPLAY%
    pause
    goto menu
  )
  set "OC_FORCE_ACCEPTANCE=0"
  set "OC_RHI_COMPAT=1"
  call "%CURRENT_GAMEPLAY%"
  set "OC_RHI_COMPAT="
  set "OC_FORCE_ACCEPTANCE="
  goto menu
)
if errorlevel 2 (
  if not exist "%BATCH_RUNTIME%" (
    echo [STOP] Відсутній пакетний runtime runner: %BATCH_RUNTIME%
    pause
    goto menu
  )
  call "%BATCH_RUNTIME%"
  set "BATCH_RC=!ERRORLEVEL!"
  if not "!BATCH_RC!"=="0" (
    echo.
    echo ============================================================
    echo [STOP] Пакетний runtime-тест завершився з проблемами. code=!BATCH_RC!
    echo Один зведений звіт: %~dp0Logs\PASS45_BATCH_RUNTIME_REPORT.txt
    echo ============================================================
    pause
  )
  goto menu
)
if errorlevel 1 (
  if not exist "%CURRENT_GAMEPLAY%" (
    echo [STOP] Відсутній основний gameplay launcher: %CURRENT_GAMEPLAY%
    pause
    goto menu
  )
  set "OC_FORCE_ACCEPTANCE=0"
  set "OC_RHI_COMPAT=0"
  call "%CURRENT_GAMEPLAY%"
  set "OC_RHI_COMPAT="
  set "OC_FORCE_ACCEPTANCE="
  goto menu
)

goto menu

:end
exit /b 0
