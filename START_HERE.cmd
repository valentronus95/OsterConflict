@echo off
setlocal
cd /d "%~dp0"
:menu
cls
echo ============================================================
echo OSTER CONFLICT - UE 5.8 - R14.7 CURRENT MAIN - LATEST GAMEPLAY ASSETS
echo ============================================================
echo 1. Compile Editor/Game with installed UE 5.8
echo 2. Full validation (Launcher-aware)
echo 3. Clean full validation
echo 4. Validate Silpo R14 source + UE build
echo 5. Launch CURRENT R14.7 Sandbox - latest locations + gameplay assets
echo 6. Launch latest packaged dedicated-server session (source UE only)
echo 7. Stop remembered local server
echo 8. Open Ukrainian first-run README
echo B. Open Unreal Editor manually
echo 0. Exit
echo.
choice /C 12345678B0 /N /M "Select: "
if errorlevel 10 goto end
if errorlevel 9 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject"
  goto menu
)
if errorlevel 8 (
  start "" notepad.exe "%~dp0FIRST_RUN_README_UA.txt"
  goto menu
)
if errorlevel 7 (
  call "%~dp0STOP_LOCAL_SERVER.cmd"
  goto menu
)
if errorlevel 6 (
  call "%~dp0RUN_LOCAL_GAME_AFTER_BUILD.cmd"
  goto menu
)
if errorlevel 5 (
  call "%~dp0RUN_R14_MAIN_SANDBOX_TEST.cmd"
  goto menu
)
if errorlevel 4 (
  call "%~dp0VALIDATE_SILPO_UE58.cmd"
  goto menu
)
if errorlevel 3 (
  call "%~dp0RUN_CLEAN_FULL_TEST.cmd"
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_PC_TEST.cmd"
  goto menu
)
if errorlevel 1 (
  call "%~dp0RUN_COMPILE_ONLY.cmd"
  goto menu
)
:end
exit /b 0
