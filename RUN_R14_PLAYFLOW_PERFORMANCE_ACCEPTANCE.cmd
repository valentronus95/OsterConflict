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
echo Pass 45 gates: authored Block0 ground + full-map grass, live pawn біля Museum, compact central Oster,
echo zero implicit filler bots, single visible Museum owner, no runtime material/layer repair,
echo proportional production vehicles, vehicle enter/exit transform preservation, M2 normal pitch,
echo grounded rack та >=30 FPS.
echo.
echo Послідовність:
echo   1. У головному меню натисніть START.
echo   2. На екрані СТВОРЕННЯ СЕРВЕРА задайте параметри та натисніть СТВОРИТИ СЕРВЕР.
echo   3. Після travel оберіть TEAM / SQUAD / ROLE.
echo   4. SPAWN виберіть BASE та натисніть У БІЙ.
echo   5. Реальний pawn має опинитися біля Museum, не на далекому legacy edge spawn.
echo   6. Museum має бути одним R13.7 visible exterior; R13.8 не повинен малювати другий shell.
echo   7. Tactical map має показувати компактний центральний Остер за reference 2026-08-24.
echo   8. Normal local run не повинен сам запускати filler bots без явних Bots/Population/BotFill options.
echo   9. 11 weapon pickups біля BASE мають бути grounded; white/default/BasicShape authored material = FAIL.
echo      Exact production payload gap дозволений лише з explicit real-mesh fallback; fallback не є production READY.
echo  10. HMMWV/M2/BTR: authored materials, правильні пропорції, жодного runtime material repair.
echo  11. Зайдіть водієм у машину далеко від Museum, проїдьте, вийдіть. Повторіть для HMMWV/BTR.
echo  12. Зайдіть gunner у M2, Invert Y OFF: mouse up має піднімати ствол; потім вийдіть з gunner seat.
echo  13. Перевірте WASD + mouse та M map.
echo  14. Залишайтесь у gameplay не менше 20 секунд для FPS sample і bounded-lifecycle evidence.
echo  15. Якщо FPS стрімко падає або ноутбук різко нагрівається - закрийте гру; acceptance має лишитися FAIL.
echo  16. Вийдіть з гри нормально. Це вікно перевірить runtime log.
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
    PASS45_SECONDARY_MENU_HOST_SETUP_QUEUED
    PASS14_MAIN_START_OPENS_SERVER_SETUP
    PASS14_HOST_TRAVEL_BEGIN
    PASS45_SECONDARY_MENU_HOST_TRAVEL_EXECUTE
    PASS14_FRONTEND_TRAVEL_HANDOFF_READY
    PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY
    PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY
    PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY
    PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY
    PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY
    PASS44_COMPACT_PLAYABLE_AREA_READY
    PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY
    PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY
    PASS45_BLOCK0_PRETICK_GROUND_READY
    PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY
    PASS45_REGIONAL_TREE_INTAKE_WIRED
    GAME_RECOVERY_WORLD_READY
    PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED
    PASS45_MUSEUM_R138_COLLISION_ONLY_READY
    PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY
    PASS45_MUSEUM_LAYER_VALIDATION_READY
    PASS14_FOLIAGE_BUDGET_READY
    PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY
    PASS37_MUSEUM_VISIBLE_BASES_READY
    PASS42_BASE_RACK_GROUNDED_READY
    PASS45_REQUIRED_AVAILABLE_WEAPONS_READY
    PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY
    PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY
    PASS45_HMMWV_PROPORTIONAL_VISUAL_READY
    PASS45_BTR4_PROPORTIONAL_VISUAL_READY
    PASS45_M2_MOUNT_ALIGNMENT_READY
    PASS45_M2_GUNNER_PITCH_CONTRACT_READY
    PASS45_VEHICLE_ENTER_TRANSFORM_READY
    PASS45_VEHICLE_EXIT_TRANSFORM_READY
    PASS45_GUNNER_EXIT_TRANSFORM_READY
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

findstr /C:"PASS45_BLOCK0_PRETICK_GROUND_FAIL" /C:"PASS45_BLOCK0_PRETICK_GROUND_CONTENT_GAP" "%LOG%" >nul
if not errorlevel 1 (
    echo [BLOCK0] Authored Ground failed or required ground content did not load.
    findstr /C:"PASS45_BLOCK0_PRETICK_GROUND_FAIL" /C:"PASS45_BLOCK0_PRETICK_GROUND_CONTENT_GAP" "%LOG%"
    exit /b 55
)

findstr /C:"PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL" /C:"PASS45_REGIONAL_TREE_INTAKE_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [BLOCK0] Full-map grass distribution or imported regional-tree intake failed.
    findstr /C:"PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL" /C:"PASS45_REGIONAL_TREE_INTAKE_FAIL" "%LOG%"
    exit /b 56
)

findstr /C:"PASS45_INITIAL_BASE_DEPLOYMENT_" "%LOG%" >nul
if errorlevel 1 (
    echo [SPAWN] Initial-only Pass45 BASE validation/recovery marker is missing.
    echo [SPAWN] Vehicle possession must never be used as replacement deployment proof.
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

findstr /C:"PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL" /C:"PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Single-visible-owner / collision-only contract failed.
    findstr /C:"PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL" /C:"PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL" "%LOG%"
    exit /b 38
)

findstr /C:"PASS45_MUSEUM_LAYER_VALIDATION_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [MUSEUM] Validation-only Museum ownership check found source overlap, a hidden visible owner, or invalid R13.8 collision state.
    echo [MUSEUM] No late repair is allowed; primary authoring must be corrected.
    findstr /C:"PASS45_MUSEUM_LAYER_VALIDATION_FAIL" "%LOG%"
    exit /b 53
)

findstr /C:"PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [LANDMARKS] Primary world authoring still places foreign/generic geometry inside a canonical landmark parcel.
    findstr /C:"PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL" "%LOG%"
    exit /b 39
)

findstr /C:"PASS42_BASE_RACK_GROUNDING_INCOMPLETE" "%LOG%" >nul
if not errorlevel 1 (
    echo [RACK] One or more Museum BASE weapons are still not grounded.
    findstr /C:"PASS42_BASE_RACK_GROUNDING_INCOMPLETE" "%LOG%"
    exit /b 47
)

findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [WEAPONS] One or more required classes have neither exact production nor explicit real fallback visual.
    findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL" "%LOG%"
    exit /b 54
)

findstr /C:"PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP" "%LOG%" >nul
if not errorlevel 1 (
    echo [WEAPONS] One or more rack meshes still have missing/default/BasicShape authored material slots.
    echo [WEAPONS] Runtime recolouring is intentionally absent; real content must be fixed instead.
    findstr /C:"PASS44_WEAPON_AUTHORED_MATERIAL_GAP" /C:"PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP" "%LOG%"
    exit /b 51
)

findstr /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL" /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP" /C:"PASS45_PRODUCTION_VEHICLE_CONTENT_GAP" "%LOG%" >nul
if not errorlevel 1 (
    echo [VEHICLES] Production vehicle authored-material/content validation failed.
    findstr /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL" /C:"PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP" /C:"PASS45_PRODUCTION_VEHICLE_CONTENT_GAP" "%LOG%"
    exit /b 48
)

findstr /C:"PASS45_VEHICLE_ENTER_TRANSFORM_FAIL" /C:"PASS45_VEHICLE_EXIT_TRANSFORM_FAIL" /C:"PASS45_GUNNER_EXIT_TRANSFORM_FAIL" "%LOG%" >nul
if not errorlevel 1 (
    echo [VEHICLES] Vehicle/gunner transform preservation failed. Museum respawn fallback must not participate in ordinary possession.
    findstr /C:"PASS45_VEHICLE_ENTER_TRANSFORM_FAIL" /C:"PASS45_VEHICLE_EXIT_TRANSFORM_FAIL" /C:"PASS45_GUNNER_EXIT_TRANSFORM_FAIL" "%LOG%"
    exit /b 52
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
echo [PASS] Block0 authored Ground READY was recorded with no Ground FAIL/CONTENT GAP.
echo [PASS] Block0 spatial grass coverage and imported regional-tree intake were proved READY.
echo [PASS] Frontend START -> server creation -> hosted travel path is stable.
echo [PASS] Landmark coordinator finished all staged pre-spawn world preparation.
echo [PASS] Normal local run did not silently auto-fill filler bots.
echo [PASS] Primary world/gameplay/BASE/vehicle seeds remained inside compact central Oster.
echo [PASS] Actual live pawn is within the Museum BASE acceptance radius.
echo [PASS] R13.7 is the one visible Museum exterior; R13.8 stayed collision/interactivity-only.
echo [PASS] Museum layer validation passed without late visibility/collision/instance repair.
echo [PASS] Landmark separation found no foreign late-repair requirement.
echo [PASS] All 11 Museum BASE pickups are grounded.
echo [PASS] Required available weapon visuals passed authored material audit; exact payload gaps remain explicit CONTENT GAP.
echo [PASS] VehicleBase did not repaint production assets; read-only HMMWV/M2/BTR material validation passed.
echo [PASS] HMMWV/BTR proportional transforms and M2 mount passed.
echo [PASS] Driver enter/exit and gunner exit preserved the current vehicle location without Museum respawn fallback.
echo [PASS] M2 Invert Y OFF contract produced mouse-up raises aim evidence.
echo [PASS] Tactical Map marker and character input passed.
echo [PASS] Startup scanners/ticks are bounded or physically retired.
echo [PASS] LowCPU foliage stayed bounded and gameplay reached the current 30 FPS target.
echo.
findstr /C:"PASS45_SECONDARY_MENU_HOST_SETUP_QUEUED" /C:"PASS14_MAIN_START_OPENS_SERVER_SETUP" /C:"PASS45_SECONDARY_MENU_HOST_TRAVEL_EXECUTE" "%LOG%"
findstr /C:"PASS45_BLOCK0_PRETICK_GROUND_READY" /C:"PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY" /C:"PASS45_REGIONAL_TREE_INTAKE_WIRED" "%LOG%"
findstr /C:"PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY" /C:"PASS45_MUSEUM_R138_COLLISION_ONLY_READY" /C:"PASS45_MUSEUM_LAYER_VALIDATION_READY" /C:"GAME_RECOVERY_WORLD_READY" "%LOG%"
findstr /C:"PASS45_REQUIRED_AVAILABLE_WEAPONS_READY" /C:"PASS36_WEAPON_MATERIAL_AUDIT_READY" /C:"PASS38_WEAPON_FALLBACK_SCAN_STOPPED" "%LOG%"
findstr /C:"PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY" /C:"PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY" "%LOG%"
findstr /C:"PASS45_HMMWV_PROPORTIONAL_VISUAL_READY" /C:"PASS45_BTR4_PROPORTIONAL_VISUAL_READY" /C:"PASS45_M2_MOUNT_ALIGNMENT_READY" "%LOG%"
findstr /C:"PASS45_VEHICLE_ENTER_TRANSFORM_READY" /C:"PASS45_VEHICLE_EXIT_TRANSFORM_READY" /C:"PASS45_GUNNER_EXIT_TRANSFORM_READY" /C:"PASS45_M2_GUNNER_PITCH_CONTRACT_READY" "%LOG%"
findstr /C:"PASS44_COMPACT_PLAYABLE_AREA_READY" /C:"PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY" /C:"PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY" "%LOG%"
findstr /C:"PASS42_BASE_RACK_GROUNDED_READY" "%LOG%"
findstr /C:"PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND" /C:"PASS39_MINIMAP_UPDATE_BUDGET_READY" "%LOG%"
findstr /C:"PASS31_GAMEPLAY_INPUT_READY" /C:"PASS41_INPUT_RECOVERY_POLL_BUDGET_READY" /C:"PASS39_FP_LOCAL_PAWN_FAST_PATH_READY" "%LOG%"
findstr /C:"PASS40_UI_STABILIZER_BUDGET_READY" /C:"PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY" "%LOG%"
findstr /C:"PASS36_LOWCPU_FOLIAGE_SCOPE_READY" /C:"PASS36_LOWCPU_FOLIAGE_RUNTIME_READY" /C:"PASS14_PERF_30FPS_READY" "%LOG%"
echo [PASS] Pass 45 automated runtime evidence gates completed. Visual fidelity still requires factual screenshot review.
exit /b 0