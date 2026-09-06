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
echo OSTER CONFLICT - WORLD GEOMETRY STABILITY PASS 12
echo ============================================================
echo This run separates late world rebuild/mutation from pure render shimmer.
echo Stay in actual gameplay for at least 21 seconds before exiting.
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
  exit /b 60
)

findstr /C:"PASS12_WORLD_GEOMETRY_STABILITY_FAIL" "%PLAYTEST_LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Late world-geometry mutation was detected.
  echo Search the log for PASS12_WORLD_GEOMETRY_STABILITY_FAIL to see the exact family/count change.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 61
)

findstr /C:"PASS12_WORLD_GEOMETRY_STABLE" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Stable-world marker was not recorded.
  echo Remain in gameplay for at least 21 seconds so baseline 12s + samples 16s/20s can complete.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 62
)

echo [PASS] PASS12_WORLD_GEOMETRY_STABLE found.
echo [PASS] Tracked source geometry did not rebuild or change instance counts after startup.
echo [NOTE] If distant shimmer is still visible with this gate green, the remaining defect is render/z-fighting/LOD rather than late geometry mutation.
pause
exit /b 0
