@echo off
set EXE=..\Binaries\Win64\OsterConflictServer.exe
%EXE% /Engine/Maps/Entry?Bots=8?BotDifficulty=Normal -log -port=7777
