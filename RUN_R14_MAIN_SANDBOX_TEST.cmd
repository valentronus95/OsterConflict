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
set "R147_ASSET_COMMIT=9fd1d2e450bfcaba668c28aff899986cc87668c4"

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

where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git was not found in PATH. Cannot verify the R14.7 asset baseline.
  pause
  exit /b 6
)

git merge-base --is-ancestor %R147_ASSET_COMMIT% HEAD >nul 2>nul
if errorlevel 1 (
  echo ============================================================
  echo R14.7 PLAYTEST BLOCKED: LOCAL MAIN IS MISSING LATEST GAMEPLAY ASSETS
  echo ============================================================
  echo GitHub Desktop: Fetch origin, then Pull origin.
  echo Required baseline: %R147_ASSET_COMMIT%
  echo The game will NOT launch an older weapon/audio/menu asset set.
  pause
  exit /b 7
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
  exit /b 8
)

echo ============================================================
echo OSTER CONFLICT - CURRENT MAIN R14.7 PLAYTEST
echo LATEST LOCATIONS + LATEST IMPORTED GAMEPLAY ASSETS
echo ============================================================
echo This launcher refuses to run if the latest R14.7 gameplay asset baseline is missing.
echo Legacy R11/R13 mixed-location launchers are not used.
echo.
echo [0/2] Verifying exclusive landmark ownership and map-separation guards...
%PY_CMD% "%VERIFY%"
if errorlevel 1 (
  echo.
  echo [STOP] Current working tree is not the expected R14 location integration.
  echo Pull current origin/main before running this test.
  pause
  exit /b 9
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
echo R14.7 content baseline includes the latest imported AK47, 1911, M14, M700,
echo MP5, MAC-10, TEC-9, Lever Action, generic weapon assets, combat audio and menu background.
echo.
echo Location check priority:
echo   - Museum exists only at the museum geo site.
echo   - Silpo exists only at Bohdana Khmelnytskoho 54.
echo   - Culture House exists only at Hranovskoho 3.
echo   - No generic building shell is nested inside any of those three landmarks.
echo   - No legacy Culture House/Civic/Silpo composite appears at the museum or Silpo site.
echo   - The old synthetic straight CentralPark-to-north-civic sidewalk/grove is absent.
echo   - Stadium remains on its separate hard-georeferenced site.
echo.
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"
"%EDITOR%" "%PROJECT%" "%VISUAL_MAP%" -game -NoFrontend -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo R14.7 MAIN PLAYTEST FINISHED - exit code %GAME_RC%
echo Log: %PLAYTEST_LOG%
echo ============================================================
pause
exit /b %GAME_RC%
