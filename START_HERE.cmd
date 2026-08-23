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
echo Поточний safe renderer: DirectX 11.
echo D3D12 тимчасово вимкнений через підтверджений startup crash у D3D12RHI.
echo.
choice /C 1230 /N /M "Оберіть: "

if errorlevel 4 goto end
if errorlevel 3 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject" -d3d11
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"
  goto menu
)
if errorlevel 1 (
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  goto menu
)

:end
exit /b 0
