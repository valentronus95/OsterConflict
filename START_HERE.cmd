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
echo 1. ТЕСТ ГРИ - зібрати current main і запустити LocationTest
echo 2. ЗВИЧАЙНА ГРА - головне меню і TEAM gameplay
echo 3. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Для звичайної роботи використовуй тільки цей START_HERE.cmd.
echo Інші RUN_*.cmd - внутрішні технічні скрипти, їх запускати не потрібно.
echo.
choice /C 1230 /N /M "Оберіть: "

if errorlevel 4 goto end
if errorlevel 3 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject"
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  goto menu
)
if errorlevel 1 (
  call "%~dp0RUN_R14_MAIN_SANDBOX_TEST.cmd"
  goto menu
)

:end
exit /b 0
