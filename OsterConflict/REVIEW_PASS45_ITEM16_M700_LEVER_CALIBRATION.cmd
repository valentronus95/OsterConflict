@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
for %%I in ("%PROJECT_DIR%..") do set "ROOT=%%~fI"
set "REVIEW=%ROOT%\PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py"

if not exist "%REVIEW%" (
    echo ERROR: missing calibration review tool:
    echo   %REVIEW%
    exit /b 70
)

echo ============================================================
echo OSTER CONFLICT - PASS45 ITEM16 M700 / LEVER CALIBRATION REVIEW
echo ============================================================
echo [PASS45] Reads local UE 5.8 pilot evidence only.
echo [PASS45] Does not author/save production content and does not run full gameplay runtime.
echo.

py -3 "%REVIEW%"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
    echo.
    echo PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_FAILED rc=%RC%
    echo Run the current-head item16 local UE58 evidence chain first.
    echo runtime_acceptance=0
    echo item16_checked=0
    echo merge_permitted=0
    exit /b %RC%
)

echo.
echo PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW_COMPLETE
echo Report:
echo   %ROOT%\PC_TEST\TEST_RESULTS\PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.md
echo STATUS: PILOT EVIDENCE CONSOLIDATED; MANUAL VISUAL CALIBRATION STILL REQUIRED.
echo runtime_acceptance=0
echo item16_checked=0
echo merge_permitted=0
exit /b 0
