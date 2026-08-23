@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "BASE_LAUNCHER=%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
set "VERIFY17=%~dp0VERIFY_RUNTIME_PERFORMANCE_PASS_17.py"
set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

if not exist "%BASE_LAUNCHER%" (
  echo [STOP] Base Pass 15-16 launcher is missing: %BASE_LAUNCHER%
  pause
  exit /b 2
)
if not exist "%VERIFY17%" (
  echo [STOP] Pass 17 verifier is missing: %VERIFY17%
  pause
  exit /b 3
)

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
  exit /b 4
)

echo [1/3] Verifying Pass 17 world performance source contract...
%PY_CMD% "%VERIFY17%"
if errorlevel 1 (
  echo [STOP] Pass 17 source verification failed.
  pause
  exit /b 5
)

echo.
echo [2/3] Running full Pass 15-16 focused runtime acceptance...
echo This still verifies frontend, Museum BASE, 11 production weapon visuals, real GPU/RHI and measured 30 FPS.
call "%BASE_LAUNCHER%"
set "BASE_RC=%ERRORLEVEL%"
if not "%BASE_RC%"=="0" (
  echo [STOP] Base Pass 15-16 runtime acceptance failed with code %BASE_RC%.
  exit /b %BASE_RC%
)

if not exist "%LOG%" (
  echo [STOP] Runtime log missing after base acceptance: %LOG%
  pause
  exit /b 20
)

echo [3/3] Verifying Pass 17 world render budget evidence...
findstr /C:"PASS17_WORLD_ISM_BUDGET_NOT_APPLIED" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Pass 17 world render budget failed to attach to AOCWorldSectorOster.
  findstr /C:"PASS17_WORLD_ISM_BUDGET_NOT_APPLIED" "%LOG%"
  pause
  exit /b 21
)

findstr /C:"PASS17_WORLD_ISM_BUDGET_READY" "%LOG%" >nul
if errorlevel 1 (
  echo [STOP] Missing Pass 17 render budget evidence.
  echo The FPS result is not accepted unless the ISM culling/shadow budget actually ran.
  pause
  exit /b 22
)

echo.
echo ============================================================
echo PASS 17 RUNTIME PERFORMANCE ACCEPTANCE: PASSED
echo ============================================================
findstr /C:"PASS16_RUNTIME_GRAPHICS_IDENTITY" "%LOG%"
findstr /C:"PASS17_WORLD_ISM_BUDGET_READY" "%LOG%"
findstr /C:"PASS15_PERF_PROBE" /C:"PASS15_EMERGENCY_PERF_PROFILE_APPLIED" /C:"PASS15_PERF_SAMPLE" /C:"PASS15_PERF_30FPS_READY" "%LOG%"
echo.
echo This is automated runtime evidence. Visual popping/cull transitions still need human inspection during the run.
pause
exit /b 0
