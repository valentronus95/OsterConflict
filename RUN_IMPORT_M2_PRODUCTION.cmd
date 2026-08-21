@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "SCRIPT=%~dp0OsterConflict\Scripts\import_m2_production_asset.py"
set "SENTINEL=%~dp0OsterConflict\Saved\ProductionAssetImportCache\M2\m2_import_success.txt"

if not exist "%EDITOR_CMD%" (
  echo [M2 IMPORT ERROR] UnrealEditor-Cmd.exe not found: %EDITOR_CMD%
  exit /b 2
)
if not exist "%PROJECT%" (
  echo [M2 IMPORT ERROR] Project not found: %PROJECT%
  exit /b 3
)
if not exist "%SCRIPT%" (
  echo [M2 IMPORT ERROR] Import script not found: %SCRIPT%
  exit /b 4
)

echo [M2 IMPORT] Ensuring /Game/Production/Weapons/M2/SM_M2_Browning...
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash
set "M2_RC=%ERRORLEVEL%"
if not "%M2_RC%"=="0" (
  echo [M2 IMPORT ERROR] Production M2 import returned exit code %M2_RC%.
  exit /b %M2_RC%
)

if not exist "%SENTINEL%" (
  echo [M2 IMPORT ERROR] Import sentinel was not created: %SENTINEL%
  exit /b 20
)
findstr /C:"source_kind=downloaded" "%SENTINEL%" >nul
if errorlevel 1 (
  echo [M2 IMPORT STOP] Only the generated approximation is available.
  echo Required real source: OsterConflict\SourceAssets\Production\Weapons\M2\m2_50cal_machinegun_cc0.glb
  echo Gameplay acceptance is blocked instead of pretending the approximation is the final Browning.
  exit /b 21
)

echo [M2 IMPORT] Real downloaded M2 production asset is ready.
exit /b 0
