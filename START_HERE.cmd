@echo off
setlocal EnableExtensions
chcp 65001 >nul
rem Єдиний користувацький launcher/test entrypoint: START_HERE.cmd.
cd /d "%~dp0"

set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "ALL_ASSET_IMPORT=%~dp0OsterConflict\IMPORT_ALL_LOCAL_INBOX_UE58.cmd"
set "MATERIAL_GATE=%~dp0OsterConflict\RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
set "ASSET_FINALIZER=%~dp0OsterConflict\Scripts\finalize_asset_acceptance.py"
set "ASSET_STATUS_COLLECTOR=%~dp0COLLECT_LOCAL_ASSET_STATUS.py"
set "ASSET_STATUS_TEXT=%~dp0OsterConflict\Saved\AssetStatus\LOCAL_ASSET_STATUS.txt"
set "ASSET_STATUS_JSON=%~dp0OsterConflict\Saved\AssetStatus\LOCAL_ASSET_STATUS.json"
set "PROJECT_DIR=%~dp0OsterConflict"
set "GAMEPLAY_LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"
set "MATERIAL_LOG=%~dp0Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "WEAPON_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\weapon_runtime_validation.txt"
set "LOCAL_INBOX_RUNTIME_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\local_inbox_runtime_validation.txt"
set "LOCAL_WORLD_RUNTIME_REPORT=%~dp0OsterConflict\Saved\AutomationReports\ProductionModels\local_world_runtime_validation.txt"
set "ASSET_RC="
set "RUNTIME_RC="

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

:verify_current_asset_source
where git >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git не знайдений; asset ingest не запускаю на невідомому source state.
  exit /b 66
)
set "CURRENT_ASSET_BRANCH="
set "LOCAL_ASSET_HEAD="
set "REMOTE_ASSET_HEAD="
for /f "delims=" %%B in ('git -C "%~dp0" branch --show-current 2^>nul') do set "CURRENT_ASSET_BRANCH=%%B"
if not defined CURRENT_ASSET_BRANCH (
  echo [STOP] Не вдалося визначити current Git branch; asset ingest скасовано.
  exit /b 67
)
echo [ASSET PRECHECK] Fetching origin/%CURRENT_ASSET_BRANCH% before UE import...
git -C "%~dp0" fetch origin "%CURRENT_ASSET_BRANCH%"
if errorlevel 1 (
  echo [STOP] Не вдалося fetch origin/%CURRENT_ASSET_BRANCH%; asset ingest скасовано замість тесту невідомого коду.
  exit /b 68
)
for /f "delims=" %%H in ('git -C "%~dp0" rev-parse HEAD 2^>nul') do set "LOCAL_ASSET_HEAD=%%H"
for /f "delims=" %%H in ('git -C "%~dp0" rev-parse "origin/%CURRENT_ASSET_BRANCH%" 2^>nul') do set "REMOTE_ASSET_HEAD=%%H"
if not defined LOCAL_ASSET_HEAD (
  echo [STOP] Не вдалося визначити локальний HEAD; asset ingest скасовано.
  exit /b 69
)
if not defined REMOTE_ASSET_HEAD (
  echo [STOP] Не вдалося визначити origin/%CURRENT_ASSET_BRANCH%; asset ingest скасовано.
  exit /b 69
)
if /I not "%LOCAL_ASSET_HEAD%"=="%REMOTE_ASSET_HEAD%" (
  echo [STOP] Локальний %CURRENT_ASSET_BRANCH% не відповідає GitHub origin/%CURRENT_ASSET_BRANCH%.
  echo Local : %LOCAL_ASSET_HEAD%
  echo GitHub: %REMOTE_ASSET_HEAD%
  echo Спочатку Fetch/Pull у GitHub Desktop. UE import на застарілому HEAD не запускається.
  exit /b 70
)
echo [ASSET PRECHECK] PASS: local HEAD matches origin/%CURRENT_ASSET_BRANCH% = %LOCAL_ASSET_HEAD%
exit /b 0

:ingest_all_assets
set "RUNTIME_RC="
call :verify_current_asset_source
if errorlevel 1 exit /b %ERRORLEVEL%
if not exist "%ALL_ASSET_IMPORT%" (
  echo [STOP] Відсутній єдиний importer усіх локальних assets: %ALL_ASSET_IMPORT%
  exit /b 5
)
call "%ALL_ASSET_IMPORT%"
set "ASSET_RC=%ERRORLEVEL%"
call :write_asset_snapshot
set "SNAPSHOT_RC=%ERRORLEVEL%"
if not "%ASSET_RC%"=="0" (
  echo [STOP] Локальні моделі/HUD/скіни не завершили повний ingest. Код: %ASSET_RC%
  if exist "%ASSET_STATUS_TEXT%" echo [ASSET STATUS] Зведення: %ASSET_STATUS_TEXT%
  exit /b %ASSET_RC%
)
if not "%SNAPSHOT_RC%"=="0" (
  echo [STOP] Asset import завершився, але current LOCAL_ASSET_STATUS не створено. Код snapshot: %SNAPSHOT_RC%
  exit /b %SNAPSHOT_RC%
)
echo [ASSET STATUS] Import snapshot: %ASSET_STATUS_TEXT%
exit /b 0

:write_asset_snapshot
if not exist "%ASSET_STATUS_COLLECTOR%" (
  echo [STOP] Відсутній collector локального asset-статусу: %ASSET_STATUS_COLLECTOR%
  exit /b 62
)
set "ASSET_PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "ASSET_PY_CMD=py -3"
if not defined ASSET_PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "ASSET_PY_CMD=python"
)
if not defined ASSET_PY_CMD (
  echo [STOP] Python 3 не знайдений; LOCAL_ASSET_STATUS snapshot не створено.
  exit /b 63
)
set "PASS45_SOURCE_SHA=unknown"
for /f "delims=" %%H in ('git -C "%~dp0" rev-parse HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
if defined ASSET_RC set "PASS45_ASSET_IMPORT_RC=%ASSET_RC%"
if defined RUNTIME_RC set "PASS45_RUNTIME_RC=%RUNTIME_RC%"
if exist "%ASSET_STATUS_TEXT%" del /q "%ASSET_STATUS_TEXT%" >nul 2>nul
if exist "%ASSET_STATUS_JSON%" del /q "%ASSET_STATUS_JSON%" >nul 2>nul
%ASSET_PY_CMD% "%ASSET_STATUS_COLLECTOR%"
set "ASSET_STATUS_RC=%ERRORLEVEL%"
set "PASS45_ASSET_IMPORT_RC="
set "PASS45_RUNTIME_RC="
if not "%ASSET_STATUS_RC%"=="0" (
  echo [STOP] Не вдалося створити LOCAL_ASSET_STATUS snapshot. Collector code: %ASSET_STATUS_RC%
  exit /b 64
)
if not exist "%ASSET_STATUS_TEXT%" (
  echo [STOP] Collector завершився без LOCAL_ASSET_STATUS.txt.
  exit /b 65
)
if not exist "%ASSET_STATUS_JSON%" (
  echo [STOP] Collector завершився без LOCAL_ASSET_STATUS.json.
  exit /b 65
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
if not exist "%ASSET_FINALIZER%" (
  echo [STOP] Відсутній final asset acceptance helper: %ASSET_FINALIZER%
  exit /b 71
)

call :ingest_all_assets
if errorlevel 1 exit /b %ERRORLEVEL%
set "RUNTIME_RC="

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
  set "RUNTIME_RC=%GAME_RC%"
  call :write_asset_snapshot
  echo [STOP] Runtime acceptance failed: %GAME_RC%
  if exist "%ASSET_STATUS_TEXT%" echo [ASSET STATUS] Runtime snapshot: %ASSET_STATUS_TEXT%
  exit /b %GAME_RC%
)

if not exist "%LOCAL_INBOX_RUNTIME_REPORT%" (
  set "RUNTIME_RC=35"
  call :write_asset_snapshot
  echo [STOP] Не отримано live runtime proof для models_game_OC.
  echo Очікувався файл: %LOCAL_INBOX_RUNTIME_REPORT%
  exit /b 35
)
findstr /L /C:"PASS45_LOCAL_INBOX_RUNTIME=PASS" "%LOCAL_INBOX_RUNTIME_REPORT%" >nul
if errorlevel 1 (
  set "RUNTIME_RC=36"
  call :write_asset_snapshot
  echo [STOP] Не всі локальні моделі реально завантажились у gameplay runtime.
  type "%LOCAL_INBOX_RUNTIME_REPORT%"
  exit /b 36
)
echo [MODEL INBOX] PASS: усі прив'язані моделі реально відкрились у gameplay runtime.

if not exist "%LOCAL_WORLD_RUNTIME_REPORT%" (
  set "RUNTIME_RC=37"
  call :write_asset_snapshot
  echo [STOP] Не отримано runtime proof, що world-моделі реально підключені до Остра.
  exit /b 37
)
findstr /L /C:"PASS45_LOCAL_WORLD_RUNTIME=PASS" "%LOCAL_WORLD_RUNTIME_REPORT%" >nul
if errorlevel 1 (
  set "RUNTIME_RC=38"
  call :write_asset_snapshot
  echo [STOP] World assets не пройшли live placement proof.
  type "%LOCAL_WORLD_RUNTIME_REPORT%"
  exit /b 38
)
echo [WORLD ASSETS] PASS: будівлі/пропи/рослинність/дороги/вода підключені до live Oster runtime.

call "%MATERIAL_GATE%"
set "MATERIAL_RC=%ERRORLEVEL%"
if not "%MATERIAL_RC%"=="0" (
  set "RUNTIME_RC=%MATERIAL_RC%"
  call :write_asset_snapshot
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
  set "RUNTIME_RC=30"
  call :write_asset_snapshot
  echo [STOP] Python 3 not found.
  exit /b 30
)

set "PASS45_SOURCE_SHA=unknown"
for /f "delims=" %%H in ('git rev-parse HEAD 2^>nul') do set "PASS45_SOURCE_SHA=%%H"
%PY_CMD% "%EVIDENCE_VERIFY%" "%GAMEPLAY_LOG%" "%MATERIAL_LOG%" "%WEAPON_REPORT%"
set "EVIDENCE_RC=%ERRORLEVEL%"
if not "%EVIDENCE_RC%"=="0" (
  set "RUNTIME_RC=%EVIDENCE_RC%"
  call :write_asset_snapshot
  exit /b %EVIDENCE_RC%
)

echo ============================================================
echo PASS45 AUTOMATED RUNTIME EVIDENCE GATES PASSED.
echo ALL models_game_OC assets also passed live runtime loading.
echo ============================================================

echo [FINALIZE PRECHECK] Перевіряю, чи взагалі можна переходити до ручного visual acceptance...
%PY_CMD% "%ASSET_FINALIZER%" --preflight
set "FINAL_PRECHECK_RC=%ERRORLEVEL%"
if not "%FINAL_PRECHECK_RC%"=="0" (
  echo [FINALIZE PENDING] Automated runtime PASS збережено, але 100%% поки заблоковано factual GAP/cleanup precheck.
  echo [FINALIZE PENDING] ZIP не видалялись. Visual acceptance не записано.
  exit /b 0
)

echo.
echo Перед підтвердженням перевірте у щойно завершеному runtime/UE:
echo - HMMWV, M2 Browning і BTR-4: масштаб, орієнтація, матеріали, кріплення;
echo - зброю, включно з M16/M4: правильні mesh/materials без placeholder;
echo - будівлі, пропи, рослинність, дороги, terrain і water;
echo - character skins та HUD/UI, які були discovered/bound;
echo - відсутність відірваних mesh, дикого масштабу або очевидно зламаних матеріалів.
echo.
choice /C YN /N /M "Ви реально оглянули ці assets і приймаєте їх візуальний стан? [Y/N]: "
if errorlevel 2 (
  echo [FINALIZE PENDING] Visual acceptance залишено PENDING. ZIP не видалялись.
  exit /b 0
)

%PY_CMD% "%ASSET_FINALIZER%" --accept-visual
set "FINALIZE_RC=%ERRORLEVEL%"
if not "%FINALIZE_RC%"=="0" (
  echo [STOP] Final visual acceptance / safe ZIP cleanup не завершено. Код: %FINALIZE_RC%
  exit /b %FINALIZE_RC%
)

echo ============================================================
echo PASS45 FULL ASSET LIFECYCLE ACCEPTED.
echo DIRECT VISUAL ACCEPTANCE: PASS.
echo SOURCE ZIP CLEANUP: PASS.
echo ============================================================
exit /b 0

:end
exit /b 0
