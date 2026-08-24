@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

echo ============================================================
echo OSTER CONFLICT - PASS 29-39 RUNTIME ACCEPTANCE
echo ============================================================
echo.
echo Перевіряється останній playtest: музей реально видимий, BASE близько, зброя не біла/сіра, графіка не деградує автоматично, runtime не накопичує rebuild/scan/tick work, map marker, input та FPS.
echo.
echo Послідовність:
echo   1. У головному меню натисніть START.
echo   2. Після travel оберіть TEAM / SQUAD / ROLE.
echo   3. SPAWN обов'язково виберіть BASE біля музею та натисніть У БІЙ.
echo   4. Одразу після spawn музей має бути перед вами приблизно за 20-30 метрів, а не за горизонтом.
echo   5. Перевірте WASD + mouse.
echo   6. Натисніть M один раз, переконайтесь що зелений маркер гравця видно над точками карти, потім закрийте M.
echo   7. Подивіться на 11 weapon pickups біля BASE. Valid imported materials не повинні бути перемальовані flat-color fallback-ом.
echo   8. Графіка не повинна через кілька секунд сама падати до розмитого 65%% emergency-профілю.
echo   9. Залишайтесь у gameplay не менше 20 секунд для FPS sample, late-startup museum check і bounded-lifecycle evidence.
echo  10. Якщо FPS стрімко падає або ноутбук різко нагрівається - закрийте гру; acceptance має залишитись FAIL, а не змушувати залізо терпіти.
echo  11. Вийдіть з гри нормально. Це вікно автоматично перевірить runtime log.
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
    PASS37_MUSEUM_VISIBLE_CORE_READY
    PASS37_MUSEUM_VISIBLE_BASES_READY
    PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH
    PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND
    PASS31_GAMEPLAY_INPUT_READY
    PASS36_LOWCPU_FOLIAGE_SCOPE_READY
    PASS36_LOWCPU_FOLIAGE_RUNTIME_READY
    PASS36_WEAPON_MATERIAL_AUDIT_READY
    PASS37_WEAPON_VISIBLE_PALETTE_READY
    PASS38_MUSEUM_REBUILD_BUDGET_READY
    PASS38_WEAPON_FALLBACK_SCAN_STOPPED
    PASS38_WEAPON_PALETTE_SCAN_STOPPED
    PASS39_GRAPHICS_QUALITY_PROFILE_READY
    PASS39_MINIMAP_UPDATE_BUDGET_READY
    PASS39_FP_LOCAL_PAWN_FAST_PATH_READY
    PASS39_PERF_SAMPLER_IDLE_READY
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

findstr /C:"PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [SPAWN] BASE deployment could not recover to the visible museum approach.
    findstr /C:"PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%"
    exit /b 37
)

findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Overlapping/obsolete museum layer survived the repair pass.
    findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%"
    exit /b 38
)

findstr /C:"PASS37_MUSEUM_VISIBLE_CORE_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Owner tags existed but the real visible structural core was not present near MuseumAnchor.
    findstr /C:"PASS37_MUSEUM_VISIBLE_CORE_FAIL" "%LOG%"
    exit /b 40
)

findstr /C:"PASS38_MUSEUM_REBUILD_BUDGET_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Museum recovery used its one rebuild budget but still failed. No repeated rebuild loop is allowed.
    findstr /C:"PASS38_MUSEUM_REBUILD_BUDGET_FAIL" /C:"PASS38_MUSEUM_SINGLE_REBUILD_EXECUTED" "%LOG%"
    exit /b 43
)

findstr /C:"PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Weapon fallback startup scan hit its hard budget instead of converging.
    findstr /C:"PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP" "%LOG%"
    exit /b 44
)

findstr /C:"PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Weapon palette startup scan hit its hard budget instead of converging.
    findstr /C:"PASS38_WEAPON_PALETTE_SCAN_BOUNDED_STOP" "%LOG%"
    exit /b 45
)

findstr /C:"PASS15_EMERGENCY_PERF_PROFILE_APPLIED" "%LOG%" >nul
if not errorlevel 1 (
    echo [GRAPHICS] Stale binary still applied the old hidden 65%% emergency graphics downgrade.
    findstr /C:"PASS15_EMERGENCY_PERF_PROFILE_APPLIED" "%LOG%"
    exit /b 46
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
    findstr /C:"PASS39_LOW_FPS_PROBE_DIAGNOSTIC" /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_BELOW_TARGET" "%LOG%"
    echo [STOP] Museum/spawn/material gates may pass, but optimization is not finished.
    exit /b 33
)

findstr /C:"PASS14_PERF_30FPS_READY" "%LOG%" >nul
if errorlevel 1 (
    echo [STOP] FPS sample exists but no 30 FPS readiness marker was recorded.
    exit /b 34
)

echo.
echo [PASS] Frontend START/travel path is stable.
echo [PASS] Museum has a real visible structural core near MuseumAnchor, not merely an owner tag.
echo [PASS] Museum recovery stayed inside a one-rebuild destructive budget; no repeated architecture churn survived.
echo [PASS] Preferred BASE is in the 20-45 m museum approach and deployment landed there.
echo [PASS] Tactical Map player marker is above objective labels.
echo [PASS] Character input is GameOnly with move/look ignore stacks released.
echo [PASS] Weapon material/fallback/palette startup scans converged and stopped instead of polling forever.
echo [PASS] Non-placeholder imported weapon materials were preserved from Pass 37 forced recolouring.
echo [PASS] Graphics profile is the Pass 39 balanced/preserved profile; old hidden 65%% emergency mutation did not run.
echo [PASS] Minimap updates are budgeted to 10 Hz and tactical-map scene capture remains one-shot.
echo [PASS] First-person weapon presentation resolves only the local pawn, not every character each frame.
echo [PASS] Performance sampler finished and retired its world tick.
echo [PASS] LowCPU foliage stayed inside the bounded museum-area population window.
echo [PASS] Gameplay reached the current 30 FPS acceptance target.
echo.
findstr /C:"PASS37_MUSEUM_VISIBLE_CORE_READY" /C:"PASS38_MUSEUM_REBUILD_BUDGET_READY" "%LOG%"
findstr /C:"PASS37_MUSEUM_VISIBLE_BASES_READY" /C:"PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH" "%LOG%"
findstr /C:"PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND" /C:"PASS39_MINIMAP_UPDATE_BUDGET_READY" "%LOG%"
findstr /C:"PASS31_GAMEPLAY_INPUT_READY" /C:"PASS39_FP_LOCAL_PAWN_FAST_PATH_READY" "%LOG%"
findstr /C:"PASS36_WEAPON_MATERIAL_AUDIT_READY" /C:"PASS37_WEAPON_VISIBLE_PALETTE_READY" /C:"PASS38_WEAPON_FALLBACK_SCAN_STOPPED" /C:"PASS38_WEAPON_PALETTE_SCAN_STOPPED" "%LOG%"
findstr /C:"PASS39_GRAPHICS_QUALITY_PROFILE_READY" /C:"PASS39_GRAPHICS_QUALITY_RECOVERY_APPLIED" /C:"PASS39_GRAPHICS_CUSTOM_PROFILE_PRESERVED" "%LOG%"
findstr /C:"PASS36_LOWCPU_FOLIAGE_SCOPE_READY" /C:"PASS36_LOWCPU_FOLIAGE_RUNTIME_READY" "%LOG%"
findstr /C:"PASS39_PERF_SAMPLER_IDLE_READY" /C:"PASS14_PERF_SAMPLE" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 29-39 automated runtime acceptance completed.
exit /b 0