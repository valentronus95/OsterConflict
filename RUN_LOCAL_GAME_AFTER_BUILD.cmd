@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0PC_TEST\LAUNCH_LATEST_LOCAL_SESSION.ps1"
set RC=%ERRORLEVEL%
echo.
echo Local launch exit code: %RC%
pause
exit /b %RC%
