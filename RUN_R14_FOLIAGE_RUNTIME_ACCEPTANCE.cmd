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
echo OSTER CONFLICT - FOLIAGE RUNTIME ACCEPTANCE PASS 10
echo ============================================================
echo This run proves that source-only cube ground-cover proxies are
 echo retired and real dense foliage owns visible gameplay grass.
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

findstr /C:"PASS10_GROUND_COVER_PROXY_RETIRED" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Source-only GrassMown/GrassRough/GrassWetland proxies were not proved retired.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 42
)

findstr /C:"PASS10_FOLIAGE_RUNTIME_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Dense foliage READY marker was not recorded.
  echo Enter actual gameplay and remain in the world for at least 25 seconds.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 43
)

echo [PASS] PASS10_GROUND_COVER_PROXY_RETIRED found.
echo [PASS] PASS10_FOLIAGE_RUNTIME_READY found.
echo [PASS] Runtime uses real DenseGrass HISM ground cover instead of source-only cube slabs.
echo [NOTE] Shimmer/density still requires direct visual observation while moving the camera.
pause
exit /b 0
