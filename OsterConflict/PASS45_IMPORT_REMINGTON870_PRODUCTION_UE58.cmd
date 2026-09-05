@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%PROJECT_DIR%..\"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PILOT=%PROJECT_DIR%TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.cmd"
set "IMPORT_SCRIPT=%REPO_ROOT%PASS45_REMINGTON870_PRODUCTION_UE58_IMPORT.py"
set "VERIFY_SCRIPT=%PROJECT_DIR%Scripts\verify_remington870_production_fresh_load.py"
set "CACHE=%PROJECT_DIR%Saved\ProductionAssetImportCache\Remington870"
set "IMPORT_SENTINEL=%CACHE%\remington870_import_success.txt"
set "FRESH_SENTINEL=%CACHE%\remington870_fresh_load_success.txt"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\Pass45Remington870ProductionImport.log"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\Pass45Remington870ProductionFreshLoad.log"
set "REVISION=PASS45_REMINGTON870_DERIVED_PUMP_PROD_R2"
set "UE_CMD="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not defined UE_CMD if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not defined UE_CMD (
  for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
    if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
  )
)
if not defined UE_CMD (
  for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "5.8 UE_5.8"') do (
    if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
  )
)

if not defined UE_CMD (
  echo [STOP] Unreal Engine 5.8 не знайдено.
  exit /b 2
)
if not exist "%UPROJECT%" (
  echo [STOP] OsterConflict.uproject не знайдено.
  exit /b 3
)
if not exist "%PILOT%" (
  echo [STOP] Ізольований Remington pump pilot відсутній.
  exit /b 4
)
if not exist "%IMPORT_SCRIPT%" (
  echo [STOP] Remington production importer відсутній.
  exit /b 5
)
if not exist "%VERIFY_SCRIPT%" (
  echo [STOP] Remington fresh-load verifier відсутній.
  exit /b 6
)

if not exist "%CACHE%" mkdir "%CACHE%" >nul 2>nul
if exist "%IMPORT_SENTINEL%" del /q "%IMPORT_SENTINEL%" >nul 2>nul
if exist "%FRESH_SENTINEL%" del /q "%FRESH_SENTINEL%" >nul 2>nul
if exist "%IMPORT_LOG%" del /q "%IMPORT_LOG%" >nul 2>nul
if exist "%FRESH_LOG%" del /q "%FRESH_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - REMINGTON 870 PUMP PRODUCTION INTAKE
echo ============================================================
echo [1/3] Ізольований UE 5.8 proof нової цівки та PumpCycle...
call "%PILOT%"
set "PILOT_RC=!ERRORLEVEL!"
if not "!PILOT_RC!"=="0" (
  echo [STOP] Remington pump pilot не пройшов. Production import заборонено. code=!PILOT_RC!
  exit /b 10
)

echo.
echo [2/3] Production import: повний Remington як один skeletal weapon...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%IMPORT_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "IMPORT_RC=!ERRORLEVEL!"
if not "!IMPORT_RC!"=="0" (
  echo.
  echo ============================================================
  echo [STOP] Remington production import не пройшов. code=!IMPORT_RC!
  echo [DIAG] Точна причина з UE логу:
  if exist "%IMPORT_LOG%" (
    findstr /C:"PASS45_REMINGTON870_PRODUCTION_IMPORT_FAIL" /C:"LogPython: Error:" /C:"Python script executed with errors" "%IMPORT_LOG%"
    echo.
    echo [DIAG] Останні 60 рядків:
    powershell -NoProfile -Command "Get-Content -LiteralPath $env:IMPORT_LOG -Tail 60" 2>nul
  ) else (
    echo [DIAG] Import log не створився.
  )
  echo Log: %IMPORT_LOG%
  echo ============================================================
  exit /b 11
)
if not exist "%IMPORT_SENTINEL%" (
  echo [STOP] Remington production import не створив sentinel.
  echo Log: %IMPORT_LOG%
  exit /b 12
)
findstr /L /C:"IMPORT_CONTRACT_REVISION=%REVISION%" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"PRODUCTION_SOURCE_READY=1" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"FULL_WEAPON_FORCED_TO_SINGLE_SKELETAL=1" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"PUMP_MOTION_PRESERVED=1" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"SHARED_SKELETON_PRESERVED=1" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"runtime_acceptance=0" "%IMPORT_SENTINEL%" >nul || goto :bad_import
findstr /L /C:"item16_checked=0" "%IMPORT_SENTINEL%" >nul || goto :bad_import

echo.
echo [3/3] Fresh-load у новому UE 5.8 процесі...
"%UE_CMD%" "%UPROJECT%" -run=pythonscript -script="%VERIFY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "FRESH_RC=!ERRORLEVEL!"
if not "!FRESH_RC!"=="0" (
  echo [STOP] Remington fresh-load verification не пройшла. code=!FRESH_RC!
  echo Log: %FRESH_LOG%
  exit /b 13
)
if not exist "%FRESH_SENTINEL%" (
  echo [STOP] Remington fresh-load sentinel відсутній.
  echo Log: %FRESH_LOG%
  exit /b 14
)
findstr /L /C:"IMPORT_CONTRACT_REVISION=%REVISION%" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"PRODUCTION_FRESH_LOAD_READY=1" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"FULL_WEAPON_SINGLE_SKELETAL=1" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"PUMP_BONE_ADDRESSABLE=1" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"PUMP_MOTION_PRESERVED=1" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"SHARED_SKELETON_PRESERVED=1" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"runtime_acceptance=0" "%FRESH_SENTINEL%" >nul || goto :bad_fresh
findstr /L /C:"item16_checked=0" "%FRESH_SENTINEL%" >nul || goto :bad_fresh

echo.
echo PASS: Remington 870 production skeletal weapon and PumpCycle fresh-load verified in UE 5.8.
echo STATUS: PRODUCTION SOURCE READY. Gameplay pump visibility/audio acceptance still pending.
echo Import log: %IMPORT_LOG%
echo Fresh log:  %FRESH_LOG%
exit /b 0

:bad_import
echo [STOP] Remington import sentinel неповний або застарілий.
type "%IMPORT_SENTINEL%"
exit /b 15

:bad_fresh
echo [STOP] Remington fresh-load sentinel неповний або застарілий.
type "%FRESH_SENTINEL%"
exit /b 16
