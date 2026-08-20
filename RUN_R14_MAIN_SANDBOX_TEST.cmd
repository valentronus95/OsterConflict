@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "VERIFY=%~dp0VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py"
set "LOG_DIR=%~dp0Logs"
set "PLAYTEST_LOG=%LOG_DIR%\R14_MAIN_LAST_PLAYTEST.log"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PLAYTEST_LOG%" del /q "%PLAYTEST_LOG%" >nul 2>nul

if not exist "%BUILD_BAT%" (
  echo [ERROR] UE 5.8 Build.bat not found at:
  echo   %BUILD_BAT%
  pause
  exit /b 2
)
if not exist "%EDITOR%" (
  echo [ERROR] UnrealEditor.exe not found at:
  echo   %EDITOR%
  pause
  exit /b 3
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found:
  echo   %PROJECT%
  pause
  exit /b 4
)
if not exist "%VERIFY%" (
  echo [ERROR] R14 location ownership verifier is missing:
  echo   %VERIFY%
  echo Pull current main before testing.
  pause
  exit /b 5
)

set "PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)
if not defined PY_CMD (
  echo [ERROR] Python 3 not found in PATH.
  pause
  exit /b 6
)

echo ============================================================
echo OSTER CONFLICT - CURRENT MAIN R14 LOCATION PLAYTEST
echo ============================================================
echo This launcher verifies and builds the CURRENT main source, then opens the runtime map directly.
echo Legacy R11/R13 frontend launchers are not used.
echo.
echo [0/2] Verifying R14 landmark ownership and blocking legacy mixed-location source...
%PY_CMD% "%VERIFY%"
if errorlevel 1 (
  echo.
  echo [STOP] Current working tree is not the expected R14 location integration.
  echo Pull/reset to current origin/main before running this test.
  pause
  exit /b 7
)

echo.
echo [1/2] Building OsterConflictEditor Win64 Development...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
  echo.
  echo [ERROR] UE build failed with exit code %BUILD_RC%.
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  pause
  exit /b %BUILD_RC%
)

echo.
echo [2/2] Launching OsterConflict_Runtime in Sandbox mode...
echo Persistent log:
echo   %PLAYTEST_LOG%
echo.
echo Location check priority:
echo   - Museum must exist only at the museum site.
echo   - Silpo must exist only at Bohdana Khmelnytskoho 54.
echo   - No legacy Culture House/Civic/Silpo composite may appear on either site.
echo.
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"
"%EDITOR%" "%PROJECT%" "%VISUAL_MAP%" -game -NoFrontend -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo R14 MAIN PLAYTEST FINISHED - exit code %GAME_RC%
echo Log: %PLAYTEST_LOG%
echo ============================================================
pause
exit /b %GAME_RC%
