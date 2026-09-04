@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%~dp0.."
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "INBOX=%REPO_ROOT%\models_game_OC"
set "AUDIT=%PROJECT_DIR%Scripts\audit_local_model_inbox.ps1"
set "PREPARE=%PROJECT_DIR%Scripts\prepare_all_local_inbox_assets.ps1"
set "DEDUPE=%PROJECT_DIR%Scripts\dedupe_local_prepared_sources.py"
set "PREPARE_WEAPONS=%PROJECT_DIR%Scripts\prepare_local_weapon_sources.ps1"
set "IMPORTER=%PROJECT_DIR%Scripts\import_all_project_assets.py"
set "NORMALIZE_WEAPONS=%PROJECT_DIR%Scripts\normalize_local_weapon_categories.py"
set "SUCCESS=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings_success.txt"
set "MANIFEST=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings.json"
set "LOG=%PROJECT_DIR%Saved\Logs\AllLocalInboxImport.log"
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "UE_CMD="
set "PY_CMD="

if not exist "%INBOX%" (
  echo [LOCAL ASSETS] models_game_OC відсутня; все одно перевіряю і підключаю assets, завантажені прямо через Unreal/Fab.
)

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not defined UE_CMD if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not defined UE_CMD (
  for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
    if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
  )
)

where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)

if not defined UE_CMD (
  echo [STOP] Unreal Engine 5.8 UnrealEditor-Cmd.exe не знайдено.
  exit /b 50
)
if not defined PY_CMD (
  echo [STOP] Python 3 не знайдено; без нього неможливо перевірити локальні assets.
  exit /b 46
)
if not exist "%BUILD_BAT%" (
  echo [STOP] UE 5.8 Build.bat не знайдено: %BUILD_BAT%
  exit /b 50
)
if not exist "%UPROJECT%" (
  echo [STOP] Проєкт не знайдено: %UPROJECT%
  exit /b 51
)
if not exist "%AUDIT%" (
  echo [STOP] Відсутній аудит локальних моделей: %AUDIT%
  exit /b 52
)
if not exist "%PREPARE%" (
  echo [STOP] Відсутній розпаковувач локальних моделей: %PREPARE%
  exit /b 53
)
if not exist "%DEDUPE%" (
  echo [STOP] Відсутня перевірка точних дублів: %DEDUPE%
  exit /b 58
)
if not exist "%PREPARE_WEAPONS%" (
  echo [STOP] Відсутній staging точних M249/Remington 870: %PREPARE_WEAPONS%
  exit /b 53
)
if not exist "%IMPORTER%" (
  echo [STOP] Відсутній єдиний UE importer локальних + Unreal/Fab моделей: %IMPORTER%
  exit /b 54
)
if not exist "%NORMALIZE_WEAPONS%" (
  echo [STOP] Відсутня перевірка live-binding усієї завантаженої зброї: %NORMALIZE_WEAPONS%
  exit /b 57
)

if exist "%SUCCESS%" del /q "%SUCCESS%" >nul 2>nul
if exist "%MANIFEST%" del /q "%MANIFEST%" >nul 2>nul
if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - ВСІ ASSETS ПРОЕКТУ
echo models_game_OC + Unreal/Fab/Marketplace Content -> dedupe -> UE import -> gameplay/runtime binding
echo ============================================================

echo [1/8] Дотягую реальні Git LFS payloads для вже доданих model packs...
where git >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git не знайдено в PATH.
  exit /b 47
)
git -C "%REPO_ROOT%" lfs version >nul 2>nul
if errorlevel 1 (
  echo [STOP] Git LFS не встановлено; без нього .uasset можуть лишитися pointer-файлами.
  exit /b 48
)
set "CURRENT_BRANCH="
for /f "delims=" %%B in ('git -C "%REPO_ROOT%" branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
if not defined CURRENT_BRANCH set "CURRENT_BRANCH=main"
git -C "%REPO_ROOT%" lfs pull origin "%CURRENT_BRANCH%"
if errorlevel 1 (
  echo [STOP] Git LFS не зміг дотягнути assets для %CURRENT_BRANCH%.
  exit /b 49
)
git -C "%REPO_ROOT%" lfs checkout >nul
if errorlevel 1 (
  echo [STOP] Git LFS checkout не зміг розгорнути локальні asset payloads.
  exit /b 49
)

echo [2/8] Інвентаризую ВСІ локальні ZIP, не тільки стару production-п'ятірку...
powershell -NoProfile -ExecutionPolicy Bypass -File "%AUDIT%" -ProjectDir "%PROJECT_DIR%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [3/8] Розпаковую локальні ZIP і переношу UE-ready assets у Content...
powershell -NoProfile -ExecutionPolicy Bypass -File "%PREPARE%" -ProjectDir "%PROJECT_DIR%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [4/8] Прибираю ТОЧНІ дублікати зі списку імпорту за SHA-256; файли користувача не видаляю...
%PY_CMD% "%DEDUPE%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [5/8] Шукаю і готую точні M249 та Remington 870 з models_game_OC...
powershell -NoProfile -ExecutionPolicy Bypass -File "%PREPARE_WEAPONS%" -ProjectDir "%PROJECT_DIR%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [6/8] Збираю актуальний OsterConflictEditor перед імпортом...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%UPROJECT%" -WaitMutex
set "BUILD_RC=!ERRORLEVEL!"
if not "!BUILD_RC!"=="0" (
  echo [STOP] UE build перед імпортом завершився з кодом !BUILD_RC!.
  exit /b !BUILD_RC!
)

echo [7/8] Імпортую ВСІ нові models_game_OC та сканую ВСІ assets із Unreal/Fab/Marketplace packs...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%IMPORTER%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "IMPORT_RC=!ERRORLEVEL!"
if not "!IMPORT_RC!"=="0" (
  echo [STOP] Unreal importer завершився з кодом !IMPORT_RC!.
  echo Log: %LOG%
  exit /b !IMPORT_RC!
)

echo [8/8] Прив'язую КОЖНУ знайдену зброю до live weapon class і забороняю тихий WEAPON_OTHER...
%PY_CMD% "%NORMALIZE_WEAPONS%"
set "WEAPON_BIND_RC=!ERRORLEVEL!"
if not "!WEAPON_BIND_RC!"=="0" (
  echo [STOP] Є імпортована зброя, яка досі не має реального gameplay/runtime binding.
  echo Manifest: %MANIFEST%
  echo Код: !WEAPON_BIND_RC!
  exit /b !WEAPON_BIND_RC!
)

if not exist "%SUCCESS%" (
  echo [STOP] Не всі знайдені моделі/HUD/скіни/зброя отримали runtime binding.
  echo Manifest: %MANIFEST%
  echo Log: %LOG%
  exit /b 55
)

findstr /L /C:"PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS" "%SUCCESS%" >nul
if errorlevel 1 (
  echo [STOP] Runtime binding sentinel не підтвердив PASS.
  exit /b 56
)

echo [ALL PROJECT ASSETS] PASS: усі знайдені локальні, production та Unreal/Fab assets мають runtime binding.
exit /b 0
