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
echo OSTER CONFLICT - STADION OSTER RUNTIME ACCEPTANCE PASS 9
echo ============================================================
echo This first runs the full main runtime acceptance gate, then
 echo requires the Stadion Oster runtime evidence marker from the same UE run.
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
  exit /b 30
)

findstr /C:"PASS9_STADION_OSTER_RUNTIME_FAIL" "%PLAYTEST_LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Stadion Oster runtime validation reported FAIL.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 31
)

findstr /C:"PASS9_STADION_OSTER_READY" "%PLAYTEST_LOG%" >nul
if errorlevel 1 (
  echo [STOP] Stadion Oster READY marker was not recorded.
  echo Enter actual gameplay and remain in the world long enough for runtime validation.
  echo Log: %PLAYTEST_LOG%
  pause
  exit /b 32
)

echo [PASS] PASS9_STADION_OSTER_READY found.
echo [PASS] Stadion Oster source ownership, runtime component presence,
echo        georeferenced XY, terrain Z and legacy-visual handoff were proved.
echo [NOTE] Photo fidelity still requires direct visual comparison in UE.
pause
exit /b 0
