@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

cls
echo ============================================================
echo OSTER CONFLICT - MUSEUM OSTER R14.5 TEST
echo ============================================================
echo.
echo This launcher is dedicated to branch: museum-oster
echo It does NOT launch the legacy R11 frontend/listen-server flow.
echo.

if not exist "%~dp0OsterConflict\VALIDATE_MUSEUM_UE58.cmd" (
    echo ERROR: Museum validation launcher was not found:
    echo %~dp0OsterConflict\VALIDATE_MUSEUM_UE58.cmd
    echo.
    echo Update the museum-oster branch in GitHub Desktop and try again.
    pause
    exit /b 2
)

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
