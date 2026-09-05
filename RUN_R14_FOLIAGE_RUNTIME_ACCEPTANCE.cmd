@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "BASE_ACCEPTANCE=%~dp0RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
set "PLAYTEST_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

if not exist "%BASE_ACCEPTANCE%" (
  echo [ERROR] Base strict runtime acceptance launcher is missing: %BASE_ACCEPTANCE%
  pause
  exit /b 2
)

echo ============================================================
echo OSTER CONFLICT - FOLIAGE RUNTIME ACCEPTANCE PASS 10 / PASS45
echo ============================================================
echo This run proves that source-only cube ground-cover proxies are
echo physically destroyed, imported regional trees are actually wired,
echo and real dense foliage owns visible gameplay grass.
echo.

call "%BASE_ACCEPTANCE%"
set "BASE_RC=%ERRORLEVEL%"
if not "%BASE_RC%"=="0" (
  echo [STOP] Base runtime acceptance failed with code %BASE_RC%.
  exit /b %BASE_RC%
)

if not exist "%PLAYTEST_LOG%" (
  echo [STOP] Runtime log is missing: %PLAYTEST_LOG%
  pause
  exit /b 40
)

findstr /C:"PASS10_FOLIAGE_RUNTIME_FAIL" "%PLAYTEST_LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Foliage runtime validation reported FAIL.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 41
)

findstr /C:"PASS45_REGIONAL_TREE_INTAKE_FAIL" "%PLAYTEST_LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Imported HillTree/ScotsPine runtime intake reported FAIL.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 45
)

findstr /C:"PASS45_REGIONAL_TREE_INTAKE_WIRED" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Imported HillTree/ScotsPine runtime intake was not proved wired.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 46
)

findstr /C:"PASS45_GROUND_COVER_PRIMITIVES_DESTROYED" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] GrassMown/GrassRough/GrassWetland proxy components were not proved destroyed.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 42
)

findstr /C:"PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Developer reference markers/text labels were not proved destroyed.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 44
)

findstr /C:"PASS10_FOLIAGE_RUNTIME_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Dense foliage READY marker was not recorded.
  echo Enter actual gameplay and remain in the world for at least 25 seconds.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 43
)

echo [PASS] PASS45_REGIONAL_TREE_INTAKE_WIRED found.
echo [PASS] PASS45_GROUND_COVER_PRIMITIVES_DESTROYED found.
echo [PASS] PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED found.
echo [PASS] PASS10_FOLIAGE_RUNTIME_READY found.
echo [PASS] Runtime uses imported regional tree families plus real DenseGrass HISM ground cover.
echo [NOTE] Gate K remains open while other BasicShape/proxy core families still exist.
echo [NOTE] Shimmer/density still requires direct visual observation while moving the camera.
pause
exit /b 0
