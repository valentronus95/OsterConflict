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
echo OSTER CONFLICT - PASS 21 / PASS45 LANDMARK OWNERSHIP ACCEPTANCE
echo ============================================================
echo This runs the focused Museum/FPS recovery path first.
echo Stay in gameplay for at least 15 seconds before exiting.
echo Current ownership is validation-only: no duplicate repair owner is allowed.
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

for %%F in (
  PASS45_MUSEUM_LAYER_VALIDATION_FAIL
  PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL
  PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL
  PASS45_SILPO_IDENTITY_VALIDATION_FAIL
  PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL
) do (
  findstr /C:"%%F" "%LOG%" >nul
  if not errorlevel 1 (
    echo [STOP] Current landmark ownership validation failed: %%F
    findstr /C:"%%F" "%LOG%"
    exit /b 21
  )
)

for %%M in (
  PASS45_LANDMARK_STARTUP_COORDINATED_READY
  PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED
  PASS45_MUSEUM_R138_COLLISION_ONLY_READY
  PASS45_MUSEUM_LAYER_VALIDATION_READY
  PASS45_LANDMARK_SEPARATION_VALIDATION_READY
  PASS45_LANDMARK_IDENTITY_VALIDATION_READY
  PASS45_SILPO_IDENTITY_VALIDATION_READY
) do (
  findstr /C:"%%M" "%LOG%" >nul
  if errorlevel 1 (
    echo [STOP] Missing current landmark ownership evidence: %%M
    echo Log: %LOG%
    exit /b 22
  )
)

echo.
echo ============================================================
echo PASS 21 / PASS45 LANDMARK OWNERSHIP: AUTOMATED EVIDENCE PASSED
echo ============================================================
findstr /C:"PASS45_LANDMARK_STARTUP_COORDINATED_READY" /C:"PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED" /C:"PASS45_MUSEUM_R138_COLLISION_ONLY_READY" /C:"PASS45_MUSEUM_LAYER_VALIDATION_READY" /C:"PASS45_LANDMARK_SEPARATION_VALIDATION_READY" /C:"PASS45_LANDMARK_IDENTITY_VALIDATION_READY" /C:"PASS45_SILPO_IDENTITY_VALIDATION_READY" "%LOG%"
echo.
echo This proves runtime Museum/R14.0 Silpo/Culture owner separation and validation only.
echo Photo fidelity and exact facade appearance still require visual inspection.
exit /b 0