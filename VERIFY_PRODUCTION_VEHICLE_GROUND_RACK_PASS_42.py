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

print("PRODUCTION VEHICLE + GROUNDED RACK PASS 42 SOURCE CONTRACT PASS")
print("- normal START attempts exact local HMMWV/M2/BTR intake when those source files are available")
print("- runtime classes point at canonical HMMWV, M2 Browning and BTR-4 production assets")
print("- production mesh authored materials are restored after legacy VehicleBase recolouring and the guard stops")
print("- Museum BASE remains ~27.8 m from MuseumAnchor and all 11 rack items use a 12 cm ground clearance")
print("- unconfigured vehicle audio components no longer tick every frame")
print("STATUS: CODED_UNTESTED; local UE 5.8 model/material/ground-contact/FPS/audio acceptance remains authoritative")