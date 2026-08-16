@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictClient.exe
if not exist "%EXE%" (
  echo Client executable not found: %EXE%
  pause
  exit /b 1
)
"%EXE%" "127.0.0.1:7777?Name=Alpha" -windowed -ResX=1280 -ResY=720 -WinX=30 -WinY=30 -log
