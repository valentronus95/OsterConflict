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
  rem Pass 42: normal game also attempts exact vehicle intake when the local source package exists.
  rem Missing local production source remains a content gap and does not block the normal frontend.
  if exist "%~dp0OsterConflict\TRY_PRODUCTION_VEHICLES_UE58.cmd" call "%~dp0OsterConflict\TRY_PRODUCTION_VEHICLES_UE58.cmd"
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  goto menu
)

:end
exit /b 0