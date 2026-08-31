@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "BUILD_BAT=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "LOG_DIR=%~dp0Logs"
set "PREVIEW_LOG=%LOG_DIR%\PASS45_FAST_PREVIEW.log"

if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if exist "%PREVIEW_LOG%" del /q "%PREVIEW_LOG%" >nul 2>nul

if not exist "%BUILD_BAT%" (
  echo [ERROR] UE 5.8 Build.bat not found: %BUILD_BAT%
  pause
  exit /b 2
)
if not exist "%EDITOR%" (
  echo [ERROR] UnrealEditor.exe not found: %EDITOR%
  pause
  exit /b 3
)
if not exist "%PROJECT%" (
  echo [ERROR] Project not found: %PROJECT%
  pause
  exit /b 4
)

set "CURRENT_BRANCH=unknown"
set "LOCAL_HEAD=unknown"
where git >nul 2>nul
if not errorlevel 1 (
  for /f "delims=" %%B in ('git branch --show-current 2^>nul') do set "CURRENT_BRANCH=%%B"
  for /f "delims=" %%H in ('git rev-parse HEAD 2^>nul') do set "LOCAL_HEAD=%%H"
)

echo ============================================================
echo OSTER CONFLICT - ШВИДКИЙ ПЕРЕГЛЯД
echo ============================================================
echo Branch: %CURRENT_BRANCH%
echo Source: %LOCAL_HEAD%
echo.
echo [PREVIEW] Це тільки швидкий візуальний перегляд поточного коду.
echo [PREVIEW] Тут НЕ виконуються strict LFS/material/weapon/vehicle/evidence acceptance gates.
echo [PREVIEW] Цей режим НІКОЛИ не означає PASS45 RUNTIME ACCEPTED.
echo.
echo [1/2] Incremental build. Unreal Build Tool повторно використовує вже зібрані object files.
call "%BUILD_BAT%" OsterConflictEditor Win64 Development -Project="%PROJECT%" -WaitMutex
set "BUILD_RC=%ERRORLEVEL%"
if not "%BUILD_RC%"=="0" (
  echo.
  echo [ERROR] Preview build failed with exit code %BUILD_RC%.
  echo UBT log: %LOCALAPPDATA%\UnrealBuildTool\Log.txt
  pause
  exit /b %BUILD_RC%
)

echo.
echo [2/2] Launching current Oster runtime without strict import/preflight passes...
echo Renderer: DX11 + SM5 + HDR off, max 60 FPS.
echo Log: %PREVIEW_LOG%
echo [PREVIEW] Loading presentation is rendered INSIDE Unreal. No external progress window and no fake percentage.
start /wait "Oster Conflict Fast Preview" "%EDITOR%" "%PROJECT%" "/Game/Maps/OsterConflict_Runtime" -game -Frontend -d3d11 -sm5 -nohdr -NoScreenMessages -abslog="%PREVIEW_LOG%" -fullscreen -ResX=1600 -ResY=900 -ExecCmds="t.MaxFPS 60" -culture=uk-UA
set "GAME_RC=%ERRORLEVEL%"

echo.
echo ============================================================
echo FAST PREVIEW FINISHED - exit code %GAME_RC%
echo Branch: %CURRENT_BRANCH%
echo Source: %LOCAL_HEAD%
echo This run is PREVIEW ONLY, runtime acceptance remains unchanged.
echo Diagnostic log: %PREVIEW_LOG%
echo ============================================================
pause
exit /b %GAME_RC%
