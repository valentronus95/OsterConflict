#!/usr/bin/env python3
"""Pass45 item 30 source gate: BTR canonical +X forward axis and remote optic gameplay."""

from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCBTR.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCBTR.cpp"
GENERATOR = ROOT / "OsterConflict" / "Scripts" / "generate_btr4_game_visual.py"
IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_btr4_production_asset.py"
VEHICLE_IMPORTER = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 BTR4 AXIS REMOTE OPTIC: FAIL: {message}")


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


header = read(HEADER)
cpp = read(CPP)
generator = read(GENERATOR)
importer = read(IMPORTER)
vehicle_importer = read(VEHICLE_IMPORTER)

for needle in (
    "virtual void Tick(float DeltaSeconds) override;",
    "BTRRemoteOpticFieldOfView = 48.0f",
    "ActiveOpticGunner",
):
    if needle not in header:
        fail(f"BTR header missing {needle!r}")

for needle in (
    "GunnerCameraPivot->SetupAttachment(BarrelPivot);",
    "GunnerCameraPivot->GetAttachParent() == BarrelPivot",
    "PASS45_BTR4_REMOTE_OPTIC_READY",
    "follows_yaw=1 follows_pitch=1",
    "PlayerCameraManager->SetFOV(BTRRemoteOpticFieldOfView)",
    "PlayerCameraManager->UnlockFOV()",
    "PASS45_BTR4_FORWARD_AXIS_READY",
    "canonical_forward=+X",
    "NativeSize.X >= NativeSize.Y && NativeSize.X >= NativeSize.Z",
    "const FQuat AxisCorrection = FQuat::Identity;",
):
    if needle not in cpp:
        fail(f"BTR runtime contract missing {needle!r}")

if "ResolveLongAxisToForward" in cpp:
    fail("ambiguous longest-axis helper was resurrected; forward sign would again be guessed")

if "+X is vehicle forward" not in generator:
    fail("authored BTR generator no longer declares +X as forward")

for needle in (
    'source_kind = "authored_external_visual_canonical_plus_x"',
    "forward_axis=+X",
):
    if needle not in importer:
        fail(f"dedicated BTR importer missing {needle!r}")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_AXIS_OPTIC_20260827_R2"',
    "SOURCE_KIND=BTR4:authored_external_visual_canonical_plus_x",
    "BTR4_FORWARD_AXIS=+X",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
):
    if needle not in vehicle_importer:
        fail(f"main production importer missing {needle!r}")

if "SOURCE_KIND=BTR4:local_user_fbx" in vehicle_importer:
    fail("uncalibrated local FBX is still auto-promoted to canonical runtime BTR")

print("PASS45 BTR4 AXIS REMOTE OPTIC: PASS")
print("- canonical runtime BTR source is the authored +X-forward GLB")
print("- runtime refuses ambiguous axis transposition instead of guessing the nose")
print("- BTR gunner viewpoint follows yaw + pitch through BarrelPivot with a locked remote-optic FOV")
print("- local uncalibrated FBX remains development-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 orientation/view gameplay acceptance remains required")
