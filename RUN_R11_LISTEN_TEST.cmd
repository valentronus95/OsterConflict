@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "EDITOR_DLL=%~dp0OsterConflict\Binaries\Win64\UnrealEditor-OsterConflict.dll"
set "SOURCE_ROOT=%~dp0OsterConflict\Source"
set "MAP_FILE=%~dp0OsterConflict\Content\Maps\OsterConflict_Runtime.umap"
set "MAP_SCRIPT=%~dp0OsterConflict\Scripts\S18B\CREATE_RELEASE_MAP.py"
set "R13_STATE=%~dp0OsterConflict\Content\Raw\R13\R13_IMPORT_STATE.txt"
set "R13_WEAPONS=%~dp0OsterConflict\Content\R13\Weapons"
set "R13_UI=%~dp0OsterConflict\Content\R13\UI"

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

if not exist "%EDITOR_DLL%" (
  echo [ERROR] Editor module is not built yet.
  echo Run START_HERE option 1 first.
  pause
  exit /b 4
)

rem A successful old DLL is still the wrong build after pulling new C++ source. Refuse to run stale gameplay.
powershell.exe -NoProfile -Command "$dll=Get-Item -LiteralPath '%EDITOR_DLL%'; $latest=Get-ChildItem -LiteralPath '%SOURCE_ROOT%' -Recurse -File -Include *.h,*.cpp ^| Sort-Object LastWriteTimeUtc -Descending ^| Select-Object -First 1; if($latest -and $latest.LastWriteTimeUtc -gt $dll.LastWriteTimeUtc){ exit 9 }"
if errorlevel 9 (
  echo.
  echo ============================================================
  echo R13 GAMEPLAY LAUNCH BLOCKED: C++ BUILD IS STALE
  echo ============================================================
  echo Source files are newer than UnrealEditor-OsterConflict.dll.
  echo Run START_HERE option 1, then return to option 4.
  echo ============================================================
  pause
  exit /b 9
)

rem Do not launch another knowingly-placeholder visual test. R13 gameplay QA now requires the current museum/photo +
rem imported weapon art state and exact runtime .uassets before the game window is allowed to open.
set "R13_ART_MISSING=0"
if not exist "%R13_STATE%" set "R13_ART_MISSING=1"
if exist "%R13_STATE%" (
  findstr /x /c:"R13_MUSEUM_WEAPONS_V2" "%R13_STATE%" >nul || set "R13_ART_MISSING=1"
)
for %%F in (machinegun pistol shotgun sniper uzi rocketlauncherModern grenade) do (
  if not exist "%R13_WEAPONS%\%%F.uasset" (
    echo [MISSING] R13 weapon asset: %%F.uasset
    set "R13_ART_MISSING=1"
  )
)
if not exist "%R13_UI%\Oster_Menu_BG.uasset" (
  echo [MISSING] R13 menu background: Oster_Menu_BG.uasset
  set "R13_ART_MISSING=1"
)
if "%R13_ART_MISSING%"=="1" (
  echo.
  echo ============================================================
  echo R13 GAMEPLAY LAUNCH BLOCKED: REQUIRED ART IS MISSING OR STALE
  echo ============================================================
  echo Run START_HERE option 8 first.
  echo It now verifies the museum background and required weapon assets
  echo instead of letting the gameplay test silently fall back to blocks.
  echo ============================================================
  pause
  exit /b 7
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
