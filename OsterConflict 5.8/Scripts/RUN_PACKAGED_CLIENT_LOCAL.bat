@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictClient.exe
if not exist "%EXE%" (
  echo OsterConflictClient.exe not found.
  echo Build/cook the Development Client target first.
  pause
  exit /b 1
)
"%EXE%" 127.0.0.1:7777 -WINDOWED -ResX=1280 -ResY=720
