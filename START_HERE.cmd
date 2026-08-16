@echo off
setlocal
cd /d "%~dp0"
:menu
cls
echo ============================================================
echo OSTER CONFLICT - UE 5.8 R11 VISUAL FOUNDATION - R11.1 LAUNCHER FIXED
echo ============================================================
echo 1. Compile Editor/Game with installed UE 5.8
echo 2. Full validation (Launcher-aware)
echo 3. Clean full validation
echo 4. Launch R11 local listen-server visual test
echo 5. Launch latest packaged dedicated-server session (source UE only)
echo 6. Stop remembered local server
echo 7. Open Ukrainian first-run README
echo 0. Exit
echo.
choice /C 12345670 /N /M "Select: "
if errorlevel 8 goto end
if errorlevel 7 (
  start "" notepad.exe "%~dp0FIRST_RUN_README_UA.txt"
  goto menu
)
if errorlevel 6 (
  call "%~dp0STOP_LOCAL_SERVER.cmd"
  goto menu
)
if errorlevel 5 (
  call "%~dp0RUN_LOCAL_GAME_AFTER_BUILD.cmd"
  goto menu
)
if errorlevel 4 (
  call "%~dp0RUN_R11_LISTEN_TEST.cmd"
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
