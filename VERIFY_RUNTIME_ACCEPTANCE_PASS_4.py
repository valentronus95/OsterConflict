from pathlib import Path

ROOT = Path(__file__).resolve().parent

FILES = {
    "launcher": ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
    "weapon_preflight": ROOT / "OsterConflict/Scripts/verify_required_weapon_assets.py",
    "source_recovery": ROOT / "OsterConflict/Scripts/prepare_local_production_sources.ps1",
    "fx": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTransientVisualFX.cpp",
    "spawn": ROOT / "OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp",
    "foliage": ROOT / "OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp",
    "r10": ROOT / "VERIFY_R10_CXX_BATCH_FIX.py",
}


def read(name):
    path = FILES[name]
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 4 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text, needle, where):
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 4 FAIL: {where}: missing {needle!r}")


launcher = read("launcher")
weapon_preflight = read("weapon_preflight")
source_recovery = read("source_recovery")
fx = read("fx")
spawn = read("spawn")
foliage = read("foliage")
r10 = read("r10")

for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL weapon visual in a fresh UE process",
    "Primitive weapon boxes are not accepted as a fallback for the normal playtest.",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
):
    require(launcher, needle, "normal gameplay launcher gate")

for needle in (
    "/Game/AK-47/Mesh/SKM_AK-47",
    "/Game/R13/Weapons/Stein/1911/SKM_1911",
    "/Game/R13/Weapons/Stein/MP5/SKM_MP5",
    "/Game/R13/Weapons/Stein/M700/SKM_M700",
    "/Game/R13/Weapons/Stein/M14/SKM_M14",
    "/Game/R13/Weapons/Stein/Mac10/SKM_Mac10",
    "/Game/R13/Weapons/Stein/Tec9/SKM_Tec9",
    "/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction",
    "/Game/R13/Weapons/machinegun",
    "/Game/R13/Weapons/shotgun",
    "/Game/R13/Weapons/rocketlauncherModern",
    "REQUIRED_REAL_WEAPON_ASSETS=PASS",
):
    require(weapon_preflight, needle, "required real weapon preflight")

for needle in (
    "$SourceRoot,",
    "Join-Path $env:USERPROFILE 'Downloads'",
    "Join-Path $env:USERPROFILE 'Desktop'",
    "Join-Path $env:USERPROFILE 'Documents'",
    "Restore-BtrTexturesFromRoots",
    "Get-ChildItem -LiteralPath $root -Recurse -File -Filter '*.zip'",
    "BTR texture ",
    "Bahnya_low_albedo.png",
    "Koleso_low_albedo.png",
    "Korpus_low_albedo.png",
    "Windows_low_albedo.png",
    "interior.png",
    "tire.png",
    "required production model sources and BTR textures are now available locally",
):
    require(source_recovery, needle, "local production source recovery")

for needle in (
    "IsOnLocalAimRay",
    "bOnLocalAimRay",
    "bRebasedToMuzzle",
    "ComponentName.Contains(TEXT(\"barrel\")",
    "ComponentName.Contains(TEXT(\"muzzle\")",
    "FMath::Min(DistanceToEnd, 900.0f)",
    "TryResolveSocketMuzzle",
):
    require(fx, needle, "muzzle/tracer source")

for needle in (
    "AOCWorldSectorOster::MuseumAnchor()",
    "FVector(-1450.0f, -900.0f, 120.0f)",
    "FVector(1450.0f, 900.0f, 120.0f)",
    "SpawnRuntimeBaseWeaponRack",
    "SnapLocationToWalkableSurface",
):
    require(spawn, needle, "museum base spawn")

for needle in (
    "constexpr int32 CellsPerBatch = 96;",
    "PopulateBatch",
    "PopulationBatchTimer",
):
    require(foliage, needle, "batched dense foliage")

# The old R10 verifier already has a file-specific OCGameUIRootWidget slot-shadow check.
# It must not revive its previous whole-project spelling ban, which incorrectly rejected unrelated helpers.
if "'if (UVerticalBoxSlot* Slot'," in r10 or "'if (UCanvasPanelSlot* Slot'," in r10 or "'if (UHorizontalBoxSlot* Slot'," in r10:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: R10 global Slot false-positive tokens returned")
require(r10, "UI Slot shadow names removed", "R10 file-specific UI shadow contract")

print("RUNTIME ACCEPTANCE PASS 4 SOURCE CONTRACT PASS")
print("- normal gameplay hard-gates required real weapon assets in a fresh UE process")
print("- HMMWV/M2/BTR source intake searches existing project sources and common Windows download locations")
print("- BTR production intake requires and restores the six known original texture files")
print("- HMMWV/M2/BTR production ingest gate remains in the normal launcher")
print("- local tracer can rebase its target-side network streak to the visible muzzle/barrel")
print("- BASE source remains tied to the canonical Museum test hub")
print("- dense foliage remains batched instead of blocking the deployment frame")
print("- R10 retains the real UI shadow check without the unrelated global spelling false positive")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
