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
echo OSTER CONFLICT - ROAD PROFILE RUNTIME ACCEPTANCE PASS 11
echo ============================================================
echo This run proves that the old raised S16A road/sidewalk cube profile
 echo is normalized without changing the road network XY topology.
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
  exit /b 50
)

findstr /C:"PASS11_ROAD_PROFILE_RUNTIME_FAIL" "%PLAYTEST_LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Road profile runtime validation reported FAIL.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 51
)

findstr /C:"PASS11_ROAD_PROFILE_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Road profile READY marker was not recorded.
  echo Enter actual gameplay and remain in the world long enough for validation.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 52
)

echo [PASS] PASS11_ROAD_PROFILE_READY found.
echo [PASS] Roads are limited to a 4 cm surface profile and sidewalks to an 8 cm curb profile.
echo [NOTE] Drive/walk across several intersections and visually confirm there is no raised/convex road appearance.
pause
exit /b 0
