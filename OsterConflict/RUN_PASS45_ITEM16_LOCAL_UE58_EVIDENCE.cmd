@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "M700=%PROJECT_DIR%TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd"
set "REMINGTON=%PROJECT_DIR%TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd"
set "LEVER=%PROJECT_DIR%TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd"
set "AUDIO=%PROJECT_DIR%PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd"

echo ============================================================
echo OSTER CONFLICT - PASS45 ITEM 16 LOCAL UE 5.8 EVIDENCE CHAIN
echo ============================================================
echo [PASS45] Fail-closed local evidence chain. This does NOT accept item 16.
echo [PASS45] Motion phases are isolated proof-only and do not save production packages.
echo [PASS45] Audio phase saves only the two repository-owned Bolt/Lever donor SoundWave assets under /Game/PASS45/Audio/ManualAction.
echo [PASS45] No Git commands are run by this orchestrator.
echo.

for %%F in ("%M700%" "%REMINGTON%" "%LEVER%" "%AUDIO%") do (
    if not exist "%%~F" (
        echo ERROR: required PASS45 launcher is missing: %%~F
        exit /b 90
    )
)

echo [1/4] M700 - bounded BOLT translation motion proof only
call "%M700%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo ERROR: item-16 evidence chain stopped at M700. rc=!RC!
    exit /b 91
)

echo.
echo [2/4] Remington 870 - derived pump motion + imported assembly evidence only
call "%REMINGTON%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo ERROR: item-16 evidence chain stopped at Remington 870. rc=!RC!
    exit /b 92
)

echo.
echo [3/4] Lever Action - LEVER moving-part motion proof only
call "%LEVER%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo ERROR: item-16 evidence chain stopped at Lever Action. rc=!RC!
    exit /b 93
)

echo.
echo [4/4] Manual-action audio - UE 5.8 import/save + independent fresh-load
echo [PASS45] This is the ONLY save-bearing phase in this orchestrator.
echo [PASS45] It is restricted to SW_PASS45_BoltAction_CC0_Donor and SW_PASS45_LeverAction_CC0_Donor.
call "%AUDIO%"
set "RC=!ERRORLEVEL!"
if not "!RC!"=="0" (
    echo ERROR: item-16 evidence chain stopped at manual-action audio. rc=!RC!
    exit /b 94
)

echo.
echo PASS45_ITEM16_LOCAL_UE58_EVIDENCE_CHAIN_COMPLETE
echo STATUS: EVIDENCE CHAIN COMPLETE, ITEM 16 STILL OPEN.
echo M700: translation proof obtained by launcher; travel calibration and bolt rotation remain pending.
echo Remington870: derived pump/assembly proof obtained by launcher; production visual completeness and gameplay acceptance remain pending.
echo LeverAction: moving-part proof obtained by launcher; pilot-angle calibration and production cutover remain pending.
echo Audio: Bolt/Lever donor SoundWaves imported and fresh-loaded; audibility, timing and final mix remain pending.
echo runtime_visual_acceptance=0
echo runtime_acceptance=0
echo item16_checked=0
echo merge_permitted=0
exit /b 0
