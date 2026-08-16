@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflict.exe
if not exist "%EXE%" (
  echo Game executable not found: %EXE%
  echo Build/package the Game target first.
  pause
  exit /b 1
)
"%EXE%" "/Engine/Maps/Entry?Mode=Sandbox?Name=SandboxSolo" -log -windowed -ResX=1440 -ResY=900
