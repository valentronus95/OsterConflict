@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "BRANCH=r13-content-gameplay-pass"
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"

echo ============================================================
echo OSTER CONFLICT - UPDATE + BUILD + OPEN
echo ============================================================

where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git is not available in PATH.
  pause
  exit /b 10
)

if not exist ".git" (
  echo [ERROR] This file must be run from the OsterConflict repository root.
  pause
  exit /b 11
)

if not exist "%BUILD_BAT%" (
  echo [ERROR] Unreal Engine 5.8 was not found at:
  echo %UE_ROOT%
  pause
  exit /b 12
)

if not exist "%PROJECT%" (
  echo [ERROR] Project file not found:
  echo %PROJECT%
  pause
  exit /b 13
)

for /f "delims=" %%I in ('git status --porcelain --untracked-files=no') do (
  echo [STOP] Tracked local changes were found.
  echo Nothing was reset, deleted or overwritten.
  echo Commit/stash those changes before using this launcher.
  git status --short
  pause
  exit /b 20
)

echo [1/4] Fetching latest repository state...
git fetch origin
if errorlevel 1 goto :git_fail

echo [2/4] Switching to %BRANCH% and fast-forwarding only...
git switch "%BRANCH%"
if errorlevel 1 goto :git_fail
git pull --ff-only origin "%BRANCH%"
if errorlevel 1 goto :git_fail

where git-lfs >nul 2>nul
if not errorlevel 1 (
  echo Pulling Git LFS payloads...
  git lfs pull
  if errorlevel 1 goto :git_fail
)

echo [3/4] Building OsterConflictEditor with UE 5.8...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
if errorlevel 1 goto :build_fail

echo [4/4] Opening Unreal Editor...
start "" "%EDITOR%" "%PROJECT%"

echo.
echo SUCCESS: repository updated, editor build succeeded, project opened.
exit /b 0

:git_fail
echo.
echo [ERROR] Git update failed. No hard reset was performed.
pause
exit /b 30

:build_fail
echo.
echo [ERROR] Editor build failed.
echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
pause
exit /b 40
