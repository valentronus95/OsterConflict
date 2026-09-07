@echo off
setlocal
cd /d "%~dp0..\.."

echo [Oster Conflict] PASS45 GAMEPLAY CONTENT GATE

echo [1/3] Acquire and inventory gameplay content
call "OsterConflict\Scripts\PREPARE_GAMEPLAY_CONTENT_58.cmd"
if errorlevel 1 (
  echo.
  echo [FAIL] Content acquisition/inventory failed.
  pause
  exit /b 1
)

echo.
echo [2/3] Verify acquisition and provenance source gate
python VERIFY_PASS45_GAMEPLAY_CONTENT_INTAKE.py
if errorlevel 1 (
  echo.
  echo [FAIL] PASS45 gameplay content acquisition gate failed.
  pause
  exit /b 1
)

echo.
echo [3/3] Verify that acquired content is actually integrated or explicitly excluded
python VERIFY_PASS45_GAMEPLAY_CONTENT_INTEGRATION.py
set "INTEGRATION_RC=%ERRORLEVEL%"
if "%INTEGRATION_RC%"=="2" (
  echo.
  echo [PENDING] Assets are downloaded/inventoried but integration is not complete.
  echo Continue PASS45_GAMEPLAY_CONTENT_INTAKE_TZ_2026-08-28.md using PASS45_CONTENT_INTEGRATION_LEDGER.csv.
  echo Do not mark the content gate complete and do not merge on acquisition alone.
  pause
  exit /b 2
)
if not "%INTEGRATION_RC%"=="0" (
  echo.
  echo [FAIL] PASS45 gameplay content integration gate failed.
  pause
  exit /b %INTEGRATION_RC%
)

echo.
echo [PASS] Acquisition and integration ledger gates are ready.
echo Final UE 5.8 runtime visual/audio/performance acceptance remains mandatory.
pause
exit /b 0
