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
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0PC_TEST\RUN_UE58_PC_VALIDATION.ps1" -UERoot "%UE_ROOT%" -Mode Full
set RC=%ERRORLEVEL%
echo.
echo Validation exit code: %RC%
echo Results are under PC_TEST\TEST_RESULTS
pause
exit /b %RC%
