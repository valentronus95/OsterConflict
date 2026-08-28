@echo off
setlocal
cd /d "%~dp0..\.."

echo [Oster Conflict] PASS45 GAMEPLAY CONTENT GATE

echo [1/2] Acquire and inventory gameplay content
call "OsterConflict\Scripts\PREPARE_GAMEPLAY_CONTENT_58.cmd"
if errorlevel 1 (
  echo.
  echo [FAIL] Content acquisition/inventory failed.
  pause
  exit /b 1
)

echo.
echo [2/2] Verify PASS45 gameplay content intake source gate
python VERIFY_PASS45_GAMEPLAY_CONTENT_INTAKE.py
if errorlevel 1 (
  echo.
  echo [FAIL] PASS45 gameplay content source gate failed.
  pause
  exit /b 1
)

echo.
echo [PASS] PASS45 gameplay content source gate is ready.
echo Runtime migration/integration/visual acceptance is still required by PASS45_GAMEPLAY_CONTENT_INTAKE_TZ_2026-08-28.md.
pause
exit /b 0
