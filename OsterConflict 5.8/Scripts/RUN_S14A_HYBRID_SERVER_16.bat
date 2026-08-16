@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictServer.exe
if not exist "%EXE%" (
  echo Server executable not found: %EXE%
  echo Build the Server target first.
  pause
  exit /b 1
)
"%EXE%" "/Engine/Maps/Entry?MaxPlayers=16?Population=16?BotFill=1?BotDifficulty=Normal" -log -port=7777
