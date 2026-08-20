@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT_ROOT=%~dp0OsterConflict"
set "PROJECT=%PROJECT_ROOT%\OsterConflict.uproject"
set "MAP_FILE=%PROJECT_ROOT%\Content\Maps\OsterConflict_Runtime.umap"
set "MAP_SCRIPT=%PROJECT_ROOT%\Scripts\S18B\CREATE_RELEASE_MAP.py"
set "READY_CHECK=%~dp0PC_TEST\CHECK_R13_LAUNCH_READY.ps1"
set "LFS_CHECK=%~dp0PC_TEST\CHECK_R13_LFS_PAYLOADS.ps1"
set "LOG_DIR=%~dp0Logs"
set "PLAYTEST_LOG=%LOG_DIR%\R13_LAST_PLAYTEST.log"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PLAYTEST_LOG%" del /q "%PLAYTEST_LOG%" >nul 2>nul

if not exist "%EDITOR%" (
  echo UE 5.8 editor not found at:
  echo   %EDITOR%
  echo.
  set /p "UE_ROOT=Enter UE 5.8 installation root: "
  set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
  set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

if not exist "%EDITOR%" (
  echo [ERROR] UnrealEditor.exe not found.
  pause
  exit /b 2
)
if not exist "%EDITOR_CMD%" (
  echo [ERROR] UnrealEditor-Cmd.exe not found.
  pause
  exit /b 2
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found: %PROJECT%
  pause
  exit /b 3
)
if not exist "%READY_CHECK%" (
  echo [ERROR] R13 launch readiness checker not found: %READY_CHECK%
  pause
  exit /b 3
)
if not exist "%LFS_CHECK%" (
  echo [ERROR] R13 Git LFS payload checker not found: %LFS_CHECK%
  pause
  exit /b 3
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%READY_CHECK%" -ProjectRoot "%PROJECT_ROOT%"
set "READY_RC=%ERRORLEVEL%"
if not "%READY_RC%"=="0" (
  echo.
  echo [STOP] Launch readiness check failed.
  pause
  exit /b %READY_RC%
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%" -ProjectRoot "%PROJECT_ROOT%"
set "LFS_RC=%ERRORLEVEL%"
if not "%LFS_RC%"=="0" (
  echo.
  echo [STOP] Git LFS payload check failed.
  pause
  exit /b %LFS_RC%
)

if not exist "%MAP_FILE%" (
  echo Creating OsterConflict_Runtime map for this fresh checkout...
  "%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%MAP_SCRIPT%" -unattended -nop4 -NullRHI -NoSplash -UTF8Output
  if errorlevel 1 (
    echo [ERROR] Runtime map generation failed.
    pause
    exit /b 5
  )
  if not exist "%MAP_FILE%" (
    echo [ERROR] Runtime map was not created: %MAP_FILE%
    pause
    exit /b 6
  )
)

echo Starting Oster Conflict standalone gameplay test...
echo The full Unreal Editor UI will NOT be opened.
echo This development build still uses UnrealEditor.exe with -game until a packaged EXE is produced.
echo.
echo Persistent runtime log:
echo   %PLAYTEST_LOG%
echo.
echo No match, bots, vehicles or gameplay audio are started behind the main menu.
echo Choose START or LOCAL GAME in the menu to create the listen-server match.
echo.
echo VEHICLE QUICK CONTROLS AFTER MATCH START:
echo   Driver: W/S drive, A/D steer, RMB free look, C camera, E exit when slow.
echo   Turret control belongs to the dedicated gunner seat; driver mouse does not steal the turret.
echo.
rem IMPORTANT: launch the runtime map as a standalone frontend shell first.
rem StartLocalGameplay() performs the explicit travel to the listen-server URL with R13Gameplay=1.
rem /wait keeps this console alive and -abslog preserves the exact runtime diagnostics after a freeze/crash/exit.
start /wait "Oster Conflict R13 Playtest" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo PLAYTEST FINISHED - exit code %GAME_RC%
echo Log preserved at:
echo %PLAYTEST_LOG%
echo ============================================================
echo This window stays open so the result cannot disappear again.
pause
exit /b %GAME_RC%
