@echo off
setlocal
cd /d "%~dp0.."
echo [Oster Conflict] Gameplay content intake
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0FETCH_GAMEPLAY_CONTENT_58.ps1" %*
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo.
  echo [FAIL] Gameplay content intake failed with exit code %RC%.
  exit /b %RC%
)
echo.
echo [PASS] Gameplay content intake completed.
exit /b 0
