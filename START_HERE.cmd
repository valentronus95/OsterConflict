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
echo 1. ЗВИЧАЙНА ГРА - головне меню, потім START / TEAM gameplay
echo 2. ТЕХНІЧНИЙ ТЕСТ ГРИ - прямий LocationTest БЕЗ головного меню
echo 3. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Для звичайного запуску гри обирай 1.
echo Пункт 2 потрібен тільки для швидкої діагностики runtime і навмисно обходить головне меню.
echo Інші RUN_*.cmd - внутрішні технічні скрипти, їх запускати не потрібно.
echo.
choice /C 1230 /N /M "Оберіть: "

if errorlevel 4 goto end
if errorlevel 3 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject"
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_R14_MAIN_SANDBOX_TEST.cmd"
  goto menu
)
if errorlevel 1 (
  call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
  goto menu
)

:end
exit /b 0
