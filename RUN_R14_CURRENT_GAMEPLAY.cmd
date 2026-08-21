@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "VERIFY=%~dp0VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py"
set "M2_IMPORT=%~dp0RUN_IMPORT_M2_PRODUCTION.cmd"
set "BTR4_IMPORT=%~dp0RUN_IMPORT_BTR4_PRODUCTION.cmd"
set "LOG_DIR=%~dp0Logs"
set "PLAYTEST_LOG=%LOG_DIR%\R14_CURRENT_GAMEPLAY.log"
set "R147_ASSET_COMMIT=9fd1d2e450bfcaba668c28aff899986cc87668c4"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PLAYTEST_LOG%" del /q "%PLAYTEST_LOG%" >nul 2>nul

if not exist "%BUILD_BAT%" (
  echo [ERROR] UE 5.8 Build.bat not found: %BUILD_BAT%
  pause
  exit /b 2
)
if not exist "%EDITOR%" (
  echo [ERROR] UnrealEditor.exe not found: %EDITOR%
  pause
  exit /b 3
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found: %PROJECT%
  pause
  exit /b 4
)

where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git was not found in PATH.
  pause
  exit /b 5
)

rem A playtest is useful only when it is actually running current main.
rem Previously this launcher accepted an old local main, so the user could spend time testing code
rem that had already been fixed on GitHub but was not present locally. Never allow that again.
for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if /I not "%CURRENT_BRANCH%"=="main" (
  echo [STOP] Normal gameplay playtest must run from branch main.
  echo Current branch: %CURRENT_BRANCH%
  echo Switch to main, then Fetch origin and Pull origin.
  pause
  exit /b 6
)

echo [PRECHECK] Fetching origin/main so a stale local build cannot be tested...
git fetch origin main
if errorlevel 1 (
  echo [STOP] Could not fetch origin/main. Playtest cancelled instead of testing unknown/stale code.
  pause
  exit /b 7
)
for /f "delims=" %%H in ('git rev-parse HEAD') do set "LOCAL_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse origin/main') do set "REMOTE_HEAD=%%H"
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local main is not current GitHub main.
  echo Local : %LOCAL_HEAD%
  echo GitHub: %REMOTE_HEAD%
  echo GitHub Desktop: Pull origin. Then start the playtest again.
  pause
  exit /b 8
)

git merge-base --is-ancestor %R147_ASSET_COMMIT% HEAD >nul 2>nul
if errorlevel 1 (
  echo [STOP] Local main is missing the current R14 gameplay asset baseline.
  echo GitHub Desktop: Fetch origin, then Pull origin.
  pause
  exit /b 9
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
  exit /b 10
)

if exist "%VERIFY%" (
  echo [0/4] Verifying current R14 landmark ownership...
  %PY_CMD% "%VERIFY%"
  if errorlevel 1 (
    echo [STOP] Current main source verification failed.
    pause
    exit /b 11
  )
)

echo.
echo [1/4] Building current OsterConflictEditor...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
  echo [ERROR] UE build failed with exit code %BUILD_RC%.
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  pause
  exit /b %BUILD_RC%
)

echo.
echo [2/4] Ensuring canonical M2 Browning production visual...
if not exist "%M2_IMPORT%" (
  echo [STOP] M2 import helper is missing. Current main is incomplete.
  pause
  exit /b 12
)
call "%M2_IMPORT%"
if errorlevel 1 (
  echo [STOP] M2 production import failed. Gameplay launch cancelled so a proxy/empty turret is not tested as if it were final.
  pause
  exit /b 13
)

echo.
echo [3/4] Ensuring canonical BTR-4 production visual...
if not exist "%BTR4_IMPORT%" (
  echo [STOP] BTR-4 import helper is missing. Current main is incomplete.
  pause
  exit /b 14
)
call "%BTR4_IMPORT%"
if errorlevel 1 (
  echo [STOP] BTR-4 production import failed. Gameplay launch cancelled so a proxy vehicle is not tested as if it were final.
  pause
  exit /b 15
)

echo.
echo [4/4] Launching CURRENT NORMAL GAME frontend...
echo This is the normal TEAM gameplay route, not the Sandbox/Test Range route.
echo Use START / LOCAL GAME in the game menu to enter the listen-server match.
echo Log: %PLAYTEST_LOG%
echo Source: %LOCAL_HEAD%
echo.
start /wait "Oster Conflict Current Gameplay" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo CURRENT GAMEPLAY FINISHED - exit code %GAME_RC%
echo Source: %LOCAL_HEAD%
echo Log: %PLAYTEST_LOG%
echo ============================================================
pause
exit /b %GAME_RC%
