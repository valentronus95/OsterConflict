@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "SCRIPT=%~dp0OsterConflict\Scripts\import_btr4_production_asset.py"

if not exist "%EDITOR_CMD%" (
  echo [BTR4 IMPORT ERROR] UnrealEditor-Cmd.exe not found: %EDITOR_CMD%
  exit /b 2
)
if not exist "%PROJECT%" (
  echo [BTR4 IMPORT ERROR] Project not found: %PROJECT%
  exit /b 3
)
if not exist "%SCRIPT%" (
  echo [BTR4 IMPORT ERROR] Import script not found: %SCRIPT%
  exit /b 4
)

echo [BTR4 IMPORT] Ensuring /Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus...
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash
set "BTR4_RC=%ERRORLEVEL%"
if not "%BTR4_RC%"=="0" (
  echo [BTR4 IMPORT WARN] Canonical BTR-4 import returned exit code %BTR4_RC%.
  echo Runtime may fall back to the legacy proxy shell for this run.
  exit /b %BTR4_RC%
)

echo [BTR4 IMPORT] Canonical BTR-4 visual import completed.
exit /b 0
