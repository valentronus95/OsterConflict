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
  echo [ERROR] Git was not found in PATH. Cannot verify current main.
  pause
  exit /b 6
)

rem Sandbox is a runtime acceptance route, not permission to test stale source.
for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if /I not "%CURRENT_BRANCH%"=="main" (
  echo [STOP] Sandbox playtest must run from branch main.
  echo Current branch: %CURRENT_BRANCH%
  echo Switch to main, then Fetch origin and Pull origin.
  pause
  exit /b 7
)

echo [PRECHECK] Fetching origin/main so a stale Sandbox build cannot be tested...
git fetch origin main
if errorlevel 1 (
  echo [STOP] Could not fetch origin/main. Sandbox cancelled instead of testing unknown/stale code.
  pause
  exit /b 8
)
for /f "delims=" %%H in ('git rev-parse HEAD') do set "LOCAL_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse origin/main') do set "REMOTE_HEAD=%%H"
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local main is not current GitHub main.
  echo Local : %LOCAL_HEAD%
  echo GitHub: %REMOTE_HEAD%
  echo GitHub Desktop: Pull origin. Then start the Sandbox playtest again.
  pause
  exit /b 9
)

echo [PRECHECK] Runtime SHA: %LOCAL_HEAD%

git merge-base --is-ancestor %R147_ASSET_COMMIT% HEAD >nul 2>nul
if errorlevel 1 (
  echo ============================================================
  echo R14.7 PLAYTEST BLOCKED: LOCAL MAIN IS MISSING LATEST GAMEPLAY ASSETS
  echo ============================================================
  echo GitHub Desktop: Fetch origin, then Pull origin.
  echo Required baseline: %R147_ASSET_COMMIT%
  echo The game will NOT launch an older weapon/audio/menu asset set.
  pause
  exit /b 10
)

git lfs version >nul 2>nul
if errorlevel 1 (
  echo ============================================================
  echo PLAYTEST BLOCKED: GIT LFS IS NOT AVAILABLE
  echo ============================================================
  echo Production UE assets are stored through Git LFS.
  echo Install/repair Git LFS, run git lfs install, then launch again.
  pause
  exit /b 11
)

echo [ASSET] Hydrating current Git LFS objects...
git lfs pull
if errorlevel 1 (
  echo [ERROR] git lfs pull failed. Production meshes are not trustworthy for this run.
  pause
  exit /b 12
)

set "LFS_ASSET_ERROR=0"
for %%F in (
  "OsterConflict\Content\QuantumCharacter\Mesh\SKM_QuantumCharacter.uasset"
  "OsterConflict\Content\QuantumCharacter\Mesh\Modules\SKM_Arms.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\1911\SKM_1911.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\Mac10\SKM_Mac10.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\M14\SKM_M14.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\M700\SKM_M700.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\MP5\SKM_MP5.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\Tec9\SKM_Tec9.uasset"
  "OsterConflict\Content\R13\Weapons\Stein\LeverAction\SKM_LeverAction.uasset"
  "OsterConflict\Content\R13\Weapons\machinegun.uasset"
  "OsterConflict\Content\R13\Weapons\pistol.uasset"
  "OsterConflict\Content\R13\Weapons\uzi.uasset"
  "OsterConflict\Content\R13\Weapons\shotgun.uasset"
  "OsterConflict\Content\R13\Weapons\rocketlauncherModern.uasset"
  "OsterConflict\Content\AK-47\Mesh\SKM_AK-47.uasset"
) do (
  if not exist "%%~F" (
    echo [ASSET ERROR] Missing required file: %%~F
    set "LFS_ASSET_ERROR=1"
  ) else (
    for %%A in ("%%~F") do if %%~zA LSS 4096 (
      echo [ASSET ERROR] File is still an LFS pointer or invalid tiny asset: %%~F ^(%%~zA bytes^)
      set "LFS_ASSET_ERROR=1"
    )
  )
)
if "%LFS_ASSET_ERROR%"=="1" (
  echo.
  echo PLAYTEST BLOCKED: REQUIRED PRODUCTION/R13 ASSETS ARE NOT HYDRATED.
  echo Do not interpret primitive fallback geometry as a gameplay result.
  pause
  exit /b 13
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
  exit /b 14
)

echo ============================================================
echo OSTER CONFLICT - CURRENT MAIN R14.7 LOCATION TEST
echo LATEST LOCATIONS + LATEST IMPORTED GAMEPLAY ASSETS
echo Runtime SHA: %LOCAL_HEAD%
echo ============================================================
echo This launcher refuses to run stale main or unhydrated critical Git LFS content.
echo Legacy R11/R13 mixed-location launchers are not used.
echo LocationTest=1 is mandatory here: it isolates the current-main location test contract and test weapon rack.
echo.
echo KNOWN OPEN ASSET GAPS ON CURRENT MAIN:
echo   - Exact /Game/Production/Weapons/M249 asset is absent; a real R13 machinegun mesh is diagnostic fallback only.
echo   - Exact /Game/Production/Weapons/Remington870 asset is absent; a real R13 shotgun mesh is diagnostic fallback only.
echo   - M2 has an automatic canonical import path; downloaded local GLB wins, authored game-visual GLB is the fallback.
echo   - BTR-4 has an automatic canonical import path; local user FBX wins when present, authored external-only 8x8 GLB is the fallback.
echo None of those fallbacks may be marked VERIFIED without UE runtime proof.
echo.
echo [0/4] Verifying exclusive landmark ownership and map-separation guards...
%PY_CMD% "%VERIFY%"
if errorlevel 1 (
  echo.
  echo [STOP] Current working tree is not the expected R14 location integration.
  echo Pull current origin/main before running this test.
  pause
  exit /b 15
)

echo.
echo [1/4] Building OsterConflictEditor Win64 Development...
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
echo [2/4] Ensuring canonical M2 Browning production visual...
if exist "%M2_IMPORT%" (
  call "%M2_IMPORT%"
  if errorlevel 1 (
    echo [WARN] M2 production import did not complete. Sandbox will retain the real-machinegun diagnostic fallback.
  )
) else (
  echo [WARN] M2 import helper is missing. Pull current main to enable automatic M2 import.
)

echo.
echo [3/4] Ensuring canonical BTR-4 production visual...
if exist "%BTR4_IMPORT%" (
  call "%BTR4_IMPORT%"
  if errorlevel 1 (
    echo [WARN] BTR-4 production import did not complete. Sandbox may retain the legacy proxy shell.
  )
) else (
  echo [WARN] BTR-4 import helper is missing. Pull current main to enable automatic BTR-4 import.
)

echo.
echo [4/4] Launching OsterConflict_Runtime in Sandbox LocationTest mode...
echo Persistent log:
echo   %PLAYTEST_LOG%
echo.
echo R14.7 content baseline includes hydrated AK47, 1911, M14, M700,
echo MP5, MAC-10, TEC-9, Lever Action, generic real weapon meshes, QuantumCharacter,
echo combat audio and menu background.
echo.
echo Runtime check priority:
echo   - Confirm the printed Runtime SHA equals current GitHub main before judging visuals.
echo   - LocationTest=1 is active on the current-main OsterConflict_Runtime map.
echo   - The test weapon rack is created beside the actually deployed/possessed pawn.
echo   - The LocationTest rack contains all 11 implemented pickup classes and legacy world pickups are suppressed for this test.
echo   - M opens/closes the tactical map; DeployTrap is on V, not M.
echo   - Tactical Map uses the dark world-synchronised vector layer; raw green scene capture is not the production map.
echo   - Pickup/HMMWV mounted gun should resolve to /Game/Production/Weapons/M2/SM_M2_Browning; verify scale, pivot, muzzle and gunner alignment.
echo   - BTR should resolve to /Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus; verify scale, ground contact, all 8 wheels, camera framing and no green proxy shell.
echo   - Enter vehicle, drive, exit, then immediately verify WASD + sprint + mouse look.
echo   - No primitive weapon fallback is visible for M249/M1911/MAC-10/Remington 870.
echo   - Museum exists only at the museum geo site.
echo   - Silpo exists only at Bohdana Khmelnytskoho 54.
echo   - Culture House exists only at Hranovskoho 3.
echo   - No generic building shell is nested inside any of those three landmarks.
echo   - No legacy Culture House/Civic/Silpo composite appears at the museum or Silpo site.
echo   - The old synthetic straight CentralPark-to-north-civic sidewalk/grove is absent.
echo   - Stadium remains on its separate hard-georeferenced site.
echo.
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1?LocationTest=1"
"%EDITOR%" "%PROJECT%" "%VISUAL_MAP%" -game -NoFrontend -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo R14.7 MAIN LOCATION TEST FINISHED - exit code %GAME_RC%
echo Log: %PLAYTEST_LOG%
echo ============================================================
pause
exit /b %GAME_RC%