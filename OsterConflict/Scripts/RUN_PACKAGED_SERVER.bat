@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictServer.exe
if not exist "%EXE%" (
  echo OsterConflictServer.exe not found.
  echo Build/cook the Development Server target first.
  pause
  exit /b 1
)
"%EXE%" -log -port=7777
