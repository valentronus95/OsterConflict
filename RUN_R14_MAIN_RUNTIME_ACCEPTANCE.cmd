@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "OC_FORCE_ACCEPTANCE=1"
echo ============================================================
echo OSTER CONFLICT - STRICT MAIN RUNTIME ACCEPTANCE
echo ============================================================
echo This route builds the current source, launches the normal frontend,
echo then rejects the run unless Museum BASE, real weapon rack and

echo production HMMWV/M2/BTR runtime evidence is present in the log.
echo.

call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - exit code %RC%
  exit /b %RC%
)

echo.
echo [ACCEPTANCE] AUTOMATED RUNTIME EVIDENCE GATES PASSED.
exit /b 0
