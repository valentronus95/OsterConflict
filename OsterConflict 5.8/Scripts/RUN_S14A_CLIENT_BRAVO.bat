@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictClient.exe
if not exist "%EXE%" (
  echo Client executable not found: %EXE%
  pause
  exit /b 1
)
"%EXE%" "127.0.0.1:7777?Name=Bravo" -windowed -ResX=1280 -ResY=720 -WinX=100 -WinY=100 -log
