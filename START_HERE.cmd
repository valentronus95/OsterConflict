@echo off
setlocal EnableExtensions
chcp 65001 >nul
rem Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.
cd /d "%~dp0"

set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "ALL_ASSET_IMPORT=%~dp0OsterConflict\IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "ASSET_STATUS_COLLECTOR=%~dp0COLLECT_LOCAL_ASSET_STATUS.py"
set "ASSET_STATUS_TEXT=%~dp0OsterConflict\Saved\AssetStatus\LOCAL_ASSET_STATUS.txt"
set "PROJECT_DIR=%~dp0OsterConflict"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
set "LOCAL_INBOX_RUNTIME_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\local_inbox_runtime_validation.txt"
set "LOCAL_WORLD_RUNTIME_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\local_world_runtime_validation.txt"

rem The old PASS45 linked worktree was only an implementation workspace. Before removing it, rescue
rem every asset family that Unreal/Fab may have written there but Git never tracked, without overwriting main.
call :consolidate_legacy_worktree
if errorlevel 1 (
  echo [STOP] Не вдалося безпечно прибрати стару OsterConflict_PASS45 після перенесення assets.
  pause
  exit /b %ERRORLEVEL%
)

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

:consolidate_legacy_worktree
for %%I in ("%~dp0..\OsterConflict_PASS45") do set "LEGACY_WORKTREE=%%~fI"
if not exist "%LEGACY_WORKTREE%" exit /b 0

echo [CLEANUP] Знайдено старий linked worktree: %LEGACY_WORKTREE%
echo [CLEANUP] Спочатку забираю з нього всі локальні UE/Fab assets, яких ще немає в main...
call :copy_missing_tree "%LEGACY_WORKTREE%\OsterConflict\Content" "%~dp0OsterConflict\Content"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_missing_tree "%LEGACY_WORKTREE%\OsterConflict\Plugins" "%~dp0OsterConflict\Plugins"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_missing_tree "%LEGACY_WORKTREE%\OsterConflict\AudioSources" "%~dp0OsterConflict\AudioSources"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_missing_tree "%LEGACY_WORKTREE%\OsterConflict\SourceAssets" "%~dp0OsterConflict\SourceAssets"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_missing_tree "%LEGACY_WORKTREE%\OsterConflict\SourceReferences" "%~dp0OsterConflict\SourceReferences"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_missing_tree "%LEGACY_WORKTREE%\models_game_OC" "%~dp0models_game_OC"
if errorlevel 1 exit /b %ERRORLEVEL%

where git >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git не знайдений. Assets перенесені, але linked worktree не видаляю навмання.
  exit /b 60
)

echo [CLEANUP] Assets врятовані. Видаляю старий linked worktree і його папку...
git -C "%~dp0" worktree remove --force "%LEGACY_WORKTREE%" >nul 2>nul
if errorlevel 1 (
  rem The registration may already be stale. The requested directory is obsolete after the asset rescue above.
  if exist "%LEGACY_WORKTREE%" rmdir /s /q "%LEGACY_WORKTREE%"
)
git -C "%~dp0" worktree prune >nul 2>nul
if exist "%LEGACY_WORKTREE%" (
  echo [STOP] Windows не дозволив видалити %LEGACY_WORKTREE%.
  exit /b 61
)
echo [CLEANUP] PASS: OsterConflict_PASS45 прибрано; лишається один локальний проект OsterConflict.
exit /b 0

:copy_missing_tree
if not exist "%~1" exit /b 0
if not exist "%~2" mkdir "%~2" >nul 2>nul
robocopy "%~1" "%~2" /E /XC /XN /XO /R:1 /W:1 /NFL /NDL /NJH /NJS /NP >nul
set "ROBO_RC=%ERRORLEVEL%"
if %ROBO_RC% GEQ 8 (
  echo [STOP] Не вдалося перенести assets: %~1 ^> %~2 ^(robocopy=%ROBO_RC%^)
  exit /b %ROBO_RC%
)
exit /b 0

:ingest_all_assets
if not exist "%ALL_ASSET_IMPORT%" (
  echo [STOP] Відсутній єдиний importer усіх локальних assets: %ALL_ASSET_IMPORT%
  exit /b 5
)
call "%ALL_ASSET_IMPORT%"
set "ASSET_RC=%ERRORLEVEL%"
call :write_asset_snapshot
if not "%ASSET_RC%"=="0" (
  echo [STOP] Локальні моделі/HUD/скіни не завершили повний ingest. Код: %ASSET_RC%
  if exist "%ASSET_STATUS_TEXT%" echo [ASSET STATUS] Зведення: %ASSET_STATUS_TEXT%
  exit /b %ASSET_RC%
)
if exist "%ASSET_STATUS_TEXT%" echo [ASSET STATUS] Import snapshot: %ASSET_STATUS_TEXT%
exit /b 0

:write_asset_snapshot
if not exist "%ASSET_STATUS_COLLECTOR%" (
  echo [WARN] Відсутній collector локального asset-статусу: %ASSET_STATUS_COLLECTOR%
  exit /b 0
)
set "ASSET_PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "ASSET_PY_CMD=py -3"
if not defined ASSET_PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "ASSET_PY_CMD=python"
)
if not defined ASSET_PY_CMD (
  echo [WARN] Python 3 не знайдений; LOCAL_ASSET_STATUS snapshot не створено.
  exit /b 0
)
set "PASS45_SOURCE_SHA=unknown"
for /f "delims=" %%H in ('git -C "%~dp0" rev-parse HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"
if errorlevel 1 (
  echo [WARN] Не вдалося створити LOCAL_ASSET_STATUS snapshot.
  exit /b 0
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
if exist "%LOCAL_WORLD_RUNTIME_REPORT%" del /q "%LOCAL_WORLD_RUNTIME_REPORT%" >nul 2>nul
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

if not exist "%LOCAL_WORLD_RUNTIME_REPORT%" (
  echo [STOP] Не отримано runtime proof, що world-моделі реально підключені до Остра.
  exit /b 37
)
findstr /L /C:"PASS45_LOCAL_WORLD_RUNTIME=PASS" "%LOCAL_WORLD_RUNTIME_REPORT%" >nul
if errorlevel 1 (
  echo [STOP] World assets не пройшли live placement proof.
  type "%LOCAL_WORLD_RUNTIME_REPORT%"
  exit /b 38
)
echo [WORLD ASSETS] PASS: будівлі/пропи/рослинність/дороги/вода підключені до live Oster runtime.

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
