@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

echo ============================================================
echo OSTER CONFLICT - PASS 14 PLAYFLOW + PERFORMANCE ACCEPTANCE
echo ============================================================
echo.
echo Expected normal flow:
echo   1. Main menu: press START.
echo   2. START must open CREATE SERVER settings. It must NOT load gameplay.
echo   3. Set MaxPlayers / Bots / BotDifficulty and press CREATE SERVER.
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
    PASS14_MAIN_START_OPENS_SERVER_SETUP
    PASS14_HOST_TRAVEL_BEGIN
    PASS14_FRONTEND_TRAVEL_HANDOFF_READY
    PASS14_FOLIAGE_BUDGET_READY
    PASS14_PERF_SAMPLE
) do (
    findstr /C:"%%M" "%LOG%" >nul
    if errorlevel 1 (
        echo [STOP] Missing Pass 14 runtime evidence: %%M
        echo Log: %LOG%
        exit /b 32
    )
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
echo [PASS] Explicit server flow evidence found.
echo [PASS] Post-travel frontend handoff evidence found.
echo [PASS] Reduced foliage runtime budget evidence found.
findstr /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 14 automated runtime gates completed.
exit /b 0
