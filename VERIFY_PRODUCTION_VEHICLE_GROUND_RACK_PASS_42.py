#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS42 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS42 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS42 VERIFY FAIL: {label}: forbidden {needle!r}")


def absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS42 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


start = read(ROOT / "START_HERE.cmd")
try_import = read(ROOT / "OsterConflict" / "TRY_PRODUCTION_VEHICLES_UE58.cmd")
importer = read(ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
pickup = read(SRC / "Private" / "OCPickupGunTruck.cpp")
btr = read(SRC / "Private" / "OCBTR.cpp")
vehicle_base = read(SRC / "Private" / "OCVehicleBase.cpp")
rack = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
guard_h = read(SRC / "Public" / "OCProductionVehicleVisualGuardSubsystem.h")
guard = read(SRC / "Private" / "OCProductionVehicleVisualGuardSubsystem.cpp")
audio_h = read(SRC / "Public" / "OCVehicleAudioComponent.h")
audio = read(SRC / "Private" / "OCVehicleAudioComponent.cpp")
settings_h = read(SRC / "Public" / "OCPlayerUserSettings.h")
settings = read(SRC / "Private" / "OCPlayerUserSettings.cpp")
dense = read(SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp")
foliage_guard_h = read(SRC / "Public" / "OCFoliageRuntimeGuardSubsystem.h")
foliage_guard = read(SRC / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")

# Old Pass37 Museum visibility/rebuild owner is forbidden after Pass45 retirement.
absent(SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h", "Museum visibility/rebuild header")
absent(SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp", "Museum visibility/rebuild source")

# Normal game no longer ignores a locally available exact production package.
require(start, 'TRY_PRODUCTION_VEHICLES_UE58.cmd', "normal launcher production intake")
for needle in (
    'SM_HMMWV_UA.uasset', 'SM_M2_Browning.uasset', 'SM_BTR4_Bucephalus.uasset',
    'IMPORT_PRODUCTION_VEHICLES_UE58.cmd',
):
    require(try_import, needle, "optional production intake helper")
for needle in (
    '/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA',
    '/Game/Production/Weapons/M2/SM_M2_Browning',
    '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus',
):
    require(importer, needle, "canonical production importer")

# Runtime classes request imported models and preserve native proportions. Exact M2 uses its authored
# receiver/mount pivot; the rejected bounds-grounding/longest-axis correction must not return.
for needle in (
    '/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA',
    '/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning',
    'PASS45_HMMWV_PROPORTIONAL_VISUAL_READY',
    'PASS45_M2_AUTHORED_PIVOT_READY',
    'm2_authored_pivot=1',
    'bounds_recenter=0',
    'longest_axis_guess=0',
    'nonuniform_stretch=0',
):
    require(pickup, needle, "HMMWV/M2 runtime visual")
forbid(pickup, 'AddGroundedTurretVisual(this, M2Parent, M2, 165.0f',
       "rejected exact-M2 bounds grounding")
for needle in (
    '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus',
    'PASS45_BTR4_PROPORTIONAL_VISUAL_READY',
    'nonuniform_stretch=0',
):
    require(btr, needle, "BTR-4 runtime visual")

# Pass45 fixes the material overwrite at its source. VehicleBase may tint blockout pieces, but every
# /Game/Production mesh must bypass BasicShape MID creation after ApplyVehicleStyle().
for needle in (
    'AssetPath.StartsWith(TEXT("/Game/Production/"))',
    'continue;',
    'PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY',
    'production_override=0',
    'legacy_tint_blockout_only=1',
):
    require(vehicle_base, needle, "VehicleBase production material bypass")

# The old Pass42 guard used to repair VehicleBase's own damage by polling and EmptyOverrideMaterials().
# That behavior is now forbidden. The guard is one-shot/read-only and only reports source/content failure.
for needle in (
    'Pass45 read-only production vehicle visual validator',
    'ValidationDelaySeconds = 1.00f',
    'PASS45_PRODUCTION_VEHICLE_VALIDATION_SCHEDULED',
    'PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL',
    'PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP',
    'PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY',
    'PASS45_PRODUCTION_VEHICLE_CONTENT_GAP',
    'validation_only=1',
    'mutation=0',
    'polling=0',
):
    require(guard_h + guard, needle, "Pass45 production vehicle validation-only guard")
for forbidden in (
    'EmptyOverrideMaterials(',
    'PASS42_PRODUCTION_MATERIALS_RESTORED',
    'PASS42_PRODUCTION_VEHICLE_VISUALS_READY',
    'MaxAuditPasses',
    'AuditIntervalSeconds',
    'SetMaterial(',
):
    forbid(guard, forbidden, "obsolete production material repair must not survive")

# Museum primary BASE/rack are grounded instead of the old floating pickup path.
for needle in (
    'FVector(1400.0f, -2400.0f, 120.0f)',
    'FVector(-1400.0f, -2400.0f, 120.0f)',
    'RequiredRackWeaponCount = 11',
    'RackGroundClearanceCm = 12.0f',
    'SnapLocationToWalkableSurface(World, Desired, RackGroundClearanceCm)',
    'PASS42_BASE_RACK_GROUNDED_READY',
    'PASS42_BASE_RACK_GROUNDING_INCOMPLETE',
):
    require(rack, needle, "Museum BASE grounded weapon rack")
if 'SnapLocationToWalkableSurface(World, Desired, 72.0f)' in rack:
    raise SystemExit("PASS42 VERIFY FAIL: old +72 cm floating rack spawn survived")

# Unconfigured audio components must not burn a render-frame tick forever.
for needle in (
    'void SetAudioProfile(UOCVehicleAudioProfile* InProfile);',
    'SetComponentTickEnabled(AudioProfile!=nullptr)',
    'PASS42_VEHICLE_AUDIO_IDLE_BUDGET_READY',
):
    require(audio_h + audio, needle, "vehicle audio lifecycle budget")

# Visual clarity recovery remains native 100% scale and texture quality 3 without expensive-lighting mutation.
for needle in (
    'bPass42GraphicsClarityRecoveryApplied = false',
    'SetTextureQuality(SafeQuality(GameSettings->GetTextureQuality(), 3))',
    'SetResolutionScaleValueEx(100.0f)',
    'const bool bLooksLikeAutomaticPass39',
    'GameSettings->SetTextureQuality(3);',
    'PASS42_GRAPHICS_CLARITY_RECOVERY_APPLIED',
    'PASS42_GRAPHICS_CUSTOM_PROFILE_PRESERVED',
    'quality_mutation_on_low_fps=0',
    'expensive_lighting_unchanged=1',
):
    require(settings_h + settings, needle, "native-scale graphics clarity recovery")

# LowCPU foliage remains bounded and its guard is throttled. PASS45 item 31 upgrades old hide-only ground
# proxy retirement to physical destruction, and removes developer reference markers/text labels as scenery.
for needle in (
    'LowCPUHalfExtentCm = 10000.0f',
    'LowCPUGridStepCm = 1500.0f',
    'LowCPUGrassCullEndCm = 8500',
    'PASS42_LOWCPU_FOLIAGE_SCOPE_EXPANDED',
    'area_m=200x200',
    'full_sector_population=0',
):
    require(dense, needle, "bounded expanded LowCPU foliage")
for needle in (
    'float ValidationAccumulator = 0.0f',
    'ValidationAccumulator < 0.25f',
    'bGroundProxyDestructionObserved || DestroySourceGroundCoverProxies()',
    'bDeveloperMarkerDestructionObserved || DestroyDeveloperVisualMarkers()',
    'PASS45_GROUND_COVER_PRIMITIVES_DESTROYED',
    'PASS45_DEVELOPER_WORLD_MARKERS_DESTROYED',
    'PASS42_FOLIAGE_GUARD_THROTTLED_READY',
    'sample_hz=4',
    'proxy_rescan_after_ready=0',
):
    require(foliage_guard_h + foliage_guard, needle, "throttled foliage acceptance guard")
for forbidden in (
    'bProxyRetirementObserved || RetireSourceGroundCoverProxies()',
    'PASS10_GROUND_COVER_PROXY_RETIRED',
):
    forbid(foliage_guard_h + foliage_guard, forbidden, "obsolete hide-only foliage retirement")

# Pass45 supersedes the old timer/rebuild choreography. Coordinator cancels historical stage timers and
# runs current Museum/Silpo/Culture stages once; the destructive visibility owner stays deleted.
for needle in (
    'Timers.ClearAllTimersForObject(Stage)',
    'Stage->RunAuthoritativeBuildNow(World)',
    'Stage->RunAuthoritativeUpgradeNow(World)',
    'PASS45_LANDMARK_STARTUP_COORDINATED_READY',
    'legacy_core_recovery=0',
    'destructive_visibility_rebuild=0',
):
    require(startup, needle, "Pass45 coordinated landmark startup")

print("PRODUCTION VEHICLE + GROUNDED RACK + VISUAL/FPS RECOVERY PASS 42/45 SOURCE CONTRACT PASS")
print("- exact local HMMWV/M2/BTR intake remains wired and production meshes preserve native proportions")
print("- exact M2 uses its authored receiver/mount pivot; rejected bounds/longest-axis recenter is forbidden")
print("- VehicleBase skips legacy tint for /Game/Production meshes at the primary source")
print("- production vehicle guard is one-shot validation-only: no material repair, no polling")
print("- Museum BASE rack remains grounded with 12 cm clearance")
print("- native-scale graphics clarity and bounded LowCPU foliage/audio budgets remain intact")
print("- PASS45 item31 physically destroys source ground-cover proxies and developer visual markers")
print("- historical Museum visibility/rebuild owner is deleted; current landmark startup is coordinated once")
print("STATUS: CODED_UNTESTED; local UE 5.8 runtime remains authoritative")
