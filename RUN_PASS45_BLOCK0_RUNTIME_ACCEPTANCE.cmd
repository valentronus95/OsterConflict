@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul
cd /d "%~dp0"

set "EXPECTED_BRANCH=fix/pass45-runtime-rejection-material-closure-20260826"
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "SOURCE_VERIFY=%~dp0VERIFY_PASS45_BLOCK0_GROUND_FOUNDATION.py"
set "FOLIAGE_SOURCE_VERIFY=%~dp0VERIFY_FOLIAGE_RUNTIME_PASS_10.py"
set "LOG_DIR=%~dp0Logs"
set "PLAYTEST_LOG=%LOG_DIR%\PASS45_BLOCK0_RUNTIME.log"
set "SOURCE_VERIFY_LOG=%LOG_DIR%\PASS45_BLOCK0_SOURCE_VERIFY.log"
set "SCREENSHOT_ROOT=%~dp0OsterConflict\Saved\Screenshots"
set "SESSION_MARKER=%TEMP%\oster_pass45_block0_session.marker"
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1?LocationTest=1"
set "RHI_FLAGS=-d3d11 -sm5 -nohdr"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PLAYTEST_LOG%" del /q "%PLAYTEST_LOG%" >nul 2>nul
if exist "%SOURCE_VERIFY_LOG%" del /q "%SOURCE_VERIFY_LOG%" >nul 2>nul

where git >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git is not available in PATH.
  exit /b 2
)
where powershell >nul 2>nul
if errorlevel 1 (
  echo [STOP] PowerShell is required for exact-session evidence collection.
  exit /b 3
)
if not exist "%BUILD_BAT%" (
  echo [STOP] UE 5.8 Build.bat not found: %BUILD_BAT%
  exit /b 4
)
if not exist "%EDITOR%" (
  echo [STOP] UnrealEditor.exe not found: %EDITOR%
  exit /b 5
)
if not exist "%PROJECT%" (
  echo [STOP] Project not found: %PROJECT%
  exit /b 6
)
if not exist "%SOURCE_VERIFY%" (
  echo [STOP] Block0 ground source verifier is missing: %SOURCE_VERIFY%
  exit /b 7
)
if not exist "%FOLIAGE_SOURCE_VERIFY%" (
  echo [STOP] Block0 foliage source verifier is missing: %FOLIAGE_SOURCE_VERIFY%
  exit /b 29
)

for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if /I not "%CURRENT_BRANCH%"=="%EXPECTED_BRANCH%" (
  echo [STOP] Block0 acceptance must run from the canonical PASS45 branch.
  echo Current : %CURRENT_BRANCH%
  echo Expected: %EXPECTED_BRANCH%
  exit /b 8
)

for /f "delims=" %%D in ('git status --porcelain --untracked-files=all') do set "DIRTY_TREE=1"
if defined DIRTY_TREE (
  echo [STOP] Working tree is not clean. Exact-head evidence would be ambiguous.
  git status --short
  exit /b 9
)

echo [PRECHECK] Fetching the canonical PASS45 branch...
git fetch origin "%CURRENT_BRANCH%"
if errorlevel 1 (
  echo [STOP] Could not fetch origin/%CURRENT_BRANCH%.
  exit /b 10
)
for /f "delims=" %%H in ('git rev-parse HEAD') do set "TESTED_HEAD=%%H"
for /f "delims=" %%H in ('git rev-parse "origin/%CURRENT_BRANCH%"') do set "REMOTE_HEAD=%%H"
if /I not "%TESTED_HEAD%"=="%REMOTE_HEAD%" (
  echo [STOP] Local PASS45 head is stale. Pull current branch before testing.
  echo Local : %TESTED_HEAD%
  echo GitHub: %REMOTE_HEAD%
  exit /b 11
)
set "HEAD_SHORT=%TESTED_HEAD:~0,12%"

echo [ASSETS] Hydrating Git LFS payloads for the exact tested head...
git lfs version >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git LFS is not installed or not available in PATH.
  exit /b 12
)
git lfs install >nul 2>nul
git lfs pull origin
if errorlevel 1 (
  echo [STOP] Git LFS pull failed.
  exit /b 13
)
git lfs checkout >nul
if errorlevel 1 (
  echo [STOP] Git LFS checkout failed.
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
  echo [STOP] Python 3 was not found in PATH.
  exit /b 15
)

echo [SOURCE] Verifying complete Block0 ground/grass/tree handoff contracts...
%PY_CMD% "%SOURCE_VERIFY%" > "%SOURCE_VERIFY_LOG%" 2>&1
set "VERIFY_RC=%ERRORLEVEL%"
if not "%VERIFY_RC%"=="0" (
  type "%SOURCE_VERIFY_LOG%"
  echo [STOP] Block0 ground/spatial source gate failed. UE runtime test cancelled.
  exit /b 16
)
%PY_CMD% "%FOLIAGE_SOURCE_VERIFY%" >> "%SOURCE_VERIFY_LOG%" 2>&1
set "FOLIAGE_VERIFY_RC=%ERRORLEVEL%"
type "%SOURCE_VERIFY_LOG%"
if not "%FOLIAGE_VERIFY_RC%"=="0" (
  echo [STOP] Block0 foliage/tree source gate failed. UE runtime test cancelled.
  exit /b 30
)

echo [BUILD] Building OsterConflictEditor Win64 Development from %TESTED_HEAD%...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
  echo [STOP] UE 5.8 build failed with exit code %BUILD_RC%.
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  exit /b %BUILD_RC%
)

powershell -NoProfile -Command "Set-Content -LiteralPath '%SESSION_MARKER%' -Value ([DateTime]::UtcNow.ToString('o')) -Encoding ASCII"
if errorlevel 1 (
  echo [STOP] Could not create the session evidence marker.
  exit /b 17
)

echo.
echo ============================================================
echo PASS45 BLOCK0 - LOCAL UE 5.8 VISUAL ACCEPTANCE
ECHO ============================================================
echo Source head: %TESTED_HEAD%
echo.
echo Stay in the playable world for at least 30 seconds, then capture with F9:
echo   1. Museum / central-sector ground context
echo   2. Central park
echo   3. College / urban lawn context
echo   4. Ordinary roadside / private-sector context
echo   5. Long sightline showing grass-ground LOD transition
echo.
echo Do NOT judge Museum, weapons, grenades, vehicles or BTR in this run.
echo Block0 is the only active content block.
echo Close the game after the five screenshots are captured.
echo ============================================================
echo.

start /wait "Oster Conflict PASS45 Block0" "%EDITOR%" "%PROJECT%" "%VISUAL_MAP%" -game -NoFrontend %RHI_FLAGS% -NoScreenMessages -log -abslog="%PLAYTEST_LOG%" -fullscreen -ResX=1600 -ResY=900 -ExecCmds="t.MaxFPS 60" -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"
if not "%GAME_RC%"=="0" (
  echo [STOP] Unreal exited with code %GAME_RC%.
  if exist "%PLAYTEST_LOG%" powershell -NoProfile -Command "Get-Content -LiteralPath '%PLAYTEST_LOG%' -Tail 160"
  exit /b %GAME_RC%
)
if not exist "%PLAYTEST_LOG%" (
  echo [STOP] Runtime log is missing: %PLAYTEST_LOG%
  exit /b 18
)

for %%M in (
  PASS45_BLOCK0_PRETICK_GROUND_FAIL
  PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL
  PASS10_FOLIAGE_RUNTIME_FAIL
  PASS45_REGIONAL_TREE_INTAKE_FAIL
) do (
  findstr /C:"%%M" "%PLAYTEST_LOG%" >nul
  if not errorlevel 1 (
    echo [STOP] Runtime fail marker found: %%M
    exit /b 19
  )
)

findstr /C:"PASS45_BLOCK0_PRETICK_GROUND_READY" "%PLAYTEST_LOG%" | findstr /C:"geometry_postcondition=1" | findstr /C:"collision_enabled=1" >nul
if errorlevel 1 (
  echo [STOP] Exact authored-ground geometry/collision READY evidence is missing.
  exit /b 20
)
findstr /C:"PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Full playable-area spatial grass coverage was not proved READY.
  exit /b 21
)
findstr /C:"PASS10_FOLIAGE_RUNTIME_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Dense foliage runtime never reached READY. Remain in gameplay at least 30 seconds.
  exit /b 22
)
findstr /C:"PASS45_REGIONAL_TREE_INTAKE_WIRED" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Imported regional tree intake was not proved wired.
  exit /b 23
)

for /f "delims=" %%T in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd_HHmmss"') do set "STAMP=%%T"
set "EVIDENCE_REL=RUNTIME_EVIDENCE\BLOCK0_%STAMP%_%HEAD_SHORT%"
set "EVIDENCE_DIR=%~dp0%EVIDENCE_REL%"
mkdir "%EVIDENCE_DIR%" >nul 2>nul
copy /y "%PLAYTEST_LOG%" "%EVIDENCE_DIR%\PASS45_BLOCK0_RUNTIME.log" >nul
copy /y "%SOURCE_VERIFY_LOG%" "%EVIDENCE_DIR%\PASS45_BLOCK0_SOURCE_VERIFY.log" >nul

set "SHOT_COUNT=0"
if exist "%SCREENSHOT_ROOT%" (
  for /f "delims=" %%N in ('powershell -NoProfile -Command "$cutoff=(Get-Item -LiteralPath '%SESSION_MARKER%').LastWriteTimeUtc; $dest='%EVIDENCE_DIR%'; $files=@(Get-ChildItem -LiteralPath '%SCREENSHOT_ROOT%' -File -Recurse -ErrorAction SilentlyContinue ^| Where-Object {$_.LastWriteTimeUtc -ge $cutoff}); foreach($f in $files){Copy-Item -LiteralPath $f.FullName -Destination (Join-Path $dest $f.Name) -Force}; $files.Count"') do set "SHOT_COUNT=%%N"
)

> "%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo PASS45 BLOCK0 UE 5.8 EVIDENCE - PENDING VISUAL REVIEW
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo SourceHead=%TESTED_HEAD%
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo Branch=%CURRENT_BRANCH%
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo AutomatedRuntimeMarkers=PASS
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo ScreenshotCount=%SHOT_COUNT%
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo RuntimeAcceptance=PENDING_VISUAL_REVIEW
>>"%EVIDENCE_DIR%\BLOCK0_EVIDENCE_MANIFEST.txt" echo RequiredViews=central_museum;central_park;college_lawn;roadside_private;long_sightline_lod

if %SHOT_COUNT% LSS 5 (
  echo [STOP] Only %SHOT_COUNT% new screenshots were captured. Block0 requires at least 5 exact-session views.
  echo Evidence retained locally: %EVIDENCE_DIR%
  exit /b 24
)

echo [EVIDENCE] Automated runtime markers PASS and %SHOT_COUNT% exact-session screenshots collected.
echo [EVIDENCE] Status remains PENDING_VISUAL_REVIEW. This script never self-declares RUNTIME ACCEPTED.

echo [PUSH] Re-checking remote head before committing evidence...
git fetch origin "%CURRENT_BRANCH%" >nul 2>nul
for /f "delims=" %%H in ('git rev-parse "origin/%CURRENT_BRANCH%"') do set "REMOTE_AFTER=%%H"
if /I not "%REMOTE_AFTER%"=="%TESTED_HEAD%" (
  echo [STOP] GitHub branch advanced during the UE test.
  echo Tested: %TESTED_HEAD%
  echo GitHub: %REMOTE_AFTER%
  echo Evidence is intentionally NOT pushed against a different source head.
  echo Rerun this launcher after pulling current PASS45.
  exit /b 25
)

git add -- "%EVIDENCE_REL%"
if errorlevel 1 (
  echo [STOP] Could not stage Block0 evidence.
  exit /b 26
)
git commit -m "evidence(pass45): capture Block0 UE58 runtime for %HEAD_SHORT%"
if errorlevel 1 (
  echo [STOP] Could not commit Block0 evidence.
  exit /b 27
)
git push origin "%CURRENT_BRANCH%"
if errorlevel 1 (
  echo [STOP] Evidence commit exists locally but push failed. No acceptance state was changed.
  exit /b 28
)

echo.
echo ============================================================
echo [PASS] BLOCK0 AUTOMATED RUNTIME MARKERS + EVIDENCE CAPTURED
ECHO [PASS] Evidence pushed for tested source %TESTED_HEAD%
echo [PENDING] Five screenshots still require factual visual review before Block0 can be RUNTIME ACCEPTED/FROZEN.
echo ============================================================
exit /b 0
