@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"
set "PROJECT_ROOT=%CD%"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "VERIFY=%~dp0VERIFY_R14_MAIN_LOCATION_OWNERSHIP.py"
set "PRODUCTION_IMPORT=%~dp0OsterConflict\IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
set "WEAPON_VERIFY=%~dp0OsterConflict\Scripts\verify_required_weapon_assets.py"
set "LFS_VERIFY_PS=%~dp0OsterConflict\Scripts\verify_playtest_lfs_payloads.ps1"
set "WEAPON_SENTINEL=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\required_weapon_asset_preflight_success.txt"
set "LOG_DIR=%~dp0Logs"
set "PLAYTEST_LOG=%LOG_DIR%\R14_CURRENT_GAMEPLAY.log"
set "WEAPON_PREFLIGHT_LOG=%LOG_DIR%\R14_REQUIRED_WEAPON_ASSETS.log"
set "R147_ASSET_COMMIT=9fd1d2e450bfcaba668c28aff899986cc87668c4"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PLAYTEST_LOG%" del /q "%PLAYTEST_LOG%" >nul 2>nul
if exist "%WEAPON_PREFLIGHT_LOG%" del /q "%WEAPON_PREFLIGHT_LOG%" >nul 2>nul
if exist "%WEAPON_SENTINEL%" del /q "%WEAPON_SENTINEL%" >nul 2>nul

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
if not exist "%EDITOR_CMD%" (
  echo [ERROR] UnrealEditor-Cmd.exe not found: %EDITOR_CMD%
  pause
  exit /b 4
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found: %PROJECT%
  pause
  exit /b 5
)
if not exist "%WEAPON_VERIFY%" (
  echo [ERROR] Required weapon preflight script is missing: %WEAPON_VERIFY%
  pause
  exit /b 6
)
if not exist "%LFS_VERIFY_PS%" (
  echo [ERROR] Git LFS payload verifier is missing: %LFS_VERIFY_PS%
  pause
  exit /b 6
)

where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git was not found in PATH.
  pause
  exit /b 7
)

for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if /I not "%CURRENT_BRANCH%"=="main" (
  echo [STOP] Normal gameplay playtest must run from branch main.
  echo Current branch: %CURRENT_BRANCH%
  echo Switch to main, then Fetch origin and Pull origin.
  pause
  exit /b 8
)

echo [PRECHECK] Fetching origin/main so a stale local build cannot be tested...
git fetch origin main
if errorlevel 1 (
  echo [STOP] Could not fetch origin/main. Playtest cancelled instead of testing unknown/stale code.
  pause
  exit /b 9
)
for /f "delims=" %%H in ('git rev-parse HEAD') do set "LOCAL_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse origin/main') do set "REMOTE_HEAD=%%H"
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local main is not current GitHub main.
  echo Local : %LOCAL_HEAD%
  echo GitHub: %REMOTE_HEAD%
  echo GitHub Desktop: Pull origin. Then start the playtest again.
  pause
  exit /b 10
)

git merge-base --is-ancestor %R147_ASSET_COMMIT% HEAD >nul 2>nul
if errorlevel 1 (
  echo [STOP] Local main is missing the current R14 gameplay asset baseline.
  echo GitHub Desktop: Fetch origin, then Pull origin.
  pause
  exit /b 11
)

rem Hydrate the current main LFS payloads using only commands supported by older Git LFS releases.
rem The previous --include form is not accepted by the Git LFS build installed on the playtest PC.
echo [ASSETS] Hydrating real weapon and foliage files from Git LFS...
git lfs version >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git LFS is not installed or not available in PATH.
  echo Install Git LFS, then run START_HERE.cmd again.
  pause
  exit /b 12
)
git lfs install >nul 2>nul
git lfs pull origin
if errorlevel 1 (
  echo [STOP] Git LFS could not hydrate the current main assets.
  pause
  exit /b 13
)
git lfs checkout >nul
if errorlevel 1 (
  echo [STOP] Git LFS checkout failed after pull.
  pause
  exit /b 13
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%LFS_VERIFY_PS%" -ProjectRoot "%PROJECT_ROOT%"
if errorlevel 1 (
  pause
  exit /b 14
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
  exit /b 15
)

if exist "%VERIFY%" (
  echo [0/4] Verifying current R14 landmark ownership...
  %PY_CMD% "%VERIFY%"
  if errorlevel 1 (
    echo [STOP] Current main source verification failed.
    pause
    exit /b 16
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
echo [2/4] Opening every required REAL weapon visual in a fresh UE process...
if exist "%WEAPON_SENTINEL%" del /q "%WEAPON_SENTINEL%" >nul 2>nul
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%WEAPON_VERIFY%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%WEAPON_PREFLIGHT_LOG%"
set "WEAPON_RC=%ERRORLEVEL%"
if not "%WEAPON_RC%"=="0" (
  echo [STOP] UE weapon asset preflight process failed with code %WEAPON_RC%.
  echo Log: %WEAPON_PREFLIGHT_LOG%
  pause
  exit /b 17
)
if not exist "%WEAPON_SENTINEL%" (
  echo [STOP] One or more required weapon models could not be opened by Unreal.
  echo Primitive weapon boxes are not accepted as a fallback for the normal playtest.
  echo Log: %WEAPON_PREFLIGHT_LOG%
  pause
  exit /b 18
)

echo.
echo [3/4] Importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets...
if not exist "%PRODUCTION_IMPORT%" (
  echo [STOP] Full production vehicle importer is missing: %PRODUCTION_IMPORT%
  pause
  exit /b 19
)
call "%PRODUCTION_IMPORT%"
if errorlevel 1 (
  echo [STOP] Production model ingest failed.
  echo The game will not launch with civilian pickup/proxy turret/proxy BTR geometry pretending to be final assets.
  echo Required local sources include:
  echo   OsterConflict\SourceAssets\Production\Vehicles\HMMWV\ukrainian_hmmwv_mk_19.glb
  echo   OsterConflict\SourceAssets\Production\Weapons\M2\m2_50cal_machinegun_cc0.glb
  echo   OsterConflict\SourceAssets\Production\Vehicles\BTR4\BTR4_Bucephalus.fbx
  pause
  exit /b 20
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
