@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "UPROJECT=%PROJECT_DIR%OsterConflict.uproject"
set "LOG_FILE=%PROJECT_DIR%Saved\Logs\OsterConflict.log"
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
set "UE_EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

if not exist "%UPROJECT%" (
    echo ERROR: project not found: %UPROJECT%
    exit /b 3
)
if not exist "%UE_EDITOR%" (
    echo ERROR: UnrealEditor.exe not found: %UE_EDITOR%
    exit /b 4
)

cls
echo ============================================================
echo OSTER CONFLICT - MUSEUM OSTER R14.5 VALIDATION - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Project: %UPROJECT%
echo Branch:  museum-oster / PR #16
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
echo PASS: UE 5.8 editor build completed.
echo.
echo [2/3] Launching museum Sandbox visual/runtime validation...
echo Close the game window after the walkaround and interaction test.
echo.
echo REQUIRED IN-GAME CHECKS:
echo   - walk around all four museum sides;
echo   - enter through the ornate main double door;
echo   - open/close the service double door;
echo   - shoot several ground-floor, vestibule, upper-room and gable windows;
echo   - check that broken glass collision disappears;
echo   - check stairs, floor and wall collision;
echo   - inspect gutters/downspouts, annex and vegetation overlap;
echo   - confirm the central slab path remains clear.
echo.

if exist "%LOG_FILE%" del /q "%LOG_FILE%" >nul 2>nul
set "VISUAL_MAP=/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1"
"%UE_EDITOR%" "%UPROJECT%" "%VISUAL_MAP%" -game -NoFrontend -log -windowed -ResX=1600 -ResY=900
set "GAME_RC=%ERRORLEVEL%"

if not exist "%LOG_FILE%" (
    echo.
    echo ERROR: runtime log not found: %LOG_FILE%
    exit /b 10
)

echo.
echo [3/3] Inspecting runtime log...
findstr /I /C:"Cannot replace existing object of a different class" "%LOG_FILE%" >nul && goto :object_collision
findstr /I /C:"Fatal error" "%LOG_FILE%" >nul && goto :fatal_runtime
findstr /I /C:"R14.5 museum validation FAILED" "%LOG_FILE%" >nul && goto :museum_failed
findstr /L /C:"R14.5 museum validation PASS" "%LOG_FILE%" >nul || goto :museum_missing

echo.
echo ============================================================
echo PASS: museum runtime validator reached R14.5 PASS.
echo No fatal UObject class/name replacement collision was found.
echo ============================================================
echo Log: %LOG_FILE%
echo.
echo NOTE: visual fidelity, door feel, bullet response and multiplayer behavior
echo still require human inspection; this script only confirms build/runtime contracts.
exit /b 0

:object_collision
echo.
echo ERROR: UObject class/name replacement collision returned.
findstr /I /C:"Cannot replace existing object of a different class" "%LOG_FILE%"
echo Log: %LOG_FILE%
exit /b 21

:fatal_runtime
echo.
echo ERROR: Fatal runtime error found in museum test log.
findstr /I /C:"Fatal error" "%LOG_FILE%"
echo Log: %LOG_FILE%
exit /b 20

:museum_failed
echo.
echo ERROR: R14.5 museum runtime validator reported FAILED.
findstr /L /C:"R14.5 museum validation FAILED" "%LOG_FILE%"
echo Log: %LOG_FILE%
exit /b 22

:museum_missing
echo.
echo ERROR: R14.5 museum PASS marker was not found.
echo The museum layer may not have spawned, validation may not have completed,
echo or the game may have been closed before the delayed validation ran.
echo Log: %LOG_FILE%
exit /b 23
