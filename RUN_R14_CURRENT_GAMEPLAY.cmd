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
set "IS_ACCEPTANCE=0"
if /I "%OC_FORCE_ACCEPTANCE%"=="1" set "IS_ACCEPTANCE=1"

rem Pass 45: keep the stable DX11/SM5/no-HDR route. Quality is restored through DX11-safe scalability,
rem rather than the old diagnostic low-quality profile. -nosplash removes the separate oversized startup image.
set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"
set "RHI_MODE=dx11_sm5_rhi_thread"
if /I "%OC_RHI_COMPAT%"=="1" (
  set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread -nosplash"
  set "RHI_MODE=dx11_sm5_no_rhi_thread_compat"
)
set "QUALITY_CMDS=t.MaxFPS 60,sg.ViewDistanceQuality 3,sg.ShadowQuality 2,sg.TextureQuality 3,sg.EffectsQuality 3,sg.FoliageQuality 2,sg.PostProcessQuality 3,sg.AntiAliasingQuality 3,sg.ShadingQuality 3,sg.GlobalIlluminationQuality 2,sg.ReflectionQuality 2,sg.LandscapeQuality 3,r.ScreenPercentage 100"

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
  exit /b 5
)

rem START_HERE option 1/3 deliberately uses this canonical launcher in lightweight mode.
rem It must not hydrate LFS, run commandlet weapon preflight, import vehicles/materials, or promote runtime acceptance.
if /I "%OC_QUICK_NORMAL%"=="1" goto quick_normal_game

if exist "%WEAPON_PREFLIGHT_LOG%" del /q "%WEAPON_PREFLIGHT_LOG%" >nul 2>nul
if exist "%WEAPON_SENTINEL%" del /q "%WEAPON_SENTINEL%" >nul 2>nul
if not exist "%EDITOR_CMD%" (
  echo [ERROR] UnrealEditor-Cmd.exe not found: %EDITOR_CMD%
  pause
  exit /b 4
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
set "FETCH_BRANCH="
set "REMOTE_REF="
if /I "%CURRENT_BRANCH%"=="main" (
  set "FETCH_BRANCH=main"
  set "REMOTE_REF=origin/main"
) else (
  echo(%CURRENT_BRANCH%| findstr /B /I /C:"fix/runtime-acceptance-" /C:"fix/runtime-map-spawn-fps-assets-" /C:"fix/runtime-recovery-" /C:"fix/pass45-runtime-rejection-" /C:"fix/single-launcher-" /C:"fix/dx11-sm5-" >nul
  if errorlevel 1 (
    echo [STOP] Normal gameplay playtest is allowed only from main or an explicit runtime-fix branch.
    echo Current branch: %CURRENT_BRANCH%
    pause
    exit /b 8
  )
  set "FETCH_BRANCH=%CURRENT_BRANCH%"
  set "REMOTE_REF=origin/%CURRENT_BRANCH%"
  set "IS_ACCEPTANCE=1"
  echo [ACCEPTANCE] Running isolated runtime acceptance branch: %CURRENT_BRANCH%
  echo [ACCEPTANCE] main will remain untouched until this branch passes the UE runtime playtest.
)

if "%IS_ACCEPTANCE%"=="1" if /I "%CURRENT_BRANCH%"=="main" (
  echo [ACCEPTANCE] Running strict runtime acceptance from current main.
  echo [ACCEPTANCE] The launcher will reject missing Museum / required-available weapon / vehicle READY evidence after the game closes.
)

echo [PRECHECK] Fetching origin/%FETCH_BRANCH% so a stale local build cannot be tested...
git fetch origin "%FETCH_BRANCH%"
if errorlevel 1 (
  echo [STOP] Could not fetch origin/%FETCH_BRANCH%. Playtest cancelled instead of testing unknown/stale code.
  pause
  exit /b 9
)
for /f "delims=" %%H in ('git rev-parse HEAD') do set "LOCAL_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse "%REMOTE_REF%"') do set "REMOTE_HEAD=%%H"
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local %CURRENT_BRANCH% is not current GitHub %REMOTE_REF%.
  echo Local : %LOCAL_HEAD%
  echo GitHub: %REMOTE_HEAD%
  echo GitHub Desktop: Fetch origin, then Pull origin. Then start the playtest again.
  pause
  exit /b 10
)

for /f "delims=" %%D in ('git status --porcelain --untracked-files=all') do echo [LOCAL CHANGE] %%D

git merge-base --is-ancestor %R147_ASSET_COMMIT% HEAD >nul 2>nul
if errorlevel 1 (
  echo [STOP] Current branch is missing the current R14 gameplay asset baseline.
  echo GitHub Desktop: Fetch origin, then Pull origin.
  pause
  exit /b 11
)

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
  echo [STOP] Git LFS could not hydrate the current branch assets.
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
    echo [STOP] Current source verification failed.
    pause
    exit /b 16
  )
)

if exist "%~dp0VERIFY_RUNTIME_ACCEPTANCE_PASS_7.py" (
  echo [0b/4] Verifying runtime acceptance Pass 7 source contracts...
  %PY_CMD% "%~dp0VERIFY_RUNTIME_ACCEPTANCE_PASS_7.py"
  if errorlevel 1 (
    echo [STOP] Pass 7 source verification failed.
    pause
    exit /b 16
  )
)

if exist "%~dp0VERIFY_RUNTIME_RECONCILE_PASS_8.py" (
  echo [0c/4] Verifying runtime reconciliation Pass 8 source contracts...
  %PY_CMD% "%~dp0VERIFY_RUNTIME_RECONCILE_PASS_8.py"
  if errorlevel 1 (
    echo [STOP] Pass 8 source verification failed.
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
echo [2/4] Opening every required REAL/playable weapon visual in a fresh UE process...
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
  echo [STOP] One or more required playable weapon models could not be opened by Unreal.
  echo Primitive-only weapon boxes are not accepted for the normal playtest.
  echo Log: %WEAPON_PREFLIGHT_LOG%
  pause
  exit /b 18
)

echo.
if "%IS_ACCEPTANCE%"=="1" (
  echo [3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets...
  if not exist "%PRODUCTION_IMPORT%" (
    echo [STOP] Full production vehicle importer is missing: %PRODUCTION_IMPORT%
    pause
    exit /b 19
  )
  call "%PRODUCTION_IMPORT%"
  if errorlevel 1 (
    echo [STOP] Production model ingest failed or remains incomplete.
    echo Strict acceptance will not accept civilian pickup/proxy turret/proxy BTR geometry as final assets.
    echo Normal gameplay may continue only outside strict acceptance; missing models remain explicit content gaps.
    pause
    exit /b 20
  )
) else (
  echo [3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.
  echo [INFO] Missing exact production models remain visible content gaps; no proxy is called production-ready.
)

echo.
echo [4/4] Launching CURRENT NORMAL GAME frontend...
echo This is the normal TEAM gameplay route, not the Sandbox/Test Range route.
echo Pass 45 renderer mode: %RHI_MODE%
echo Renderer flags: %RHI_FLAGS%
echo Visual profile: NORMAL HIGH, 100%% render scale
echo Thermal recovery cap: 60 FPS
echo D3D12/SM6 stays disabled after confirmed startup renderer crashes.
echo Use START / LOCAL GAME in the game menu to enter the listen-server match.
echo Branch: %CURRENT_BRANCH%
echo Log: %PLAYTEST_LOG%
echo Source: %LOCAL_HEAD%
echo.
echo [PASS45] PASS45_RHI_MODE mode=%RHI_MODE% flags=%RHI_FLAGS%
echo [PASS45] PASS45_NORMAL_VISUAL_QUALITY scale=100 view=3 shadow=2 texture=3 effects=3 foliage=2 post=3 aa=3 shading=3 gi=2 reflection=2 landscape=3
start /wait "Oster Conflict Current Gameplay" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend %RHI_FLAGS% -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -fullscreen -ResX=1600 -ResY=900 -ExecCmds="%QUALITY_CMDS%" -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

if not "%GAME_RC%"=="0" (
  echo.
  echo [CRASH-DIAGNOSTICS] Unreal exited with code %GAME_RC%.
  echo [CRASH-DIAGNOSTICS] Relevant frontend markers:
  if exist "%PLAYTEST_LOG%" findstr /C:"PASS45_" /C:"PASS44_" /C:"PASS43_" /C:"PASS29_" /C:"PASS28_" /C:"PASS27_" /C:"PASS26_" /C:"PASS25_" /C:"PASS24_" /C:"R13 frontend:" "%PLAYTEST_LOG%"
  echo [CRASH-DIAGNOSTICS] Last 180 gameplay-log lines:
  if exist "%PLAYTEST_LOG%" powershell -NoProfile -Command "Get-Content -LiteralPath $env:PLAYTEST_LOG -Tail 180"
)

if "%IS_ACCEPTANCE%"=="1" (
  echo.
  echo [ACCEPTANCE] Inspecting runtime evidence from the exact playtest source...
  if not exist "%PLAYTEST_LOG%" (
    echo [STOP] Acceptance log is missing: %PLAYTEST_LOG%
    pause
    exit /b 21
  )

  findstr /C:"PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL" "%PLAYTEST_LOG%" >nul
  if not errorlevel 1 (
    echo [STOP] Production vehicle runtime validation failed. Proxy HMMWV/M2/BTR visuals are not accepted.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 22
  )

  findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL" "%PLAYTEST_LOG%" >nul
  if not errorlevel 1 (
    echo [STOP] Required weapon rack validation failed. Every class needs either its exact production visual or an explicit real-mesh fallback.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 23
  )

  findstr /C:"PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP" "%PLAYTEST_LOG%" >nul
  if not errorlevel 1 (
    echo [STOP] One or more required rack weapon visuals still have missing/default authored materials.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 23
  )

  findstr /C:"PASS7_PRODUCTION_VEHICLES_READY" "%PLAYTEST_LOG%" >nul
  if errorlevel 1 (
    echo [STOP] No production vehicle READY marker was recorded.
    echo Complete the actual gameplay deployment and remain in the runtime long enough for validation.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 24
  )

  findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPONS_READY" "%PLAYTEST_LOG%" >nul
  if errorlevel 1 (
    echo [STOP] No required-available weapon READY marker was recorded for the Museum 11-class rack.
    echo Exact production gaps may use explicit real fallbacks, but primitive-only or missing visuals are not accepted.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 25
  )

  findstr /C:"PASS36_WEAPON_MATERIAL_AUDIT_READY" "%PLAYTEST_LOG%" >nul
  if errorlevel 1 (
    echo [STOP] Rack material audit never reached READY. White/default materials remain a Pass45 failure.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 25
  )

  findstr /C:"PASS7_MUSEUM_BASES_READY" "%PLAYTEST_LOG%" >nul
  if errorlevel 1 (
    echo [STOP] No authoritative Museum BASE readiness marker was recorded.
    echo The acceptance run did not prove the Museum spawn route.
    echo Log: %PLAYTEST_LOG%
    pause
    exit /b 26
  )

  echo [ACCEPTANCE] PASS7_PRODUCTION_VEHICLES_READY found.
  echo [ACCEPTANCE] PASS45_REQUIRED_AVAILABLE_WEAPONS_READY found.
  echo [ACCEPTANCE] PASS36_WEAPON_MATERIAL_AUDIT_READY found.
  echo [ACCEPTANCE] PASS7_MUSEUM_BASES_READY found.
  echo [ACCEPTANCE] Exact weapon payload gaps, if any, remain CONTENT GAP and are not called production-ready.
  echo [ACCEPTANCE] Automated runtime evidence gates passed. Visual/UI checklist still requires direct observation.
)

echo.
echo ============================================================
echo CURRENT GAMEPLAY FINISHED - exit code %GAME_RC%
echo Branch: %CURRENT_BRANCH%
echo Source: %LOCAL_HEAD%
echo RHI mode: %RHI_MODE%
echo Log: %PLAYTEST_LOG%
echo ============================================================
pause
exit /b %GAME_RC%

:quick_normal_game
set "CURRENT_BRANCH=unknown"
set "LOCAL_HEAD=unknown"
where git >nul 2>nul
if not errorlevel 1 (
  for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
  for /f "delims=" %%H in ('git rev-parse HEAD 2^>nul') do set "LOCAL_HEAD=%%H"
  for /f "delims=" %%D in ('git status --porcelain --untracked-files=all 2^>nul') do echo [LOCAL CHANGE] %%D
)

echo.
echo [QUICK NORMAL] Incremental C++ build only. Asset reimport is skipped.
echo [QUICK NORMAL] LFS hydration, weapon commandlet preflight, production vehicle import and acceptance gates are skipped.
echo [QUICK NORMAL] Branch: %CURRENT_BRANCH%
echo [QUICK NORMAL] Source: %LOCAL_HEAD%
echo [QUICK NORMAL] Renderer: %RHI_MODE% ^(%RHI_FLAGS%^)
echo [QUICK NORMAL] Windowed 1600x900, NORMAL HIGH quality, 100%% render scale, max 60 FPS.
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
  echo [ERROR] UE build failed with exit code %BUILD_RC%.
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  exit /b %BUILD_RC%
)

echo.
echo [QUICK NORMAL] Launching current Oster runtime directly. No asset importer runs before this process.
echo [PASS45] PASS45_NORMAL_VISUAL_QUALITY scale=100 view=3 shadow=2 texture=3 effects=3 foliage=2 post=3 aa=3 shading=3 gi=2 reflection=2 landscape=3
start /wait "Oster Conflict Quick Normal" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend %RHI_FLAGS% -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -windowed -ResX=1600 -ResY=900 -ExecCmds="%QUALITY_CMDS%" -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"
echo.
echo ============================================================
echo QUICK NORMAL FINISHED - exit code %GAME_RC%
echo Branch: %CURRENT_BRANCH%
echo Source: %LOCAL_HEAD%
echo RHI mode: %RHI_MODE%
echo Log: %PLAYTEST_LOG%
echo Runtime acceptance: NOT RUN
 echo ============================================================
exit /b %GAME_RC%
