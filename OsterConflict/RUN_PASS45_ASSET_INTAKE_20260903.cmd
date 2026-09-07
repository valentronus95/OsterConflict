@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
for %%I in ("%PROJECT_DIR%..") do set "ROOT=%%~fI"
set "AUDITOR=%ROOT%\PASS45_ASSET_INTAKE_20260903.py"

echo ============================================================
echo OSTER CONFLICT - PASS45 ASSET INTAKE AUDIT - 2026-09-03
echo ============================================================
echo [PASS45] Quarantine audit only.
echo [PASS45] No Git commands, no UE import, no production extraction, no deletion.
echo.

if not exist "%AUDITOR%" (
    echo ERROR: missing auditor:
    echo   %AUDITOR%
    exit /b 90
)

set "SOURCE=%~1"
if defined SOURCE (
    echo [PASS45] Explicit source: %SOURCE%
    py -3 "%AUDITOR%" "%SOURCE%"
) else (
    py -3 "%AUDITOR%"
)

set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
    echo.
    echo PASS45_ASSET_INTAKE_AUDIT_FAILED rc=%RC%
    echo runtime_acceptance=0
    echo item16_checked=0
    echo merge_permitted=0
    exit /b %RC%
)

echo.
echo PASS45_ASSET_INTAKE_AUDIT_PASS
echo STATUS: INTAKE REPORT GENERATED; NO ASSET PROMOTED.
echo runtime_acceptance=0
echo item16_checked=0
echo merge_permitted=0
exit /b 0
