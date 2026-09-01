@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
for %%I in ("%PROJECT_DIR%..") do set "REPO_ROOT=%%~fI"
set "COMMANDLET_UPROJECT=%PROJECT_DIR%OsterConflictPass45Commandlet.uproject"
set "IMPORT_SCRIPT=%REPO_ROOT%\PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py"
set "FRESH_SCRIPT=%REPO_ROOT%\PASS45_MANUAL_ACTION_AUDIO_UE_FRESH_LOAD.py"
set "BOLT_ASSET=%PROJECT_DIR%Content\PASS45\Audio\ManualAction\SW_PASS45_BoltAction_CC0_Donor.uasset"
set "LEVER_ASSET=%PROJECT_DIR%Content\PASS45\Audio\ManualAction\SW_PASS45_LeverAction_CC0_Donor.uasset"
set "IMPORT_LOG=%PROJECT_DIR%Saved\Logs\Pass45ManualActionAudioImport.log"
set "FRESH_LOG=%PROJECT_DIR%Saved\Logs\Pass45ManualActionAudioFreshLoad.log"
set "UE_ROOT="

if exist "%ProgramFiles%\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_ROOT=%ProgramFiles%\Epic Games\UE_5.8"
)
if not defined UE_ROOT if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" (
    set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
)
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
    exit /b 31
)

set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if not exist "%COMMANDLET_UPROJECT%" (
    echo ERROR: isolated PASS45 commandlet project not found: %COMMANDLET_UPROJECT%
    exit /b 32
)
if not exist "%IMPORT_SCRIPT%" (
    echo ERROR: manual-action audio importer not found: %IMPORT_SCRIPT%
    exit /b 33
)
if not exist "%FRESH_SCRIPT%" (
    echo ERROR: manual-action audio fresh-load verifier not found: %FRESH_SCRIPT%
    exit /b 34
)
if exist "%IMPORT_LOG%" del /q "%IMPORT_LOG%" >nul 2>nul
if exist "%FRESH_LOG%" del /q "%FRESH_LOG%" >nul 2>nul

echo ============================================================
echo OSTER CONFLICT - PASS45 MANUAL-ACTION AUDIO - UE 5.8
echo ============================================================
echo UE:      %UE_ROOT%
echo Host:    %COMMANDLET_UPROJECT%
echo.
echo [PASS45] Importing provenance-pinned CC0 action-family WAV donors through the isolated content commandlet host.
echo [PASS45] This step does not claim exact M700/Stein identity or runtime acceptance.
echo.

"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%IMPORT_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%IMPORT_LOG%"
set "IMPORT_RC=!ERRORLEVEL!"
if not "!IMPORT_RC!"=="0" (
    echo ERROR: manual-action SoundWave import failed with code !IMPORT_RC!.
    echo Log: %IMPORT_LOG%
    exit /b 35
)
if not exist "%BOLT_ASSET%" (
    echo ERROR: imported BoltCycle SoundWave file is missing: %BOLT_ASSET%
    exit /b 36
)
if not exist "%LEVER_ASSET%" (
    echo ERROR: imported LeverCycle SoundWave file is missing: %LEVER_ASSET%
    exit /b 37
)

echo.
echo [VERIFY] Fresh-loading both imported SoundWaves in a second isolated UE 5.8 process...
"%UE_CMD%" "%COMMANDLET_UPROJECT%" -run=pythonscript -script="%FRESH_SCRIPT%" -unattended -nop4 -nosplash -nullrhi -stdout -FullStdOutLogOutput -UTF8Output -abslog="%FRESH_LOG%"
set "FRESH_RC=!ERRORLEVEL!"
if not "!FRESH_RC!"=="0" (
    echo ERROR: manual-action SoundWave fresh-load verification failed with code !FRESH_RC!.
    echo Log: %FRESH_LOG%
    exit /b 38
)

echo.
echo PASS: manual-action BoltCycle/LeverCycle donor SoundWaves were imported and independently fresh-loaded.
echo STATUS: UE ASSET IMPORT/FRESH-LOAD READY; audibility, timing, final mix and authored action animations remain pending.
echo runtime_acceptance=0 item16_checked=0
exit /b 0
