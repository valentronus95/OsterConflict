@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"

:menu
cls
echo ============================================================
echo OSTER CONFLICT - ГОЛОВНИЙ ЗАПУСК
 echo ============================================================
echo.
echo 1. ЗВИЧАЙНА ГРА
echo 2. ПОВНИЙ RUNTIME-ТЕСТ
echo 3. SAFE СУМІСНІСТЬ ^(RHI THREAD OFF^)
echo 4. ВІДКРИТИ UNREAL EDITOR
echo 0. ВИХІД
echo.
echo Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.
echo Окремі TRY_/REVIEW_/PASS-specific BAT/CMD для ручного запуску більше не використовуються.
echo.
choice /C 12340 /N /M "Оберіть: "

if errorlevel 5 goto end
if errorlevel 4 (
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject" -d3d11 -sm5 -nohdr
  goto menu
)
if errorlevel 3 (
  set "OC_FORCE_ACCEPTANCE=0"
  set "OC_RHI_COMPAT=1"
  call "%CURRENT_GAMEPLAY%"
  set "OC_RHI_COMPAT="
  set "OC_FORCE_ACCEPTANCE="
  goto menu
)
if errorlevel 2 (
  call :full_runtime_test
  goto menu
)
if errorlevel 1 (
  set "OC_FORCE_ACCEPTANCE=0"
  set "OC_RHI_COMPAT=0"
  call "%CURRENT_GAMEPLAY%"
  set "OC_RHI_COMPAT="
  set "OC_FORCE_ACCEPTANCE="
  goto menu
)

goto menu

:full_runtime_test
if not exist "%CURRENT_GAMEPLAY%" (
  echo [STOP] Відсутній основний gameplay launcher: %CURRENT_GAMEPLAY%
  exit /b 2
)
if not exist "%MATERIAL_GATE%" (
  echo [STOP] Відсутній strict material gate: %MATERIAL_GATE%
  exit /b 3
)
if not exist "%EVIDENCE_VERIFY%" (
  echo [STOP] Відсутній runtime evidence verifier: %EVIDENCE_VERIFY%
  exit /b 4
)

set "OC_FORCE_ACCEPTANCE=1"
set "OC_RHI_COMPAT=0"
call "%CURRENT_GAMEPLAY%"
set "GAME_RC=%ERRORLEVEL%"
set "OC_RHI_COMPAT="
set "OC_FORCE_ACCEPTANCE="
if not "%GAME_RC%"=="0" (
  echo [STOP] Runtime acceptance failed: %GAME_RC%
  exit /b %GAME_RC%
)

call "%MATERIAL_GATE%"
set "MATERIAL_RC=%ERRORLEVEL%"
if not "%MATERIAL_RC%"=="0" (
  echo [STOP] Material/dependency gate failed: %MATERIAL_RC%
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
  echo [STOP] Python 3 not found.
  exit /b 30
)

set "PASS45_SOURCE_SHA=unknown"
for /f "delims=" %%H in ('git rev-parse HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
%PY_CMD% "%EVIDENCE_VERIFY%" "%GAMEPLAY_LOG%" "%MATERIAL_LOG%" "%WEAPON_REPORT%"
set "EVIDENCE_RC=%ERRORLEVEL%"
if not "%EVIDENCE_RC%"=="0" exit /b %EVIDENCE_RC%

echo ============================================================
echo PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED.
echo VISUAL ACCEPTANCE IS STILL PENDING direct observation.
echo ============================================================
exit /b 0

:end
exit /b 0
