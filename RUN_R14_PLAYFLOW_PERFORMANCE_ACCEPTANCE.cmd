@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

echo ============================================================
echo OSTER CONFLICT - PASS 14/29 PLAYFLOW + PERFORMANCE ACCEPTANCE
echo ============================================================
echo.
echo Expected normal flow after Pass 29:
echo   1. Main menu: press START.
echo   2. START must NOT rebuild/open a CREATE SERVER Slate page.
echo   3. START queues the hosted session directly from the stable menu using saved/default host values.
echo   4. After travel, choose TEAM / SQUAD / ROLE / SPAWN and press У БІЙ.
echo   5. Stay in gameplay for at least 16 seconds after spawning so FPS sampling completes.
echo   6. Exit the game normally and let this window inspect the log.
echo.

set "OC_FORCE_ACCEPTANCE=1"
call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"
set "GAME_RC=%ERRORLEVEL%"
if not "%GAME_RC%"=="0" (
    echo [STOP] Base runtime acceptance failed with exit code %GAME_RC%.
    exit /b %GAME_RC%
)

if not exist "%LOG%" (
    echo [STOP] Runtime log is missing: %LOG%
    exit /b 31
)

for %%M in (
    PASS29_MAIN_START_DIRECT_HOST_QUEUED
    PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE
    PASS14_HOST_TRAVEL_BEGIN
    PASS14_FRONTEND_TRAVEL_HANDOFF_READY
    PASS14_FOLIAGE_BUDGET_READY
    PASS14_PERF_SAMPLE
) do (
    findstr /C:"%%M" "%LOG%" >nul
    if errorlevel 1 (
        echo [STOP] Missing Pass 14/29 runtime evidence: %%M
        echo Log: %LOG%
        exit /b 32
    )
)

findstr /C:"PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED" "%LOG%" >nul
if not errorlevel 1 (
    echo [STOP] An obsolete frontend page transition was attempted during runtime.
    findstr /C:"PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED" "%LOG%"
    exit /b 35
)

findstr /C:"PASS14_PERF_BELOW_TARGET" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Gameplay remained below the current 30 FPS acceptance target.
    findstr /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_BELOW_TARGET" "%LOG%"
    echo [STOP] Playflow evidence passed, but optimization is not finished.
    exit /b 33
)

findstr /C:"PASS14_PERF_30FPS_READY" "%LOG%" >nul
if errorlevel 1 (
    echo [STOP] FPS sample exists but no 30 FPS readiness marker was recorded.
    exit /b 34
)

echo.
echo [PASS] Pass 29 static START reached hosted travel without a live frontend page transition.
echo [PASS] Post-travel frontend handoff evidence found.
echo [PASS] Reduced foliage runtime budget evidence found.
findstr /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 14/29 automated runtime gates completed.
exit /b 0
