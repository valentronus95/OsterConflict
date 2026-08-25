@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "LOG=%~dp0Logs\R14_CURRENT_GAMEPLAY.log"

echo ============================================================
echo OSTER CONFLICT - PASS 45 CURRENT RUNTIME ACCEPTANCE
echo ============================================================
echo.
echo Перевіряється фактичний normal-game runtime, а не старі source-only припущення.
echo Pass 45 gates: live pawn біля Museum, compact central Oster, zero implicit filler bots,
echo no BasicShape/grey material disguise, one coordinated landmark startup, proportional production vehicles,
echo grounded rack, bounded startup work та >=30 FPS runtime evidence.
echo.
echo Послідовність:
echo   1. У головному меню натисніть START.
echo   2. Після travel оберіть TEAM / SQUAD / ROLE.
echo   3. SPAWN виберіть BASE та натисніть У БІЙ.
echo   4. Реальний pawn має опинитися біля Museum, не на далекому legacy edge spawn.
echo   5. Tactical map має показувати компактний центральний Остер за reference 2026-08-24.
echo   6. Normal local run не повинен сам запускати filler bots без явних Bots/Population/BotFill options.
echo   7. 11 weapon pickups біля BASE мають бути grounded; white/default/BasicShape authored material = FAIL.
echo   8. HMMWV/M2/BTR production content: missing source/material або distorted transform = FAIL.
echo   9. Перевірте WASD + mouse та M map.
echo  10. Обов'язково перевірте vehicle enter/drive/exit далеко від Museum: жодного teleport назад до BASE.
echo  11. Перевірте M2: neutral mount aligned, mouse up raises gun при Invert Y OFF.
echo  12. Залишайтесь у gameplay не менше 20 секунд для FPS sample і bounded-lifecycle evidence.
echo  13. Якщо FPS стрімко падає або ноутбук різко нагрівається - закрийте гру; acceptance має лишитися FAIL.
echo  14. Вийдіть з гри нормально. Це вікно перевірить runtime log.
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
    PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY
    PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY
    PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY
    PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY
    PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY
    PASS44_COMPACT_PLAYABLE_AREA_READY
    PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY
    PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY
    PASS45_LANDMARK_STARTUP_COORDINATED_READY
    PASS14_FOLIAGE_BUDGET_READY
    PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED
    PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY
    PASS32_MUSEUM_LAYER_BUDGET_READY
    PASS37_MUSEUM_VISIBLE_BASES_READY
    PASS42_BASE_RACK_GROUNDED_READY
    PASS42_PRODUCTION_VEHICLE_VISUALS_READY
    PASS45_HMMWV_PROPORTIONAL_VISUAL_READY
    PASS45_BTR4_PROPORTIONAL_VISUAL_READY
    PASS45_M2_MOUNT_ALIGNMENT_READY
    PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND
    PASS31_GAMEPLAY_INPUT_READY
    PASS41_INPUT_RECOVERY_POLL_BUDGET_READY
    PASS36_LOWCPU_FOLIAGE_SCOPE_READY
    PASS36_LOWCPU_FOLIAGE_RUNTIME_READY
    PASS36_WEAPON_MATERIAL_AUDIT_READY
    PASS38_WEAPON_FALLBACK_SCAN_STOPPED
    PASS39_GRAPHICS_QUALITY_PROFILE_READY
    PASS39_MINIMAP_UPDATE_BUDGET_READY
    PASS39_FP_LOCAL_PAWN_FAST_PATH_READY
    PASS39_PERF_SAMPLER_IDLE_READY
    PASS40_UI_STABILIZER_BUDGET_READY
    PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY
    PASS14_PERF_SAMPLE
) do (
    findstr /C:"%%M" "%LOG%" >nul
    if errorlevel 1 (
        echo [STOP] Missing required runtime evidence: %%M
        echo Log: %LOG%
        exit /b 32
    )
)

findstr /C:"PASS45_INITIAL_BASE_DEPLOYMENT_" "%LOG%" >nul
if errorlevel 1 (
    echo [SPAWN] Initial-only Pass45 BASE validation/recovery marker is missing.
    echo [SPAWN] Vehicle possession must never be used as a replacement deployment proof.
    exit /b 35
)

findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%" | findstr /C:"moveIgnored=0 lookIgnored=0" >nul
if errorlevel 1 (
    echo [INPUT] Character was possessed but move/look input was not fully released.
    findstr /C:"PASS31_GAMEPLAY_INPUT_READY" "%LOG%"
    exit /b 36
)

findstr /C:"PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [SPAWN] Actual live pawn did not remain within the Museum BASE acceptance radius.
    findstr /C:"PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL" /C:"PASS44_ACTUAL_PAWN_MUSEUM_BASE_CORRECTED" "%LOG%"
    exit /b 49
)

findstr /C:"PASS44_COMPACT_PLAYABLE_AREA_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MAP] Compact central Oster playable-area trim failed.
    findstr /C:"PASS44_COMPACT_PLAYABLE_AREA_FAIL" "%LOG%"
    exit /b 50
)

findstr /C:"PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [SPAWN] Initial BASE recovery reported failure.
    findstr /C:"PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL" "%LOG%"
    exit /b 37
)

findstr /C:"PASS42_BASE_RACK_GROUNDING_INCOMPLETE" "%LOG%" >nul
if not errorlevel 1 (
    echo [RACK] One or more Museum BASE weapons are still not grounded.
    findstr /C:"PASS42_BASE_RACK_GROUNDING_INCOMPLETE" "%LOG%"
    exit /b 47
)

findstr /C:"PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP" "%LOG%" >nul
if not errorlevel 1 (
    echo [WEAPONS] One or more rack meshes still have missing/default/BasicShape authored material slots.
    echo [WEAPONS] Runtime recolouring is intentionally absent; real content must be fixed instead.
    findstr /C:"PASS44_WEAPON_AUTHORED_MATERIAL_GAP" /C:"PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP" "%LOG%"
    exit /b 51
)

findstr /C:"PASS42_PRODUCTION_VEHICLE_CONTENT_GAP" "%LOG%" >nul
if not errorlevel 1 (
    echo [VEHICLES] Exact HMMWV / M2 Browning / BTR-4 production visual did not appear in strict runtime.
    findstr /C:"PASS42_PRODUCTION_VEHICLE_CONTENT_GAP" "%LOG%"
    exit /b 48
)

findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Overlapping/obsolete museum layer survived the current ownership budget.
    findstr /C:"PASS32_MUSEUM_LAYER_BUDGET_FAIL" "%LOG%"
    exit /b 38
)

findstr /C:"PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP" "%LOG%" >nul
if not errorlevel 1 (
    echo [PERF] Weapon fallback/material audit hit its hard startup budget instead of reaching a terminal state.
    findstr /C:"PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP" "%LOG%"
    exit /b 44
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
    echo [STOP] Runtime gates may pass, but optimization is not finished.
    exit /b 33
)

findstr /C:"PASS14_PERF_30FPS_READY" "%LOG%" >nul
if errorlevel 1 (
    echo [STOP] FPS sample exists but no 30 FPS readiness marker was recorded.
    exit /b 34
)

echo.
echo [PASS] Frontend START/travel path is stable.
echo [PASS] Normal local run did not silently auto-fill filler bots.
echo [PASS] Primary world/gameplay/BASE/vehicle seeds remained inside compact central Oster.
echo [PASS] Actual live pawn, not merely a spawn actor, is within the Museum BASE acceptance radius.
echo [PASS] Museum/Silpo/Culture startup ran through the current coordinated owner window; deleted recovery owners were not required.
echo [PASS] All 11 Museum BASE pickups are grounded.
echo [PASS] Weapon authored materials passed without BasicShape/grey runtime disguise.
echo [PASS] HMMWV + M2 Browning + BTR-4 production visual gates loaded with proportional source markers.
echo [PASS] Tactical Map marker and character input passed.
echo [PASS] Startup scanners/ticks are bounded or physically retired.
echo [PASS] LowCPU foliage stayed bounded and gameplay reached the current 30 FPS target.
echo [MANUAL] Vehicle enter/drive/exit Museum-teleport test and M2 pitch-direction check remain visual/input acceptance items.
echo.
findstr /C:"PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY" /C:"PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY" /C:"PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY" /C:"PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY" /C:"PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY" "%LOG%"
findstr /C:"PASS44_COMPACT_PLAYABLE_AREA_READY" /C:"PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY" /C:"PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY" "%LOG%"
findstr /C:"PASS45_LANDMARK_STARTUP_COORDINATED_READY" /C:"PASS45_INITIAL_BASE_DEPLOYMENT_" "%LOG%"
findstr /C:"PASS42_BASE_RACK_GROUNDED_READY" /C:"PASS36_WEAPON_MATERIAL_AUDIT_READY" /C:"PASS38_WEAPON_FALLBACK_SCAN_STOPPED" "%LOG%"
findstr /C:"PASS45_HMMWV_PROPORTIONAL_VISUAL_READY" /C:"PASS45_BTR4_PROPORTIONAL_VISUAL_READY" /C:"PASS45_M2_MOUNT_ALIGNMENT_READY" "%LOG%"
findstr /C:"PASS42_PRODUCTION_MATERIALS_RESTORED" /C:"PASS42_PRODUCTION_VEHICLE_VISUALS_READY" "%LOG%"
findstr /C:"PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND" /C:"PASS39_MINIMAP_UPDATE_BUDGET_READY" "%LOG%"
findstr /C:"PASS31_GAMEPLAY_INPUT_READY" /C:"PASS41_INPUT_RECOVERY_POLL_BUDGET_READY" /C:"PASS39_FP_LOCAL_PAWN_FAST_PATH_READY" "%LOG%"
findstr /C:"PASS40_UI_STABILIZER_BUDGET_READY" /C:"PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY" "%LOG%"
findstr /C:"PASS36_LOWCPU_FOLIAGE_SCOPE_READY" /C:"PASS36_LOWCPU_FOLIAGE_RUNTIME_READY" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 45 automated runtime evidence gates completed; manual visual/input gates still apply.
exit /b 0
