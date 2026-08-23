@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

echo ============================================================
echo OSTER CONFLICT - PASS 29-36 RUNTIME ACCEPTANCE
echo ============================================================
echo.
echo Перевіряється саме проблема останнього playtest: музей, spawn, map marker, weapon materials, input та FPS.
echo.
echo Послідовність:
echo   1. У головному меню натисніть START.
echo   2. Після travel оберіть TEAM / SQUAD / ROLE.
echo   3. SPAWN обов'язково виберіть BASE біля музею та натисніть У БІЙ.
echo   4. Після spawn перевірте WASD + mouse.
echo   5. Натисніть M один раз, переконайтесь що зелений маркер гравця видно над точками карти, потім закрийте M.
echo   6. Подивіться на 11 weapon pickups біля BASE: вони не повинні бути білими/DefaultMaterial.
echo   7. Залишайтесь у gameplay не менше 16 секунд для FPS sample.
echo   8. Вийдіть з гри нормально. Це вікно автоматично перевірить runtime log.
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
    PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED
    PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY
    PASS32_MUSEUM_LAYER_BUDGET_READY
    PASS35_MUSEUM_CORE_READY
    PASS35_MUSEUM_BASE_DISTANCE_READY
    PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND
    PASS31_GAMEPLAY_INPUT_READY
    PASS30_BASE_DEPLOYMENT_OUTSIDE_MUSEUM
    PASS36_LOWCPU_FOLIAGE_SCOPE_READY
    PASS36_LOWCPU_FOLIAGE_RUNTIME_READY
    PASS36_WEAPON_MATERIAL_AUDIT_READY
    PASS14_PERF_SAMPLE
) do (
    findstr /C:"%%M" "%LOG%" >nul
    if errorlevel 1 (
        echo [STOP] Missing required runtime evidence: %%M
        echo Log: %LOG%
        exit /b 32
    )
)

findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%" | findstr /C:"moveIgnored=0 lookIgnored=0" >nul
if errorlevel 1 (
    echo [INPUT] Character was possessed but move/look input was not fully released.
    findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%"
    exit /b 36
)

findstr /C:"PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED" "%LOG%" >nul
if not errorlevel 1 (
    echo [STOP] An obsolete frontend page transition was attempted during runtime.
    findstr /C:"PASS29_UNSAFE_FRONTEND_PAGE_TRANSITION_BLOCKED" "%LOG%"
    exit /b 35
)

findstr /C:"PASS30_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [SPAWN] BASE deployment recovery failed.
    findstr /C:"PASS30_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%"
    exit /b 37
)

findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Overlapping/obsolete museum layer survived the repair pass.
    findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%"
    exit /b 38
)

findstr /C:"PASS35_MUSEUM_CORE_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Museum authoritative core recovery failed.
    findstr /C:"PASS35_MUSEUM_CORE_FAIL" "%LOG%"
    exit /b 40
)

findstr /C:"PASS35_MUSEUM_BASE_DISTANCE_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [SPAWN] BASE is not inside the current 30-60 m near-museum acceptance band.
    findstr /C:"PASS35_MUSEUM_BASE_DISTANCE_FAIL" "%LOG%"
    exit /b 41
)

findstr /C:"PASS10_FOLIAGE_RUNTIME_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] LowCPU foliage runtime contract failed.
    findstr /C:"PASS10_FOLIAGE_RUNTIME_FAIL" "%LOG%"
    exit /b 42
)

findstr /C:"PASS14_PERF_BELOW_TARGET" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Gameplay remained below the current 30 FPS acceptance target.
    findstr /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_BELOW_TARGET" "%LOG%"
    echo [STOP] Museum/input/material gates may pass, but optimization is not finished.
    exit /b 33
)

findstr /C:"PASS14_PERF_30FPS_READY" "%LOG%" >nul
if errorlevel 1 (
    echo [STOP] FPS sample exists but no 30 FPS readiness marker was recorded.
    exit /b 34
)

echo.
echo [PASS] Frontend START/travel path is stable.
echo [PASS] BASE spawn is outside the museum collision exclusion and still 30-60 m from MuseumAnchor.
echo [PASS] Museum authoritative core exists in runtime.
echo [PASS] Tactical Map player marker was raised above objective labels.
echo [PASS] Character input is GameOnly with move/look ignore stacks released.
echo [PASS] Weapon rack material slots were audited; authored materials were preserved and missing defaults recovered.
echo [PASS] LowCPU foliage stayed inside the bounded museum-area population window.
echo [PASS] Gameplay reached the current 30 FPS acceptance target.
echo.
findstr /C:"PASS35_MUSEUM_CORE_READY" /C:"PASS35_MUSEUM_BASE_DISTANCE_READY" "%LOG%"
findstr /C:"PASS30_BASE_DEPLOYMENT_OUTSIDE_MUSEUM" "%LOG%"
findstr /C:"PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND" "%LOG%"
findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%"
findstr /C:"PASS36_WEAPON_MATERIAL_AUDIT_READY" /C:"PASS36_WEAPON_MATERIAL_RECOVERED" "%LOG%"
findstr /C:"PASS36_LOWCPU_FOLIAGE_SCOPE_READY" /C:"PASS36_LOWCPU_FOLIAGE_RUNTIME_READY" "%LOG%"
findstr /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 29-36 automated runtime acceptance completed.
exit /b 0
