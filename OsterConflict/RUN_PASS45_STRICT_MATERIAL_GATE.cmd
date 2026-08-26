@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "REPORT_DIR=%PROJECT_DIR%Saved\AutomationReports\ProductionModels"
set "WEAPON_REPORT=%REPORT_DIR%\weapon_runtime_validation.txt"
set "WEAPON_SENTINEL=%REPORT_DIR%\production_weapon_runtime_success.txt"
set "MATERIAL_LOG=%~dp0..\Logs\PASS45_STRICT_MATERIAL_GATE.log"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not defined UE_ROOT (
  for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
    if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%%B"
  )
)

if not defined UE_ROOT (
  echo [PASS45 MATERIAL] ERROR: Unreal Engine 5.8 was not found.
  exit /b 2
)

set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not exist "%UPROJECT%" (
  echo [PASS45 MATERIAL] ERROR: project not found: %UPROJECT%
  exit /b 3
)

if not exist "%~dp0..\Logs" mkdir "%~dp0..\Logs"
if not exist "%REPORT_DIR%" mkdir "%REPORT_DIR%"
if exist "%WEAPON_REPORT%" del /q "%WEAPON_REPORT%" >nul 2>nul
if exist "%WEAPON_SENTINEL%" del /q "%WEAPON_SENTINEL%" >nul 2>nul
if exist "%MATERIAL_LOG%" del /q "%MATERIAL_LOG%" >nul 2>nul

set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"

echo [PASS45 MATERIAL] Running headless authored material/dependency gate...
"%UE_CMD%" "%UPROJECT%" "%VISUAL_MAP%" -game -NoFrontend ^
  -ValidateProductionWeapons -ValidateProductionWeaponsHeadless ^
  -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%MATERIAL_LOG%"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo [PASS45 MATERIAL] FAIL: Unreal headless gate exited with code %RC%.
  echo Log: %MATERIAL_LOG%
  exit /b %RC%
)

if not exist "%WEAPON_REPORT%" (
  echo [PASS45 MATERIAL] FAIL: weapon dependency report is missing.
  exit /b 10
)
if not exist "%WEAPON_SENTINEL%" (
  echo [PASS45 MATERIAL] FAIL: required-available weapon success sentinel is missing.
  exit /b 11
)

findstr /L /C:"PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS" "%WEAPON_SENTINEL%" >nul || goto :weapon_fail
findstr /L /C:"PASS45_AUTHORED_WEAPON_MATERIALS=PASS" "%WEAPON_SENTINEL%" >nul || goto :weapon_fail
findstr /L /C:"PASS45_WEAPON_DEPENDENCY_REPORT=PASS" "%WEAPON_SENTINEL%" >nul || goto :weapon_fail
findstr /L /C:"PASS45_EXACT_PRODUCTION_CONTENT_GAPS=" "%WEAPON_SENTINEL%" >nul || goto :weapon_fail
findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY" "%MATERIAL_LOG%" >nul || goto :weapon_fail
findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL" "%MATERIAL_LOG%" >nul && goto :weapon_fail

findstr /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL" "%MATERIAL_LOG%" >nul && goto :vehicle_fail
findstr /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP" "%MATERIAL_LOG%" >nul && goto :vehicle_fail
findstr /C:"PASS45_PRODUCTION_VEHICLE_CONTENT_GAP" "%MATERIAL_LOG%" >nul && goto :vehicle_fail
findstr /C:"PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY" "%MATERIAL_LOG%" >nul || goto :vehicle_fail
findstr /C:"PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY" "%MATERIAL_LOG%" >nul || goto :vehicle_fail

echo [PASS45 MATERIAL] PASS: required available weapon visuals/material dependencies and HMMWV/M2/BTR material slots validated.
findstr /L /C:"PASS45_EXACT_PRODUCTION_CONTENT_GAPS=" "%WEAPON_SENTINEL%"
echo [PASS45 MATERIAL] Exact weapon payload gaps remain CONTENT GAP and are not called production-ready.
echo Weapon report: %WEAPON_REPORT%
echo Runtime log:   %MATERIAL_LOG%
exit /b 0

:weapon_fail
echo [PASS45 MATERIAL] FAIL: required available weapon authored material/dependency gate did not pass.
echo Exact production payload gaps are allowed only when the explicit real fallback passes the same material/texture checks.
echo Sentinel: %WEAPON_SENTINEL%
echo Report:   %WEAPON_REPORT%
echo Log:      %MATERIAL_LOG%
if exist "%WEAPON_REPORT%" type "%WEAPON_REPORT%"
exit /b 12

:vehicle_fail
echo [PASS45 MATERIAL] FAIL: production HMMWV/M2/BTR authored material validation did not pass.
echo Log: %MATERIAL_LOG%
findstr /C:"PASS45_PRODUCTION_VEHICLE_" "%MATERIAL_LOG%"
exit /b 13
