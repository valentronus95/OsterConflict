@echo off
setlocal
cd /d "%~dp0.."
echo [Oster Conflict] STEP 1/2 - downloading verified gameplay content sources
call "%~dp0FETCH_GAMEPLAY_CONTENT_58.cmd" %*
if errorlevel 1 exit /b %ERRORLEVEL%
echo.
echo [Oster Conflict] STEP 2/2 - scanning local Fab content and external Unreal projects
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0SCAN_LOCAL_GAMEPLAY_CONTENT_58.ps1"
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo.
  echo [FAIL] Local content inventory failed with exit code %RC%.
  exit /b %RC%
)
echo.
echo [PASS] Download and local content inventory completed.
echo Review: SourceAssets\ThirdParty\Gameplay and SourceAssets\ThirdParty\LocalInventory
exit /b 0
