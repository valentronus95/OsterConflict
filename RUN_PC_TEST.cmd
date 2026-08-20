@echo off
setlocal
cd /d "%~dp0"
echo ============================================================
echo OSTER CONFLICT - UE 5.8 PC VALIDATION
echo ============================================================
echo.
if "%UE_ROOT%"=="" if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if "%UE_ROOT%"=="" (
  echo UE_ROOT is not set.
  echo Example:
  echo   set UE_ROOT=D:\UnrealEngine-5.8
  echo.
  set /p UE_ROOT=Enter UE 5.8 root folder: 
)
if "%UE_ROOT%"=="" (
  echo ERROR: UE_ROOT was not provided.
  pause
  exit /b 2
)
set "LFS_CHECK=%~dp0PC_TEST\CHECK_R13_LFS_PAYLOADS.ps1"
if not exist "%LFS_CHECK%" (
  echo ERROR: R13 Git LFS payload checker not found: %LFS_CHECK%
  pause
  exit /b 3
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LFS_CHECK%" -ProjectRoot "%~dp0OsterConflict"
set "LFS_RC=%ERRORLEVEL%"
if not "%LFS_RC%"=="0" (
  pause
  exit /b %LFS_RC%
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0PC_TEST\RUN_UE58_PC_VALIDATION.ps1" -UERoot "%UE_ROOT%" -Mode Full
set RC=%ERRORLEVEL%
echo.
echo Validation exit code: %RC%
echo Results are under PC_TEST\TEST_RESULTS
pause
exit /b %RC%
