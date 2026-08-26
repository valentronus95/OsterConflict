@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "OC_FORCE_ACCEPTANCE=1"
set "PLAYFLOW=%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
set "EVIDENCE_OUT=%~dp0Logs\PASS45_RUNTIME_ACCEPTANCE_EVIDENCE.txt"

echo ============================================================
echo OSTER CONFLICT - STRICT PASS45 MAIN RUNTIME ACCEPTANCE
echo ============================================================
echo This route executes the normal game exactly once through the playflow/performance wrapper,
echo then applies Pass45 material/dependency and interaction evidence gates.
echo RUN_R14_CURRENT_GAMEPLAY.cmd remains the only process that launches gameplay.
echo A log-only PASS still does NOT replace the required visual/screenshots acceptance.
echo.

if not exist "%PLAYFLOW%" (
  echo [ACCEPTANCE] FAILED - playflow/performance wrapper is missing: %PLAYFLOW%
  exit /b 2
)
if not exist "%CURRENT_GAMEPLAY%" (
  echo [ACCEPTANCE] FAILED - normal gameplay launcher is missing: %CURRENT_GAMEPLAY%
  exit /b 2
)
if not exist "%MATERIAL_GATE%" (
  echo [ACCEPTANCE] FAILED - strict material gate is missing: %MATERIAL_GATE%
  exit /b 3
)
if not exist "%EVIDENCE_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 evidence verifier is missing: %EVIDENCE_VERIFY%
  exit /b 4
)

call "%PLAYFLOW%"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - playflow/performance runtime gate exit code %RC%
  exit /b %RC%
)

echo.
echo [ACCEPTANCE] Running strict authored material/dependency gate on the imported current assets...
call "%MATERIAL_GATE%"
set "MATERIAL_RC=%ERRORLEVEL%"
if not "%MATERIAL_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 strict material gate exit code %MATERIAL_RC%
  exit /b %MATERIAL_RC%
)

set "PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)
if not defined PY_CMD (
  echo [ACCEPTANCE] FAILED - Python 3 not found in PATH.
  exit /b 30
)

set "PASS45_SOURCE_SHA=unknown"
for /f "delims=" %%H in ('git rev-parse HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
set "PASS45_SOURCE_SHA=%PASS45_SOURCE_SHA%"

echo.
echo [ACCEPTANCE] Verifying Pass45 interaction/material evidence from the exact run...
%PY_CMD% "%EVIDENCE_VERIFY%" "%GAMEPLAY_LOG%" "%MATERIAL_LOG%" "%WEAPON_REPORT%"
set "EVIDENCE_RC=%ERRORLEVEL%"
if not "%EVIDENCE_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 evidence is incomplete.
  echo The strict run must actually exercise driver enter/exit and M2 gunner aim/exit.
  echo Evidence: %EVIDENCE_OUT%
  exit /b %EVIDENCE_RC%
)

echo.
echo ============================================================
echo [ACCEPTANCE] PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED.
echo [ACCEPTANCE] Source: %PASS45_SOURCE_SHA%
echo [ACCEPTANCE] Evidence: %EVIDENCE_OUT%
echo [ACCEPTANCE] Exact weapon payload gaps remain CONTENT GAP unless real production content is later supplied.
echo [ACCEPTANCE] VISUAL ACCEPTANCE IS STILL PENDING direct screenshots/observation.
echo ============================================================
exit /b 0
