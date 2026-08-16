@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictClient.exe
if not exist "%EXE%" (
  echo Client executable not found: %EXE%
  echo Build/package the Client target first.
  pause
  exit /b 1
)
"%EXE%" "127.0.0.1:7777?Name=Charlie?Role=Medic" -windowed -ResX=1280 -ResY=720 -WinX=40 -WinY=800 -log
