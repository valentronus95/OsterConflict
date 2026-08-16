@echo off
setlocal
cd /d "%~dp0"
if "%UE_ROOT%"=="" if exist "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if "%UE_ROOT%"=="" set /p UE_ROOT=Enter UE 5.8 installation root: 
if "%UE_ROOT%"=="" exit /b 2
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0PC_TEST\RUN_UE58_PC_VALIDATION.ps1" -UERoot "%UE_ROOT%" -Mode Compile
set RC=%ERRORLEVEL%
echo.
echo Compile-only exit code: %RC%
echo Results are under PC_TEST\TEST_RESULTS
pause
exit /b %RC%
