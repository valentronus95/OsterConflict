@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%~dp0.."
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "INBOX=%REPO_ROOT%\models_game_OC"
set "AUDIT=%PROJECT_DIR%Scripts\audit_local_model_inbox.ps1"
set "PREPARE=%PROJECT_DIR%Scripts\prepare_all_local_inbox_assets.ps1"
set "IMPORTER=%PROJECT_DIR%Scripts\import_all_project_assets.py"
set "SUCCESS=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings_success.txt"
set "MANIFEST=%PROJECT_DIR%Saved\LocalModelInbox\runtime_bindings.json"
set "LOG=%PROJECT_DIR%Saved\Logs\AllLocalInboxImport.log"
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "UE_CMD="

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

if not defined UE_CMD (
  echo [STOP] Unreal Engine 5.8 UnrealEditor-Cmd.exe не знайдено.
  exit /b 50
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
if not exist "%IMPORTER%" (
  echo [STOP] Відсутній єдиний UE importer локальних + Unreal/Fab моделей: %IMPORTER%
  exit /b 54
)

if exist "%SUCCESS%" del /q "%SUCCESS%" >nul 2>nul
if exist "%MANIFEST%" del /q "%MANIFEST%" >nul 2>nul
if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - ВСІ МОДЕЛІ ПРОЕКТУ
echo models_game_OC + Unreal/Fab/Marketplace Content -> UE import/catalog -> runtime binding
echo ============================================================

echo [1/4] Перевіряю локальні ZIP...
powershell -NoProfile -ExecutionPolicy Bypass -File "%AUDIT%" -ProjectDir "%PROJECT_DIR%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [2/4] Розпаковую локальні ZIP і переношу UE-ready assets у Content...
powershell -NoProfile -ExecutionPolicy Bypass -File "%PREPARE%" -ProjectDir "%PROJECT_DIR%"
if errorlevel 1 exit /b !ERRORLEVEL!

echo [3/4] Збираю актуальний OsterConflictEditor перед імпортом...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%UPROJECT%" -WaitMutex
set "BUILD_RC=!ERRORLEVEL!"
if not "!BUILD_RC!"=="0" (
  echo [STOP] UE build перед імпортом завершився з кодом !BUILD_RC!.
  exit /b !BUILD_RC!
)

echo [4/4] Імпортую models_game_OC і додаю ВСІ mesh assets із Unreal/Fab/Marketplace packs у runtime catalog...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%IMPORTER%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "IMPORT_RC=!ERRORLEVEL!"
if not "!IMPORT_RC!"=="0" (
  echo [STOP] Unreal importer завершився з кодом !IMPORT_RC!.
  echo Log: %LOG%
  exit /b !IMPORT_RC!
)

if not exist "%SUCCESS%" (
  echo [STOP] Не всі знайдені моделі/HUD/скіни отримали runtime binding.
  echo Manifest: %MANIFEST%
  echo Log: %LOG%
  exit /b 55
)

findstr /L /C:"PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS" "%SUCCESS%" >nul
if errorlevel 1 (
  echo [STOP] Runtime binding sentinel не підтвердив PASS.
  exit /b 56
)

echo [ALL PROJECT ASSETS] PASS: локальні та Unreal/Fab mesh assets зібрані в один runtime catalog.
exit /b 0
