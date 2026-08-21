@echo off
setlocal EnableExtensions
cd /d "%~dp0"

echo ============================================================
echo OSTER CONFLICT - SAFE LOCAL MAIN RECOVERY
echo ============================================================
echo.

where git >nul 2>nul || (
  echo [ERROR] Git was not found in PATH.
  pause
  exit /b 2
)

echo [1/4] Fetching origin...
git fetch origin
if errorlevel 1 goto :fail

echo [2/4] Clearing interrupted Git operation state...
git merge --abort >nul 2>nul
git rebase --abort >nul 2>nul
git cherry-pick --abort >nul 2>nul

echo [3/4] Restoring local main exactly to origin/main...
git switch main >nul 2>nul
git reset --hard origin/main
if errorlevel 1 goto :fail

echo [4/4] Refreshing Git LFS objects...
git lfs pull
if errorlevel 1 goto :fail

echo.
echo ============================================================
echo RECOVERY COMPLETE
echo Local branch: main
git log -1 --oneline
echo ============================================================
echo.
echo Generated PC_TEST/Logs files are ignored by repository rules.
echo Stashed Changes are intentionally NOT restored.
pause
exit /b 0

:fail
echo.
echo [ERROR] Recovery stopped. Review the command output above.
pause
exit /b 1
