@echo off
set EXE=..\Binaries\Win64\OsterConflictServer.exe
%EXE% /Engine/Maps/Entry?Bots=16?BotDifficulty=Hard -log -port=7777
