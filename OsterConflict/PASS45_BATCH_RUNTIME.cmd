@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0.."

set "BATCH_SCRIPT=%~dp0Scripts\pass45_batch_runtime_progress.py"
set "PY_CMD="
where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)

if not defined PY_CMD (
  echo [STOP] Python 3 не знайдено в PATH.
  exit /b 90
)
if not exist "%BATCH_SCRIPT%" (
  echo [STOP] Progress-wrapper пакетного runtime відсутній: %BATCH_SCRIPT%
  exit /b 91
)

%PY_CMD% "%BATCH_SCRIPT%"
exit /b %ERRORLEVEL%
