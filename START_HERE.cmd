@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "ALL_ASSET_IMPORT=%~dp0OsterConflict\IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "PROJECT_DIR=%~dp0OsterConflict"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
set "LOCAL_INBOX_RUNTIME_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\local_inbox_runtime_validation.txt"

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
echo models_game_OC тепер обробляється автоматично перед грою або Editor:
echo ZIP -> розпакування -> Content -> UE import -> runtime binding.
echo.
choice /C 12340 /N /M "Оберіть: "

if errorlevel 5 goto end
if errorlevel 4 (
  call :ingest_all_assets
  if errorlevel 1 goto menu
  start "" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0OsterConflict\OsterConflict.uproject" -d3d11 -sm5 -nohdr
  goto menu
)
if errorlevel 3 (
  call :ingest_all_assets
  if errorlevel 1 goto menu
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
  call :ingest_all_assets
  if errorlevel 1 goto menu
  set "OC_FORCE_ACCEPTANCE=0"
  set "OC_RHI_COMPAT=0"
  call "%CURRENT_GAMEPLAY%"
  set "OC_RHI_COMPAT="
  set "OC_FORCE_ACCEPTANCE="
  goto menu
)

goto menu

:ingest_all_assets
if not exist "%ALL_ASSET_IMPORT%" (
  echo [STOP] Відсутній єдиний importer усіх локальних assets: %ALL_ASSET_IMPORT%
  exit /b 5
)
call "%ALL_ASSET_IMPORT%"
set "ASSET_RC=%ERRORLEVEL%"
if not "%ASSET_RC%"=="0" (
  echo [STOP] Локальні моделі/HUD/скіни не завершили повний ingest. Код: %ASSET_RC%
  exit /b %ASSET_RC%
)
exit /b 0

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

call :ingest_all_assets
if errorlevel 1 exit /b %ERRORLEVEL%

if exist "%LOCAL_INBOX_RUNTIME_REPORT%" del /q "%LOCAL_INBOX_RUNTIME_REPORT%" >nul 2>nul
set "OC_FORCE_ACCEPTANCE=1"
set "OC_RHI_COMPAT=0"
set "OC_VALIDATE_LOCAL_INBOX=1"
call "%CURRENT_GAMEPLAY%"
set "GAME_RC=%ERRORLEVEL%"
set "OC_VALIDATE_LOCAL_INBOX="
set "OC_RHI_COMPAT="
set "OC_FORCE_ACCEPTANCE="
if not "%GAME_RC%"=="0" (
  echo [STOP] Runtime acceptance failed: %GAME_RC%
  exit /b %GAME_RC%
)

if not exist "%LOCAL_INBOX_RUNTIME_REPORT%" (
  echo [STOP] Не отримано live runtime proof для models_game_OC.
  echo Очікувався файл: %LOCAL_INBOX_RUNTIME_REPORT%
  exit /b 35
)
findstr /L /C:"PASS45_LOCAL_INBOX_RUNTIME=PASS" "%LOCAL_INBOX_RUNTIME_REPORT%" >nul
if errorlevel 1 (
  echo [STOP] Не всі локальні моделі реально завантажились у gameplay runtime.
  type "%LOCAL_INBOX_RUNTIME_REPORT%"
  exit /b 36
)
echo [MODEL INBOX] PASS: усі прив'язані моделі реально відкрились у gameplay runtime.

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
echo ALL models_game_OC assets also passed live runtime loading.
echo VISUAL ACCEPTANCE IS STILL PENDING direct observation.
echo ============================================================
exit /b 0

:end
exit /b 0
