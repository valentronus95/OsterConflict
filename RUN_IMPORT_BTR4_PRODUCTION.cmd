@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "SCRIPT=%~dp0OsterConflict\Scripts\import_btr4_production_asset.py"
set "SENTINEL=%~dp0OsterConflict\Saved\ProductionAssetImportCache\BTR4\btr4_import_success.txt"

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
  echo [BTR4 IMPORT ERROR] Canonical BTR-4 import returned exit code %BTR4_RC%.
  exit /b %BTR4_RC%
)

if not exist "%SENTINEL%" (
  echo [BTR4 IMPORT ERROR] Import sentinel was not created: %SENTINEL%
  exit /b 20
)
findstr /C:"source_kind=local_user_fbx" "%SENTINEL%" >nul
if errorlevel 1 (
  echo [BTR4 IMPORT STOP] Only the generated approximation is available.
  echo Required real source: OsterConflict\SourceAssets\Production\Vehicles\BTR4\BTR4_Bucephalus.fbx
  echo Gameplay acceptance is blocked instead of showing a proxy BTR as if it were final.
  exit /b 21
)

echo [BTR4 IMPORT] Real user-selected BTR-4 production asset is ready.
exit /b 0
