#!/usr/bin/env python3
"""Pass45 item 30 source gate: BTR canonical +X forward, glTF Y-up and remote optic gameplay."""

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

# 2026-08-27 runtime rejection proved that declaring internal +Z up is not enough for a glTF file.
# glTF is Y-up; the generated root must explicitly map internal +Z -> glTF +Y while preserving +X forward.
for needle in (
    "Internal modeling coordinates are +X forward, +Y lateral, +Z up",
    "glTF is Y-up",
    "BTR4_Z_UP_TO_GLTF_Y_UP_ROTATION = [-0.7071067811865476, 0.0, 0.0, 0.7071067811865476]",
    '"rotation": BTR4_Z_UP_TO_GLTF_Y_UP_ROTATION',
    '"internal_axis_contract": "+X forward, +Z up"',
    '"gltf_axis_contract": "+X forward, +Y up"',
):
    if needle not in generator:
        fail(f"authored BTR glTF up-axis guard missing {needle!r}")

for needle in (
    'source_kind = "authored_external_visual_canonical_plus_x"',
    "forward_axis=+X",
):
    if needle not in importer:
        fail(f"dedicated BTR importer missing {needle!r}")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    "SOURCE_KIND=BTR4:authored_external_visual_canonical_plus_x",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
):
    if needle not in vehicle_importer:
        fail(f"main production importer missing {needle!r}")

if "PASS45_BTR_AXIS_OPTIC_20260827_R2" in vehicle_importer:
    fail("stale R2 import revision can reuse the sideways BTR asset")
if "SOURCE_KIND=BTR4:local_user_fbx" in vehicle_importer:
    fail("uncalibrated local FBX is still auto-promoted to canonical runtime BTR")

print("PASS45 BTR4 AXIS REMOTE OPTIC: PASS")
print("- canonical runtime BTR source is authored +X-forward internal geometry exported through an explicit glTF Y-up root")
print("- R3 import revision forces replacement of any stale sideways R2 asset")
print("- runtime refuses ambiguous forward-axis transposition instead of guessing the nose")
print("- BTR gunner viewpoint follows yaw + pitch through BarrelPivot with a locked remote-optic FOV")
print("- local uncalibrated FBX remains development-only")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 orientation/view gameplay acceptance remains required")
