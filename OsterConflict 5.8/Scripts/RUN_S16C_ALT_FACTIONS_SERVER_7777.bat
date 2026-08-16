@echo off
set EXE=%~dp0..\Binaries\Win64\OsterConflictServer.exe
if not exist "%EXE%" (
  echo Missing %EXE%
  pause
  exit /b 1
)
"%EXE%" OsterCenter?listen?MaxPlayers=16?Population=16?BotFill=1?Team1Faction=Rangers?Team2Faction=Insurgents -port=7777 -log
