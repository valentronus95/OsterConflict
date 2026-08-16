@echo off
setlocal
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "MAP_FILE=%~dp0OsterConflict\Content\Maps\OsterConflict_Runtime.umap"
set "MAP_SCRIPT=%~dp0OsterConflict\Scripts\S18B\CREATE_RELEASE_MAP.py"

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

if not exist "%~dp0OsterConflict\Binaries\Win64\UnrealEditor-OsterConflict.dll" (
  echo [ERROR] Editor module is not built yet.
  echo Run START_HERE option 1 first.
  pause
  exit /b 4
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

echo Starting R12.2 Krushelnytska visual gameplay test...
echo Real environment slice near spawn + neutral daylight + vehicles + bots + listen server.
start "Oster Conflict R12.2" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16" -game -NoFrontend -R12VisualSlice -log -windowed -ResX=1600 -ResY=900 -culture=uk-UA
exit /b 0
