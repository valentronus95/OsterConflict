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


start = read(ROOT / "START_HERE.cmd")
try_import = read(ROOT / "OsterConflict" / "TRY_PRODUCTION_VEHICLES_UE58.cmd")
importer = read(ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
pickup = read(SRC / "Private" / "OCPickupGunTruck.cpp")
btr = read(SRC / "Private" / "OCBTR.cpp")
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
museum137 = read(SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
museum138 = read(SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
museum_visibility = read(SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp")

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

# Runtime classes must actually request the imported models rather than source-only blocks.
require(pickup, '/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA.SM_HMMWV_UA', "HMMWV runtime visual")
require(pickup, '/Game/Production/Weapons/M2/SM_M2_Browning.SM_M2_Browning', "M2 runtime visual")
require(btr, '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.SM_BTR4_Bucephalus', "BTR-4 runtime visual")

# VehicleBase historically recoloured every StaticMeshComponent after derived production style was applied.
# Pass 42 must restore authored material slots for production meshes and stop polling after a finite budget.
for needle in (
    'AssetPath.StartsWith(TEXT("/Game/Production/"))',
    'Component->EmptyOverrideMaterials();',
    'MaxAuditPasses = 12',
    'ClearTimer(AuditTimer)',
    'PASS42_PRODUCTION_MATERIALS_RESTORED',
    'PASS42_PRODUCTION_VEHICLE_VISUALS_READY',
    'PASS42_PRODUCTION_VEHICLE_CONTENT_GAP',
):
    require(guard_h + guard, needle, "production authored-material guard")

# Museum primary BASE is still the close 27.8 m approach, and rack items are now grounded instead of +72 cm.
for needle in (
    'FVector(1400.0f, -2400.0f, 120.0f)',
    'FVector(-1400.0f, -2400.0f, 120.0f)',
    'RequiredRackWeaponCount = 11',
    'RackGroundClearanceCm = 12.0f',
    'SnapLocationToWalkableSurface(World, Desired, RackGroundClearanceCm)',
    'PASS42_BASE_RACK_GROUNDED_READY',
    'PASS42_BASE_RACK_GROUNDING_INCOMPLETE',
):
    require(rack, needle, "museum BASE grounded weapon rack")
if 'SnapLocationToWalkableSurface(World, Desired, 72.0f)' in rack:
    raise SystemExit("PASS42 VERIFY FAIL: old +72 cm floating rack spawn survived")

# Unconfigured audio components must not burn a render-frame tick forever; exact vehicle audio remains content-driven.
for needle in (
    'void SetAudioProfile(UOCVehicleAudioProfile* InProfile);',
    'SetComponentTickEnabled(AudioProfile!=nullptr)',
    'PASS42_VEHICLE_AUDIO_IDLE_BUDGET_READY',
):
    require(audio_h + audio, needle, "vehicle audio lifecycle budget")

# Visual clarity recovery: do not hide CPU/runtime issues behind a permanently soft 85% automatic profile.
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

# Keep LowCPU foliage bounded but useful around BASE, and stop using the acceptance guard as a per-frame scanner.
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
    'bProxyRetirementObserved || RetireSourceGroundCoverProxies()',
    'PASS42_FOLIAGE_GUARD_THROTTLED_READY',
    'sample_hz=4',
    'proxy_rescan_after_ready=0',
):
    require(foliage_guard_h + foliage_guard, needle, "throttled foliage acceptance guard")

# The player spawns beside MuseumAnchor, so the normal museum build must happen before the old five-second gap.
for needle in (
    'MuseumPhotoModelDelaySeconds = 0.75f',
    'PASS42_MUSEUM_EXTERIOR_EARLY_SCHEDULED',
):
    require(museum137, needle, "early R13.7 museum exterior schedule")
for needle in (
    'R138MuseumDelaySeconds = 1.10f',
    'PASS42_MUSEUM_ARCHITECTURE_EARLY_SCHEDULED',
):
    require(museum138, needle, "early R13.8 museum architecture schedule")
for needle in (
    'FirstPollDelaySeconds = 1.45f',
    'PollIntervalSeconds = 0.35f',
    'LateStartupSettleSeconds = 2.20f',
    'MaxRebuildAttempts = 1',
    'PASS42_MUSEUM_EARLY_VISIBILITY_READY',
    'normal_architecture_delay=1.10',
    'destructive_loop=0',
):
    require(museum_visibility, needle, "museum visibility guard aligned after early build")

print("PRODUCTION VEHICLE + GROUNDED RACK + VISUAL/FPS RECOVERY PASS 42 SOURCE CONTRACT PASS")
print("- normal START attempts exact local HMMWV/M2/BTR intake when those source files are available")
print("- runtime classes point at canonical HMMWV, M2 Browning and BTR-4 production assets")
print("- production mesh authored materials are restored after legacy VehicleBase recolouring and the guard stops")
print("- Museum BASE remains ~27.8 m from MuseumAnchor and all 11 rack items use a 12 cm ground clearance")
print("- automatic graphics profiles recover native 100% scale + texture quality 3 without raising costly lighting")
print("- LowCPU foliage is bounded to 200 x 200 m around Museum/BASE and its acceptance scan is throttled to 4 Hz")
print("- Museum exterior/architecture now schedule at 0.75/1.10 s with visibility proof starting at 1.45 s")
print("- unconfigured vehicle audio components no longer tick every frame")
print("STATUS: CODED_UNTESTED; local UE 5.8 model/material/ground-contact/FPS/audio acceptance remains authoritative")
