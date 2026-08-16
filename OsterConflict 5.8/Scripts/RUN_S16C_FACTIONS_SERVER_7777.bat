@echo off
set EXE=%~dp0..\Binaries\Win64\OsterConflictServer.exe
if not exist "%EXE%" (
  echo Missing %EXE%
  echo Build/package the UE 5.8 Server target first.
  pause
  exit /b 1
)
"%EXE%" OsterCenter?listen?MaxPlayers=16?Population=16?BotFill=1?Team1Faction=UA?Team2Faction=Masked -port=7777 -log
