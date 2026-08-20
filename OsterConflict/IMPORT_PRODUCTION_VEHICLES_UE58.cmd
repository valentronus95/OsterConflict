@echo off
setlocal EnableExtensions

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "PY_SCRIPT=%PROJECT_DIR%Scripts\import_production_vehicle_assets.py"
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

echo.
echo PASS: HMMWV, M2 Browning and BTR-4 production assets imported and saved.
exit /b 0
