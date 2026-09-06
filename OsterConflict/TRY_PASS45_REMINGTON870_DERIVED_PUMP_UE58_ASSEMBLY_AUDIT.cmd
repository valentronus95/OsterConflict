@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_ROOT=%PROJECT_DIR%..\"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "SCRIPT=%REPO_ROOT%PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py"
set "SOURCE=%REPO_ROOT%SOURCE_ASSETS\PASS45\Remington870\remington_870_8siandude_ccby4.glb"
set "LOG=%PROJECT_DIR%Saved\Logs\Pass45Remington870DerivedPumpUE58AssemblyAudit.log"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.8" /v InstalledDirectory 2^>nul ^| find "InstalledDirectory"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%%B"
    )
)
if not defined UE_ROOT (
    for /f "tokens=2,*" %%A in ('reg query "HKCU\SOFTWARE\Epic Games\Unreal Engine\Builds" 2^>nul ^| findstr /i "5.8 UE_5.8"') do (
        if exist "%%B\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" set "UE_ROOT=%%B"
    )
)
if not defined UE_ROOT (
    echo ERROR: Unreal Engine 5.8 installation was not found.
    exit /b 2
)

set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if not exist "%COMMANDLET_UPROJECT%" (
    echo ERROR: isolated Pass45 commandlet project not found: %COMMANDLET_UPROJECT%
    exit /b 3
)
if not exist "%SCRIPT%" (
    echo ERROR: Remington derived-pump assembly audit script not found: %SCRIPT%
    exit /b 4
)
if not exist "%SOURCE%" (
    echo ERROR: exact registered Remington donor not found: %SOURCE%
    exit /b 5
)
for %%F in ("%SOURCE%") do set "SOURCE_SIZE=%%~zF"
if %SOURCE_SIZE% LSS 1000000 (
    echo ERROR: registered Remington donor looks like a Git LFS pointer rather than the exact payload.
    echo Use the repository's normal Git LFS path first. No automatic working-tree mutation is performed here.
    exit /b 6
)

if exist "%LOG%" del /q "%LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 REMINGTON 870 UE 5.8 ASSEMBLY AUDIT
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo Source:  %SOURCE%
echo.
echo [PASS45] ASSEMBLY EVIDENCE ONLY.
echo [PASS45] The base derived-pump import/motion pilot runs first in the same isolated process.
echo [PASS45] No production package save, no gameplay cutover, no item-16 acceptance.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%LOG%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo.
    echo ERROR: Remington derived-pump UE 5.8 assembly audit failed with code !RC!.
    echo Log: %LOG%
    exit /b 7
)

if not exist "%LOG%" (
    echo ERROR: Remington assembly-audit log was not created.
    exit /b 8
)

findstr /L /C:"PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_PASS" "%LOG%" >nul || goto :audit_failed
findstr /L /C:"PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_PASS" "%LOG%" >nul || goto :audit_failed
findstr /L /C:"production_visual_completeness=UNPROVEN" "%LOG%" >nul || goto :audit_failed
findstr /L /C:"production_cutover=0 runtime_acceptance=0 item16_checked=0" "%LOG%" >nul || goto :audit_failed

echo.
echo PASS: UE 5.8 preserved the derived pump proof and recorded the imported Remington assembly shape.
echo STATUS: ASSEMBLY EVIDENCE ONLY. Production visual completeness and gameplay acceptance remain pending.
echo Log: %LOG%
exit /b 0

:audit_failed
echo.
echo ERROR: UE commandlet returned success but required Remington assembly evidence markers are incomplete.
echo Log: %LOG%
exit /b 9
