@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "OC_FORCE_ACCEPTANCE=1"
set "PLAYFLOW=%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "GATE_K_VERIFY=%~dp0VERIFY_PASS45_GATE_K_RUNTIME_LOG.py"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "MANUAL_ACTION_VERIFY=%~dp0VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"
set "GRENADE_ANIM_VERIFY=%~dp0VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py"
set "FLASH_VFX_VERIFY=%~dp0VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
set "EVIDENCE_OUT=%~dp0Logs\PASS45_RUNTIME_ACCEPTANCE_EVIDENCE.txt"

echo ============================================================
echo OSTER CONFLICT - STRICT PASS45 MAIN RUNTIME ACCEPTANCE
echo ============================================================
echo This route executes the normal game exactly once through the playflow/performance wrapper,
echo then applies Pass45 Gate K visual truth, material/dependency and interaction evidence gates.
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
if not exist "%GATE_K_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 Gate K verifier is missing: %GATE_K_VERIFY%
  exit /b 4
)
if not exist "%EVIDENCE_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 evidence verifier is missing: %EVIDENCE_VERIFY%
  exit /b 4
)
if not exist "%MANUAL_ACTION_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 manual-action runtime verifier is missing: %MANUAL_ACTION_VERIFY%
  exit /b 4
)
if not exist "%GRENADE_ANIM_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 grenade throw animation verifier is missing: %GRENADE_ANIM_VERIFY%
  exit /b 4
)
if not exist "%FLASH_VFX_VERIFY%" (
  echo [ACCEPTANCE] FAILED - Pass45 flash grenade VFX verifier is missing: %FLASH_VFX_VERIFY%
  exit /b 4
)

where git >nul 2>nul
if errorlevel 1 (
  echo [ACCEPTANCE] FAILED - Git not found in PATH; exact-head runtime evidence cannot be bound.
  exit /b 31
)
set "PASS45_SOURCE_SHA="
for /f "delims=" %%H in ('git rev-parse --verify HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
if not defined PASS45_SOURCE_SHA (
  echo [ACCEPTANCE] FAILED - current Git HEAD could not be resolved before runtime.
  exit /b 31
)
echo [ACCEPTANCE] Exact source HEAD pinned before runtime: %PASS45_SOURCE_SHA%

git diff --quiet --ignore-submodules --
if errorlevel 1 (
  echo [ACCEPTANCE] FAILED - tracked unstaged Changes are present before runtime.
  echo [ACCEPTANCE] Local Changes are preserved. This launcher never resets, cleans, stashes or restores them.
  echo [ACCEPTANCE] Exact-head evidence is refused because the tested tracked bytes would not equal HEAD.
  exit /b 32
)
git diff --cached --quiet --ignore-submodules --
if errorlevel 1 (
  echo [ACCEPTANCE] FAILED - tracked staged Changes are present before runtime.
  echo [ACCEPTANCE] Local Changes are preserved. This launcher never resets, cleans, stashes or restores them.
  echo [ACCEPTANCE] Exact-head evidence is refused because the tested tracked bytes would not equal HEAD.
  exit /b 32
)
echo [ACCEPTANCE] Exact tracked worktree matches pinned HEAD before runtime. Untracked evidence files do not block acceptance.

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

set "PASS45_SOURCE_SHA_AFTER="
for /f "delims=" %%H in ('git rev-parse --verify HEAD 2^>nul') do set "PASS45_SOURCE_SHA_AFTER=%%H"
if not defined PASS45_SOURCE_SHA_AFTER (
  echo [ACCEPTANCE] FAILED - current Git HEAD could not be resolved after runtime/material gates.
  exit /b 31
)
if /I not "%PASS45_SOURCE_SHA_AFTER%"=="%PASS45_SOURCE_SHA%" (
  echo [ACCEPTANCE] FAILED - source HEAD changed during runtime acceptance.
  echo [ACCEPTANCE] Before: %PASS45_SOURCE_SHA%
  echo [ACCEPTANCE] After:  %PASS45_SOURCE_SHA_AFTER%
  exit /b 31
)

git diff --quiet --ignore-submodules --
if errorlevel 1 (
  echo [ACCEPTANCE] FAILED - runtime/import stages changed tracked unstaged content.
  echo [ACCEPTANCE] Exact-head evidence is refused; generated or imported tracked bytes must be committed before acceptance.
  exit /b 32
)
git diff --cached --quiet --ignore-submodules --
if errorlevel 1 (
  echo [ACCEPTANCE] FAILED - runtime/import stages changed tracked staged content.
  echo [ACCEPTANCE] Exact-head evidence is refused; staged bytes are not part of the pinned HEAD.
  exit /b 32
)

echo [ACCEPTANCE] Exact source HEAD and tracked worktree remained stable through runtime/material gates: %PASS45_SOURCE_SHA%

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

echo.
echo [ACCEPTANCE] Verifying Gate K final-world visual truth...
%PY_CMD% "%GATE_K_VERIFY%" "%GAMEPLAY_LOG%"
set "GATE_K_RC=%ERRORLEVEL%"
if not "%GATE_K_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 Gate K still contains visible BasicShape/proxy core content.
  echo Gameplay log: %GAMEPLAY_LOG%
  exit /b %GATE_K_RC%
)

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
echo [ACCEPTANCE] Verifying M700 / Remington 870 / Lever Action manual-action runtime evidence...
%PY_CMD% "%MANUAL_ACTION_VERIFY%" "%GAMEPLAY_LOG%"
set "MANUAL_ACTION_RC=%ERRORLEVEL%"
if not "%MANUAL_ACTION_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 item 16 manual-action evidence is incomplete.
  echo The strict run must exercise M700 bolt, Remington 870 pump and Lever Action, with loaded mechanical audio and authored moving-part animation.
  echo Gameplay log: %GAMEPLAY_LOG%
  exit /b %MANUAL_ACTION_RC%
)

echo.
echo [ACCEPTANCE] Verifying authored first-person grenade hand/throw/recover animation evidence...
%PY_CMD% "%GRENADE_ANIM_VERIFY%" "%GAMEPLAY_LOG%"
set "GRENADE_ANIM_RC=%ERRORLEVEL%"
if not "%GRENADE_ANIM_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 grenade throw presentation is still a content gap.
  echo A presentation event bridge alone cannot satisfy item 24 without authored hand/throw/recover runtime evidence.
  echo Gameplay log: %GAMEPLAY_LOG%
  exit /b %GRENADE_ANIM_RC%
)

echo.
echo [ACCEPTANCE] Verifying distinct authored flash-grenade world VFX evidence...
%PY_CMD% "%FLASH_VFX_VERIFY%" "%GAMEPLAY_LOG%"
set "FLASH_VFX_RC=%ERRORLEVEL%"
if not "%FLASH_VFX_RC%"=="0" (
  echo.
  echo [ACCEPTANCE] FAILED - Pass45 flash grenade has no accepted distinct authored world presentation.
  echo Gameplay flash semantics alone cannot satisfy item 24 frag/smoke/flash visual distinction.
  echo Gameplay log: %GAMEPLAY_LOG%
  exit /b %FLASH_VFX_RC%
)

echo.
echo ============================================================
echo [ACCEPTANCE] PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED.
echo [ACCEPTANCE] Source: %PASS45_SOURCE_SHA%
echo [ACCEPTANCE] Evidence: %EVIDENCE_OUT%
echo [ACCEPTANCE] Gate K: zero visible Engine BasicShape core content in final Oster/stadium presentation.
echo [ACCEPTANCE] Manual action: M700/870/Lever authoritative cycle + mechanical audio + authored moving-part evidence present.
echo [ACCEPTANCE] Grenade throw: authored hand/throw/recover runtime evidence present.
echo [ACCEPTANCE] Flash grenade: distinct authored world VFX runtime evidence present.
echo [ACCEPTANCE] Exact weapon payload gaps remain CONTENT GAP unless real production content is later supplied.
echo [ACCEPTANCE] VISUAL ACCEPTANCE IS STILL PENDING direct screenshots/observation.
echo ============================================================
exit /b 0
