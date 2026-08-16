@echo off
setlocal
set EXE=%~dp0..\Binaries\Win64\OsterConflictClient.exe
if not exist "%EXE%" (echo Client executable not found: %EXE% & echo Build the Client target first. & pause & exit /b 1)
"%EXE%" 127.0.0.1:7777?Name=PerfClient -windowed -ResX=1600 -ResY=900 -trace=default,net -statnamedevents -log
