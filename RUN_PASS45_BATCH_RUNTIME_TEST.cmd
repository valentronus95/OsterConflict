@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "RUNNER=%~dp0OsterConflict\Scripts\run_pass45_batch_runtime_test.ps1"
if not exist "%RUNNER%" (
  echo [STOP] Відсутній пакетний runtime runner: %RUNNER%
  exit /b 90
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%RUNNER%"
exit /b %ERRORLEVEL%
