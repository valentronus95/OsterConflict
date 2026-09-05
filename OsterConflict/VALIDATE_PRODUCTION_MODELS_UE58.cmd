@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "REPORT_DIR=%PROJECT_DIR%Saved\AutomationReports\ProductionModels"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_automation_success.txt"
set "WEAPON_RUNTIME_REPORT=%REPORT_DIR%\weapon_runtime_validation.txt"
set "WEAPON_RUNTIME_SENTINEL=%REPORT_DIR%\production_weapon_runtime_success.txt"
set "STEIN_REIMPORT_CMD=%PROJECT_DIR%PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" (
    set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
)

if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" (
    set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
)

if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
        if exist "%%B\Engine\Build\BatchFiles\Build.bat" set "UE_ROOT=%%B"
    )
)

if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "5.8 UE_5.8"') do (
        if exist "%%B\Engine\Build\BatchFiles\Build.bat" set "UE_ROOT=%%B"
    )
)

if not defined UE_ROOT (
    echo ERROR: Unreal Engine 5.8 installation was not found.
    exit /b 2
)

set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "UE_EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

if not exist "%UPROJECT%" (
    echo ERROR: project not found: %UPROJECT%
    exit /b 3
)
if not exist "%UE_CMD%" (
    echo ERROR: UnrealEditor-Cmd.exe not found: %UE_CMD%
    exit /b 4
)
if not exist "%UE_EDITOR%" (
    echo ERROR: UnrealEditor.exe not found: %UE_EDITOR%
    exit /b 5
)
if not exist "%STEIN_REIMPORT_CMD%" (
    echo ERROR: Pass45 Stein authored-material reimport command is missing: %STEIN_REIMPORT_CMD%
    exit /b 6
)

if exist "%REPORT_DIR%" rmdir /s /q "%REPORT_DIR%"
if exist "%SUCCESS_SENTINEL%" del /q "%SUCCESS_SENTINEL%" >nul 2>nul

mkdir "%REPORT_DIR%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - R14 PRODUCTION MODEL VALIDATION - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Project: %UPROJECT%
echo.

echo [1/5] Building OsterConflictEditor Development Win64...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%UPROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
    echo.
    echo ERROR: OsterConflictEditor build failed with code %BUILD_RC%.
    echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
    exit /b %BUILD_RC%
)

echo.
echo [2/5] Running OsterConflict.ProductionModels automation tests...
"%UE_CMD%" "%UPROJECT%" -unattended -nop4 -nosplash -nullrhi -stdout ^
    -ExecCmds="Automation RunTest OsterConflict.ProductionModels;Quit" ^
    -ReportExportPath="%REPORT_DIR%"
set "TEST_RC=%ERRORLEVEL%"

if not "%TEST_RC%"=="0" (
    echo.
    echo ERROR: Unreal automation process failed with code %TEST_RC%.
    echo Report: %REPORT_DIR%
    exit /b %TEST_RC%
)

if not exist "%SUCCESS_SENTINEL%" (
    echo.
    echo ERROR: production automation success sentinel is missing.
    echo The canonical production asset test did not fully pass.
    echo Report: %REPORT_DIR%
    exit /b 10
)

findstr /L /C:"Ukrainian HMMWV=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed
findstr /L /C:"M2 Browning=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed
findstr /L /C:"BTR-4 Bucephalus=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed
findstr /L /C:"Anti-Armor Launcher=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed

echo.
echo PASS: production asset automation checks passed.
echo Report: %REPORT_DIR%
echo.
echo [3/5] Reimporting Stein authored textures/materials with the Pass45 texture-first contract...
call "%STEIN_REIMPORT_CMD%"
set "STEIN_RC=%ERRORLEVEL%"
if not "%STEIN_RC%"=="0" (
    echo.
    echo ERROR: Pass45 Stein authored-material reimport failed with code %STEIN_RC%.
    exit /b %STEIN_RC%
)

echo.
echo [4/5] Running headless production-weapon authored-material runtime gate...
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"
if exist "%WEAPON_RUNTIME_REPORT%" del /q "%WEAPON_RUNTIME_REPORT%" >nul 2>nul
if exist "%WEAPON_RUNTIME_SENTINEL%" del /q "%WEAPON_RUNTIME_SENTINEL%" >nul 2>nul

"%UE_CMD%" "%UPROJECT%" "%VISUAL_MAP%" -game -NoFrontend ^
    -ValidateProductionWeapons -ValidateProductionWeaponsHeadless ^
    -unattended -nop4 -nosplash -nullrhi -stdout
set "WEAPON_RUNTIME_RC=%ERRORLEVEL%"

if not "%WEAPON_RUNTIME_RC%"=="0" (
    echo.
    echo ERROR: headless production-weapon runtime process failed with code %WEAPON_RUNTIME_RC%.
    echo Report: %WEAPON_RUNTIME_REPORT%
    exit /b %WEAPON_RUNTIME_RC%
)

if not exist "%WEAPON_RUNTIME_REPORT%" goto :weapon_runtime_failed

rem Gate every canonical weapon whose source/content actually exists in this repository.
rem Missing Remington870/M249 production payload is an explicit CONTENT GAP, never READY and never a fake material repair.
call :require_weapon_pass "AK-47" || goto :weapon_runtime_failed
call :require_weapon_pass "MP5" || goto :weapon_runtime_failed
call :require_weapon_pass "M1911" || goto :weapon_runtime_failed
call :require_weapon_pass "M700" || goto :weapon_runtime_failed
call :require_weapon_pass "M14" || goto :weapon_runtime_failed
call :require_weapon_pass "MAC-10" || goto :weapon_runtime_failed
call :require_weapon_pass "TEC-9" || goto :weapon_runtime_failed
call :require_weapon_pass "Lever Action .45-70" || goto :weapon_runtime_failed
call :require_weapon_pass "Anti-Armor Launcher" || goto :weapon_runtime_failed

if exist "%PROJECT_DIR%Content\Production\Weapons\Remington870\SM_Remington870.uasset" (
    call :require_weapon_pass "Remington 870" || goto :weapon_runtime_failed
) else (
    echo CONTENT GAP: Remington 870 exact production payload is absent; it is not counted READY.
)

if exist "%PROJECT_DIR%Content\Production\Weapons\M249\SM_M249.uasset" (
    call :require_weapon_pass "M249" || goto :weapon_runtime_failed
) else (
    echo CONTENT GAP: M249 exact production payload is absent; it is not counted READY.
)

echo PASS: every repository-available canonical weapon passed mesh + authored material + runtime material dependency checks.
echo CONTENT GAP weapons remain explicitly not READY until their exact production payload exists.
echo Report: %WEAPON_RUNTIME_REPORT%
echo.
echo [5/5] Launching standalone Sandbox visual check...
start "Oster Conflict - R14 Production Model Visual Check" "%UE_EDITOR%" "%UPROJECT%" "%VISUAL_MAP%" -game -NoFrontend -ValidateProductionWeapons -log -windowed -ResX=1600 -ResY=900

echo.
echo ============================================================
echo R14 AUTOMATION + AVAILABLE-WEAPON MATERIAL GATE PASS. Visual Sandbox launched.
echo Check the rack in rendered gameplay: authored appearance is mandatory and white/default slots fail.
echo Also check HMMWV scale/materials, M2 pivot+muzzle and BTR-4 shell/materials.
echo This command does NOT promote Pass45 to VERIFIED RUNTIME by itself.
echo ============================================================
exit /b 0

:require_weapon_pass
set "REQUIRED_WEAPON_LABEL=%~1"
findstr /L /B /C:"%REQUIRED_WEAPON_LABEL% |" "%WEAPON_RUNTIME_REPORT%" | findstr /L /C:"RESULT=PASS" >nul
if errorlevel 1 (
    echo ERROR: repository-available weapon failed authored-material runtime gate: %REQUIRED_WEAPON_LABEL%
    exit /b 1
)
exit /b 0

:test_failed
echo.
echo ERROR: production automation sentinel is incomplete.
echo File: %SUCCESS_SENTINEL%
echo Report: %REPORT_DIR%
exit /b 11

:weapon_runtime_failed
echo.
echo ERROR: R14 repository-available production-weapon authored-material gate did not pass.
echo Report:   %WEAPON_RUNTIME_REPORT%
echo Sentinel: %WEAPON_RUNTIME_SENTINEL%
if exist "%WEAPON_RUNTIME_REPORT%" type "%WEAPON_RUNTIME_REPORT%"
exit /b 12
