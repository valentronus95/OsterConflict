@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul
cd /d "%~dp0"

cls
echo ============================================================
echo OSTER CONFLICT - MUSEUM OSTER R14.5 TEST
echo ============================================================
echo.
echo Current-main compatible museum validation launcher.
echo Legacy R11 frontend/listen-server flow is NOT used here.
echo.

where git >nul 2>nul
if "%ERRORLEVEL%"=="0" (
    for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
    if defined CURRENT_BRANCH (
        echo Current Git branch: !CURRENT_BRANCH!
        if /I not "!CURRENT_BRANCH!"=="main" if /I not "!CURRENT_BRANCH!"=="museum-oster" (
            echo.
            echo ERROR: Wrong branch. Use main or museum-oster for this validator.
            pause
            exit /b 3
        )
    )
)

if not exist "%~dp0OsterConflict\VALIDATE_MUSEUM_UE58.cmd" (
    echo ERROR: Museum validation launcher was not found:
    echo %~dp0OsterConflict\VALIDATE_MUSEUM_UE58.cmd
    echo.
    echo Pull current main and try again.
    pause
    exit /b 2
)

if not exist "%~dp0OsterConflict\Source\OsterConflict\Private\OCR145MuseumTreeLayoutSubsystem.cpp" (
    echo ERROR: R14.5 museum source is missing locally.
    echo The working copy is stale. Pull current main first.
    pause
    exit /b 4
)

echo.
echo Starting CURRENT museum build and direct runtime validation...
echo.
call "%~dp0OsterConflict\VALIDATE_MUSEUM_UE58.cmd"
set "RC=%ERRORLEVEL%"

echo.
if "%RC%"=="0" (
    echo ============================================================
    echo MUSEUM TEST COMPLETED SUCCESSFULLY
    echo ============================================================
) else (
    echo ============================================================
    echo MUSEUM TEST FAILED - EXIT CODE %RC%
    echo ============================================================
)
echo.
pause
exit /b %RC%
