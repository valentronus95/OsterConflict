@echo off
setlocal EnableExtensions
cd /d "%~dp0"

echo ============================================================
echo OSTER CONFLICT - SILPO OSTER R14 VALIDATION
echo Branch target: current main R14 integration
echo Static contracts + UE 5.8 Editor build
echo ============================================================
echo.

set "PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)

if not defined PY_CMD (
  echo [ERROR] Python 3 not found in PATH.
  echo Static Silpo validation could not run.
  pause
  exit /b 2
)

echo [1/2] Silpo source/reference contracts...
%PY_CMD% "OsterConflict\Tools\validate_silpo_location.py"
if errorlevel 1 (
  echo.
  echo [FAIL] Silpo static validation failed. UE build not started.
  pause
  exit /b 3
)

echo.
echo [PASS] Static Silpo contracts.
echo.

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"

if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
  echo [ERROR] UE 5.8 Build.bat not found at:
  echo %UE_ROOT%
  echo Edit UE_ROOT in VALIDATE_SILPO_UE58.cmd if UE is installed elsewhere.
  pause
  exit /b 4
)

if not exist "%PROJECT%" (
  echo [ERROR] Project not found:
  echo %PROJECT%
  pause
  exit /b 5
)

echo [2/2] Building OsterConflictEditor Win64 Development with UE 5.8...
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "RC=%ERRORLEVEL%"

echo.
if "%RC%"=="0" (
  echo ============================================================
  echo SILPO VALIDATION BUILD PASSED
  echo Next runtime check: START_HERE option 5.
  echo Check R14.0 shell, R14.1 site details, R14.2 interior detail,
  echo R14.3 facade identity, the single entrance door and navigation.
  echo Confirm Silpo is separate from Museum and no legacy civic composite appears.
  echo ============================================================
) else (
  echo ============================================================
  echo SILPO UE BUILD FAILED with exit code %RC%
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  echo ============================================================
)

pause
exit /b %RC%
