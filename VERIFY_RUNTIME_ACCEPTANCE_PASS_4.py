from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent

FILES = {
    "launcher": ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
    "weapon_preflight": ROOT / "OsterConflict/Scripts/verify_required_weapon_assets.py",
    "lfs_verify": ROOT / "OsterConflict/Scripts/verify_playtest_lfs_payloads.ps1",
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
lfs_verify = read("lfs_verify")
source_recovery = read("source_recovery")
fx = read("fx")
spawn = read("spawn")
foliage = read("foliage")
r10 = read("r10")

for needle in (
    "verify_required_weapon_assets.py",
    "required_weapon_asset_preflight_success.txt",
    "Opening every required REAL/playable weapon visual in a fresh UE process",
    "Primitive-only weapon boxes are not accepted for the normal playtest.",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
):
    require(launcher, needle, "normal gameplay launcher gate")

# Pass 20 keeps the production intake but moves it behind strict acceptance rather than blocking normal play.
for needle in (
    'if "%IS_ACCEPTANCE%"=="1" (',
    "[3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets",
    'call "%PRODUCTION_IMPORT%"',
    "[3/4] NORMAL GAME: skipping strict production vehicle intake.",
    "Exact HMMWV/M2/BTR production source files remain an open content gap",
):
    require(launcher, needle, "Pass 20 strict/normal production split")
strict_stage = launcher.find("[3/4] STRICT ACCEPTANCE")
acceptance_gate = launcher.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = launcher.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = launcher.find(") else (", strict_stage)
if strict_stage < 0 or acceptance_gate < 0 or import_call < 0 or normal_else < 0 or not (acceptance_gate < strict_stage < import_call < normal_else):
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: production importer escaped strict acceptance")

for needle in (
    "verify_playtest_lfs_payloads.ps1",
    "git lfs pull origin",
    "git lfs checkout >nul",
    'powershell -NoProfile -ExecutionPolicy Bypass -File "%LFS_VERIFY_PS%"',
):
    require(launcher, needle, "Windows LFS launcher precheck")
if "--include=" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: unsupported Git LFS --include flag returned to Windows launcher")
if "^|" in launcher:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: cmd caret-pipe leaked into inline PowerShell again")

for needle in (
    "Content\\AK-47",
    "Content\\R13\\Weapons",
    "Content\\PN_FoliageCollection",
    "version https://git-lfs.github.com/spec/v1",
    "$MissingRoots",
    "$Bad | Select-Object -First 20 | ForEach-Object",
):
    require(lfs_verify, needle, "PowerShell LFS payload verifier")
if "^|" in lfs_verify:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: invalid cmd caret escaping present in PowerShell verifier")

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
    "IsOnAimRay",
    "ResolveFiringWeapon",
    "Character->GetCurrentWeapon()",
    "ResolveWeaponMuzzle",
    "bRebasedToMuzzle",
    "ComponentName.Contains(TEXT(\"barrel\")",
    "ComponentName.Contains(TEXT(\"muzzle\")",
    "FMath::Min(DistanceToEnd, 900.0f)",
    "TryResolveSocketMuzzle",
    "const FBoxSphereBounds LocalBounds = Component.GetLocalBounds();",
    "LocalBounds.Origin - LocalBounds.BoxExtent",
    "LocalBounds.Origin + LocalBounds.BoxExtent",
    "const FVector VisualStart = ResolveWeaponMuzzle",
    "const FVector VisualMuzzle = ResolveWeaponMuzzle",
):
    require(fx, needle, "muzzle/tracer source")
if "Component.GetLocalBounds(LocalMin, LocalMax)" in fx:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: obsolete two-argument GetLocalBounds call is incompatible with UE 5.8")

for needle in (
    "AOCWorldSectorOster::MuseumAnchor()",
    "FVector(-1450.0f, -900.0f, 120.0f)",
    "FVector(1450.0f, 900.0f, 120.0f)",
    "SpawnRuntimeBaseWeaponRack",
    "SnapLocationToWalkableSurface",
):
    require(spawn, needle, "museum base spawn")

for needle in (
    "PopulateBatch",
    "PopulationBatchTimer",
):
    require(foliage, needle, "batched dense foliage")
batch_match = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage)
if not batch_match:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: foliage batch-size contract is missing")
batch_size = int(batch_match.group(1))
if not 1 <= batch_size <= 96:
    raise SystemExit(f"RUNTIME ACCEPTANCE PASS 4 FAIL: foliage batch size {batch_size} exceeds accepted non-blocking ceiling 96")

if "'if (UVerticalBoxSlot* Slot'," in r10 or "'if (UCanvasPanelSlot* Slot'," in r10 or "'if (UHorizontalBoxSlot* Slot'," in r10:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 4 FAIL: R10 global Slot false-positive tokens returned")
require(r10, "UI Slot shadow names removed", "R10 file-specific UI shadow contract")

print("RUNTIME ACCEPTANCE PASS 4 SOURCE CONTRACT PASS")
print("- normal gameplay hard-gates required real/playable weapon assets in a fresh UE process")
print("- Windows launcher uses Git LFS commands compatible with the playtest PC and a separate PowerShell verifier")
print("- HMMWV/M2/BTR source intake searches existing project sources and common Windows download locations")
print("- BTR production intake requires and restores the six known original texture files")
print("- HMMWV/M2/BTR production ingest remains mandatory in strict acceptance but no longer blocks normal frontend launch")
print("- tracer/muzzle presentation resolves the actual firing CurrentWeapon, not only the first local pawn")
print("- muzzle bounds fallback uses the UE 5.8 return-value GetLocalBounds API")
print("- BASE source remains tied to the canonical Museum test hub")
print("- dense foliage remains batched with an explicit non-blocking batch-size ceiling")
print("- R10 retains the real UI shadow check without the unrelated global spelling false positive")
print("STATUS: CODED_UNTESTED; local UE 5.8 build/playtest still required")
