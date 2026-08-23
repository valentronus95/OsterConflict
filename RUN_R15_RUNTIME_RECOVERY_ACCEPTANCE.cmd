@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "VERIFY=%~dp0VERIFY_RUNTIME_RECOVERY_PASS_15.py"
set "VERIFY16=%~dp0VERIFY_RUNTIME_GRAPHICS_PASS_16.py"
set "LOG_DIR=%~dp0Logs"
set "LOG=%LOG_DIR%\R14_CURRENT_GAMEPLAY.log"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%LOG%" del /q "%LOG%" >nul 2>nul

if not exist "%BUILD_BAT%" (
  echo [STOP] UE 5.8 Build.bat not found: %BUILD_BAT%
  pause
  exit /b 2
)
if not exist "%EDITOR%" (
  echo [STOP] UnrealEditor.exe not found: %EDITOR%
  pause
  exit /b 3
)
if not exist "%PROJECT%" (
  echo [STOP] Project not found: %PROJECT%
  pause
  exit /b 4
)

where git >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git not found in PATH.
  pause
  exit /b 5
)

for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if /I "%CURRENT_BRANCH%"=="main" (
  set "REMOTE_REF=origin/main"
) else (
  echo(%CURRENT_BRANCH%| findstr /B /I /C:"fix/runtime-acceptance-" /C:"fix/single-launcher-" >nul
  if errorlevel 1 (
    echo [STOP] Focused recovery launcher accepts only main or explicit runtime-fix branches.
    pause
    exit /b 6
  )
  set "REMOTE_REF=origin/%CURRENT_BRANCH%"
)

echo [1/6] Fetching exact branch...
git fetch origin "%CURRENT_BRANCH%"
if errorlevel 1 exit /b 7
for /f "delims=" %%H in ('git rev-parse HEAD') do set "LOCAL_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse "%REMOTE_REF%"') do set "REMOTE_HEAD=%%H"
if /I not "%LOCAL_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local source is stale.
  echo Local : %LOCAL_HEAD%
  echo GitHub: %REMOTE_HEAD%
  echo Pull origin, then run START_HERE.cmd again.
  pause
  exit /b 8
)

for /f "delims=" %%D in ('git status --porcelain --untracked-files=all') do echo [LOCAL CHANGE] %%D

echo [2/6] Hydrating Git LFS assets...
git lfs pull origin
if errorlevel 1 exit /b 9
git lfs checkout >nul
if errorlevel 1 exit /b 9

set "PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)
if not defined PY_CMD (
  echo [STOP] Python 3 not found.
  pause
  exit /b 10
)

echo [3/6] Verifying Pass 15 + Pass 16 source contracts...
%PY_CMD% "%VERIFY%"
if errorlevel 1 (
  echo [STOP] Pass 15 source verification failed.
  pause
  exit /b 11
)
if not exist "%VERIFY16%" (
  echo [STOP] Pass 16 graphics verifier is missing: %VERIFY16%
  pause
  exit /b 11
)
%PY_CMD% "%VERIFY16%"
if errorlevel 1 (
  echo [STOP] Pass 16 graphics verification failed.
  pause
  exit /b 11
)

echo [4/6] Building OsterConflictEditor...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
if errorlevel 1 (
  echo [STOP] UE build failed.
  pause
  exit /b 12
)

echo.
echo [5/6] Launching focused runtime recovery test...
echo ------------------------------------------------------------
echo Safe renderer: DirectX 11 ^(-d3d11^).
echo D3D12 is temporarily disabled after the confirmed D3D12RHI startup crash.
echo 1. Main menu - press START.
echo 2. Confirm server fields are DARK/readable and panel is opaque.
echo 3. Press CREATE SERVER.
echo 4. Complete TEAM - SQUAD - ROLE - SPAWN - У БІЙ.
echo 5. You MUST appear beside Museum and see 11 PLAYABLE real-mesh weapons nearby.
echo 6. Stay in gameplay at least 15 seconds for GPU/RHI + FPS evidence.
echo 7. Exit the game. This launcher checks the log automatically.
echo.
echo NOTE: exact production-art certification and BTR/HMMWV/M2 production intake are separate strict gates.
echo ------------------------------------------------------------

start /wait "Oster Conflict Pass 15-19 Recovery" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend -d3d11 -NoScreenMessages -log -abslog="%LOG%" -windowed -ResX=1600 -ResY=900 -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

if not exist "%LOG%" (
  echo [STOP] Runtime log missing: %LOG%
  pause
  exit /b 20
)

echo [6/6] Inspecting focused runtime evidence...
for %%M in (
  PASS15_FRONTEND_FIELDS_OPAQUE_READY
  PASS14_MAIN_START_OPENS_SERVER_SETUP
  PASS14_HOST_TRAVEL_BEGIN
  PASS14_FRONTEND_TRAVEL_HANDOFF_READY
  PASS15_MUSEUM_BASES_WEAPONS_READY
  PASS19_PLAYABLE_WEAPON_SET_READY
  PASS16_RUNTIME_GRAPHICS_IDENTITY
  PASS15_PERF_SAMPLE
) do (
  findstr /C:"%%M" "%LOG%" >nul
  if errorlevel 1 (
    echo [STOP] Missing runtime evidence: %%M
    echo Log: %LOG%
    pause
    exit /b 21
  )
)

findstr /C:"PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] BASE deployment recovery failed to find a primary Museum BASE.
  pause
  exit /b 22
)

findstr /C:"PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM" /C:"PASS15_BASE_DEPLOYMENT_RECOVERED" "%LOG%" >nul
if errorlevel 1 (
  echo [STOP] No evidence that the actual player pawn deployed at Museum BASE.
  pause
  exit /b 23
)

findstr /C:"PASS19_PLAYABLE_WEAPON_SET_FAIL" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] The Museum rack contains missing/primitive-only weapon visuals.
  findstr /C:"PASS19_PLAYABLE_WEAPON_SET_FAIL" "%LOG%"
  pause
  exit /b 24
)

findstr /C:"PASS15_MUSEUM_BASES_WEAPONS_NOT_READY" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Museum BASE or physical 11-weapon rack did not become ready.
  pause
  exit /b 25
)

findstr /C:"PASS15_PERF_BELOW_TARGET" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Gameplay is still below the 30 FPS recovery target.
  findstr /C:"PASS16_RUNTIME_GRAPHICS_IDENTITY" /C:"PASS15_PERF_PROBE" /C:"PASS15_EMERGENCY_PERF_PROFILE_APPLIED" /C:"PASS15_PERF_SAMPLE" /C:"PASS15_PERF_BELOW_TARGET" "%LOG%"
  pause
  exit /b 26
)

findstr /C:"PASS15_PERF_30FPS_READY" "%LOG%" >nul
if errorlevel 1 (
  echo [STOP] No 30 FPS readiness marker was recorded.
  pause
  exit /b 27
)

echo.
echo ============================================================
echo PASS 15-19 FOCUSED RUNTIME RECOVERY: AUTOMATED EVIDENCE PASSED
echo Source: %LOCAL_HEAD%
echo ============================================================
findstr /C:"PASS16_RUNTIME_GRAPHICS_IDENTITY" "%LOG%"
findstr /C:"PASS19_PLAYABLE_WEAPON_SET_READY" "%LOG%"
findstr /C:"PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM" /C:"PASS15_BASE_DEPLOYMENT_RECOVERED" "%LOG%"
findstr /C:"PASS15_PERF_PROBE" /C:"PASS15_EMERGENCY_PERF_PROFILE_APPLIED" /C:"PASS15_PERF_SAMPLE" /C:"PASS15_PERF_30FPS_READY" "%LOG%"
echo.
echo Exact production-art readiness is intentionally NOT certified by this focused recovery launcher.
echo Manual visual confirmation still required for exact Museum appearance and weapon placement.
pause
exit /b %GAME_RC%
