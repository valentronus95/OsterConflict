@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "REPORT_DIR=%PROJECT_DIR%Saved\AutomationReports\ProductionModels"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_automation_success.txt"
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

if exist "%REPORT_DIR%" rmdir /s /q "%REPORT_DIR%"
if exist "%SUCCESS_SENTINEL%" del /q "%SUCCESS_SENTINEL%" >nul 2>nul

mkdir "%REPORT_DIR%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PRODUCTION MODEL VALIDATION - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Project: %UPROJECT%
echo.

echo [1/3] Building OsterConflictEditor Development Win64...
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%UPROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
    echo.
    echo ERROR: OsterConflictEditor build failed with code %BUILD_RC%.
    echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
    exit /b %BUILD_RC%
)

echo.
echo [2/3] Running OsterConflict.ProductionModels automation tests...
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
    echo The canonical HMMWV/M2/BTR-4 asset test did not fully pass.
    echo Report: %REPORT_DIR%
    exit /b 10
)

findstr /L /C:"Ukrainian HMMWV=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed
findstr /L /C:"M2 Browning=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed
findstr /L /C:"BTR-4 Bucephalus=PASS" "%SUCCESS_SENTINEL%" >nul || goto :test_failed

echo.
echo PASS: production asset automation checks passed.
echo Report: %REPORT_DIR%
echo.
echo [3/3] Launching standalone Sandbox visual check...
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"
start "Oster Conflict - Production Model Visual Check" "%UE_EDITOR%" "%UPROJECT%" "%VISUAL_MAP%" -game -NoFrontend -log -windowed -ResX=1600 -ResY=900

echo.
echo ============================================================
echo AUTOMATION PASS. Visual Sandbox launched.
echo Check HMMWV scale/materials, M2 pivot+muzzle, BTR-4 shell/materials,
echo and first-person hands/ADS/reload before marking PR #12 ready.
echo ============================================================
exit /b 0

:test_failed
echo.
echo ERROR: production automation sentinel is incomplete.
echo File: %SUCCESS_SENTINEL%
echo Report: %REPORT_DIR%
exit /b 11
