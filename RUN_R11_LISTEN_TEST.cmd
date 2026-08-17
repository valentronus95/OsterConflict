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

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%READY_CHECK%" -ProjectRoot "%PROJECT_ROOT%"
set "READY_RC=%ERRORLEVEL%"
if not "%READY_RC%"=="0" (
  pause
  exit /b %READY_RC%
)

if not exist "%MAP_FILE%" (
  echo Creating OsterConflict_Runtime map for this fresh archive...
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

echo Starting R13 content + gameplay listen-server test...
echo Museum menu background + verified R13 weapon art + third-person vehicle default + bots + listen server.
echo.
echo ARMED VEHICLE QUICK CONTROLS:
echo   Solo driver turret: hold RMB to aim, move mouse, LMB fire, R reload.
echo   Dedicated gunner: a second character approaches an occupied armed vehicle and presses E.
echo   C toggles vehicle camera. E exits when the vehicle is slow enough.
echo.
start "Oster Conflict R13" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16" -game -NoFrontend -R12VisualSlice -log -windowed -ResX=1600 -ResY=900 -culture=uk-UA
exit /b 0
