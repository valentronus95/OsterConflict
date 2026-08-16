@echo off
setlocal
cd /d "%~dp0"
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"

if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
  echo [ERROR] UE 5.8 not found at:
  echo %UE_ROOT%
  echo Edit UE_ROOT in this .cmd if your UE is installed elsewhere.
  pause
  exit /b 2
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found:
  echo %PROJECT%
  pause
  exit /b 3
)

echo Cleaning generated project build folders from the failed attempt...
if exist "%~dp0OsterConflict\Binaries" rmdir /s /q "%~dp0OsterConflict\Binaries"
if exist "%~dp0OsterConflict\Intermediate" rmdir /s /q "%~dp0OsterConflict\Intermediate"

echo.
echo Building OsterConflictEditor with installed UE 5.8...
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "RC=%ERRORLEVEL%"
echo.
if "%RC%"=="0" (
  echo BUILD SUCCESSFUL
  echo You can now open OsterConflict\OsterConflict.uproject in UnrealEditor.exe
) else (
  echo BUILD FAILED with exit code %RC%
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
)
pause
exit /b %RC%
