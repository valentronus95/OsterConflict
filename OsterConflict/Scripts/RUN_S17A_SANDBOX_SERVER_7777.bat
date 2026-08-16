@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictServer.exe
if not exist "%EXE%" (
  echo Server executable not found: %EXE%
  echo Build the Server target first.
  pause
  exit /b 1
)
"%EXE%" "/Engine/Maps/Entry?Mode=Sandbox?BotFill=0?Bots=4" -log -port=7777
