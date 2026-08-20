@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
set "SUCCESS_SENTINEL=%PROJECT_DIR%Saved\ProductionAssetImportCache\production_import_success.txt"
set "UE_CMD="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_CMD=%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

if not defined UE_CMD if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_CMD=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
)

if not defined UE_CMD (
    for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    )
)

if not defined UE_CMD (
    for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "UE_5.8"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_CMD=%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    )
)

if not defined UE_CMD (
    echo ERROR: Unreal Engine 5.8 UnrealEditor-Cmd.exe was not found.
    echo Expected a Launcher install such as C:\Program Files\Epic Games\UE_5.8.
    exit /b 2
)

if not exist "%UPROJECT%" (
    echo ERROR: Project not found: %UPROJECT%
    exit /b 3
)

if not exist "%PY_SCRIPT%" (
    echo ERROR: Import script not found: %PY_SCRIPT%
    exit /b 4
)

if exist "%SUCCESS_SENTINEL%" del /q "%SUCCESS_SENTINEL%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PRODUCTION VEHICLE IMPORT
echo ============================================================
echo UE:      %UE_CMD%
echo Project: %UPROJECT%
echo Script:  %PY_SCRIPT%
echo.

"%UE_CMD%" "%UPROJECT%" -ExecutePythonScript="%PY_SCRIPT%" -unattended -nop4 -nosplash -nullrhi
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo ERROR: Unreal production asset import failed with code %RESULT%.
    exit /b %RESULT%
)

rem Do not trust the process exit code alone. Python writes this file only after all three import
rem tasks report the canonical assets as created/updated and the assets are saved successfully.
if not exist "%SUCCESS_SENTINEL%" (
    echo.
    echo ERROR: Unreal exited with code 0 but the production import success sentinel is missing.
    echo Treat this as a failed import and inspect the Unreal log.
    exit /b 5
)

findstr /L /C:"/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Weapons/M2/SM_M2_Browning" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel
findstr /L /C:"/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" "%SUCCESS_SENTINEL%" >nul || goto :bad_sentinel

echo.
echo PASS: HMMWV, M2 Browning and BTR-4 production assets imported, verified and saved.
exit /b 0

:bad_sentinel
echo.
echo ERROR: production import success sentinel is incomplete or invalid.
echo File: %SUCCESS_SENTINEL%
exit /b 6
