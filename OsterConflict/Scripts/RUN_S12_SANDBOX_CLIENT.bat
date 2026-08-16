@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflict.exe
if not exist "%EXE%" (
  echo Client executable not found: %EXE%
  echo Build/package the Client/Game target first.
  pause
  exit /b 1
)
"%EXE%" 127.0.0.1:7777?Name=SandboxTester -log -windowed -ResX=1440 -ResY=900
