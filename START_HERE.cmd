@echo off
setlocal
cd /d "%~dp0"
:menu
cls
echo ============================================================
echo OSTER CONFLICT - UE 5.8 - R13 CONTENT + GAMEPLAY PASS
echo ============================================================
echo 1. UPDATE + BUILD only  [RECOMMENDED BEFORE TEST]
echo 2. Compile Editor/Game only
echo 3. Full validation (Launcher-aware)
echo 4. Clean full validation
echo 5. Launch R13 local gameplay test + persistent log
echo 6. Launch latest packaged dedicated-server session (source UE only)
echo 7. Stop remembered local server
echo 8. Open Ukrainian first-run README
echo 9. Download + import R13 CC0 models, audio and Oster menu photo
echo A. Open selected FREE Fab weapon/animation/vehicle packs
echo B. Open Unreal Editor manually
echo 0. Exit
echo.
choice /C 123456789AB0 /N /M "Select: "
if errorlevel 12 goto end
if errorlevel 11 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject"
  goto menu
)
if errorlevel 10 (
  call "%~dp0R13_OPEN_FREE_FAB_PACKS.cmd"
  goto menu
)
if errorlevel 9 (
  call "%~dp0R13_DOWNLOAD_AND_IMPORT_CONTENT.cmd"
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
  call "%~dp0RUN_R13_LISTEN_TEST.cmd"
  goto menu
)
if errorlevel 4 (
  call "%~dp0RUN_CLEAN_FULL_TEST.cmd"
  goto menu
)
if errorlevel 3 (
  call "%~dp0RUN_PC_TEST.cmd"
  goto menu
)
if errorlevel 2 (
  call "%~dp0RUN_COMPILE_ONLY.cmd"
  goto menu
)
if errorlevel 1 (
  call "%~dp0R13_UPDATE_BUILD_OPEN.cmd"
  goto menu
)
:end
exit /b 0
