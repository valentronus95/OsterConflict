@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "BASE_ACCEPTANCE=%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

if not exist "%BASE_ACCEPTANCE%" (
  echo [STOP] Base Museum/FPS recovery launcher is missing: %BASE_ACCEPTANCE%
  exit /b 2
)

echo ============================================================
echo OSTER CONFLICT - PASS 21 LANDMARK OWNERSHIP ACCEPTANCE
echo ============================================================
echo This runs the normal frontend/Museum/FPS recovery path first.
echo Stay in gameplay for at least 15 seconds before exiting.
echo Pass 21 then verifies that current Museum/Silpo/Culture owners are unique
 echo and anchored to their canonical sites after all historical delayed stages.
echo.

call "%BASE_ACCEPTANCE%"
set "BASE_RC=%ERRORLEVEL%"
if not "%BASE_RC%"=="0" (
  echo [STOP] Base runtime recovery acceptance failed with code %BASE_RC%.
  exit /b %BASE_RC%
)

if not exist "%LOG%" (
  echo [STOP] Runtime log is missing: %LOG%
  exit /b 20
)

findstr /C:"PASS21_LANDMARK_OWNERSHIP_FAIL" "%LOG%" >nul
if not errorlevel 1 (
  echo [STOP] Landmark shell ownership validation failed.
  findstr /C:"PASS21_LANDMARK_OWNERSHIP_FAIL" /C:"PASS21_LANDMARK_DUPLICATE_REPAIRED" "%LOG%"
  exit /b 21
)

findstr /C:"PASS21_LANDMARK_OWNERSHIP_READY" "%LOG%" >nul
if errorlevel 1 (
  echo [STOP] No Pass 21 landmark ownership readiness marker was recorded.
  echo Stay in gameplay at least 15 seconds so the historical startup window can close.
  exit /b 22
)

echo.
echo ============================================================
echo PASS 21 LANDMARK OWNERSHIP: AUTOMATED EVIDENCE PASSED
echo ============================================================
findstr /C:"PASS21_LANDMARK_DUPLICATE_REPAIRED" /C:"PASS21_LANDMARK_OWNERSHIP_READY" "%LOG%"
echo.
echo This proves final runtime ownership/count/anchor geometry only.
echo Photo fidelity and exact facade appearance still require visual inspection.
exit /b 0
