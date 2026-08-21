@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

rem Existing local UE 5.8 build helper. START_HERE.cmd remains the only user-facing launcher.
rem Optional environment override:
rem   set UE_ROOT=D:\Epic Games\UE_5.8
rem Optional flags:
rem   /clean    remove project Binaries + Intermediate before building
rem   /nopause  return immediately with the UnrealBuildTool exit code

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "DO_CLEAN=0"
set "NO_PAUSE=0"

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="/clean" set "DO_CLEAN=1"
if /I "%~1"=="/nopause" set "NO_PAUSE=1"
shift
goto parse_args

:args_done
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
  echo [ERROR] UE 5.8 Build.bat not found:
  echo %UE_ROOT%\Engine\Build\BatchFiles\Build.bat
  echo Set UE_ROOT before running this helper if UE 5.8 is installed elsewhere.
  set "RC=2"
  goto finish
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found:
  echo %PROJECT%
  set "RC=3"
  goto finish
)

if "%DO_CLEAN%"=="1" (
  echo Cleaning generated project build folders...
  if exist "%~dp0OsterConflict\Binaries" rmdir /s /q "%~dp0OsterConflict\Binaries"
  if exist "%~dp0OsterConflict\Intermediate" rmdir /s /q "%~dp0OsterConflict\Intermediate"
) else (
  echo Incremental build. Use /clean only when a clean rebuild is actually needed.
)

echo.
echo Building OsterConflictEditor - Win64 Development - UE 5.8...
echo UE_ROOT: %UE_ROOT%
echo PROJECT: %PROJECT%
echo.
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex -NoHotReloadFromIDE
set "RC=%ERRORLEVEL%"
echo.
if "%RC%"=="0" (
  echo BUILD SUCCESSFUL
  echo OsterConflictEditor compiled with installed UE 5.8.
) else (
  echo BUILD FAILED with exit code %RC%
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
)

:finish
if not defined RC set "RC=1"
if "%NO_PAUSE%"=="0" pause
exit /b %RC%
