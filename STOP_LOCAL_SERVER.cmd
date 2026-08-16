@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0PC_TEST\STOP_LOCAL_SERVER.ps1"
set RC=%ERRORLEVEL%
pause
exit /b %RC%
