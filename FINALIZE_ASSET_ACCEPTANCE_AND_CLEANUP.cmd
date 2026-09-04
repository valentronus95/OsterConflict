@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "FINALIZER=%~dp0OsterConflict\Scripts\finalize_asset_acceptance.py"
set "PY_CMD="

if not exist "%FINALIZER%" (
  echo [STOP] Відсутній finalizer: %FINALIZER%
  exit /b 2
)

where py >nul 2>nul
if not errorlevel 1 set "PY_CMD=py -3"
if not defined PY_CMD (
  where python >nul 2>nul
  if not errorlevel 1 set "PY_CMD=python"
)
if not defined PY_CMD (
  echo [STOP] Python 3 не знайдений.
  exit /b 3
)

echo ============================================================
echo OSTER CONFLICT - FINAL ASSET ACCEPTANCE
echo ============================================================
echo.
echo Цей крок запускається ТІЛЬКИ після успішного ПОВНОГО RUNTIME-ТЕСТУ
echo і після того, як ви власними очима перевірили assets у грі/UE.
echo.
echo Скрипт ще раз перевірить exact HEAD, automated PASS, bindings,
echo production assets та SHA-256 кожного source ZIP.
echo Непідтверджені ZIP не видаляються.
echo.
choice /C YN /N /M "Ви дійсно завершили прямий візуальний огляд і приймаєте результат? [Y/N]: "
if errorlevel 2 (
  echo [CANCEL] Visual acceptance не записано. ZIP не видалялись.
  exit /b 0
)

%PY_CMD% "%FINALIZER%" --accept-visual
set "RC=%ERRORLEVEL%"
if not "%RC%"=="0" (
  echo [STOP] Final asset acceptance не завершено. Код: %RC%
  exit /b %RC%
)

echo [PASS] Visual acceptance і safe source ZIP cleanup завершені.
exit /b 0
