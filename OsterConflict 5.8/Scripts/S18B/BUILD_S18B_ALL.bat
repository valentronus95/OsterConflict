@echo off
setlocal
cd /d "%~dp0\..\.."
if "%UE_ROOT%"=="" (
  echo [S18B] UE_ROOT is not set.
  echo Example: set UE_ROOT=D:\UnrealEngine-5.8
  exit /b 2
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0BuildS18B.ps1" -UERoot "%UE_ROOT%"
exit /b %ERRORLEVEL%
