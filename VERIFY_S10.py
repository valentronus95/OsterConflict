from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
REQ = [
    "Source/OsterConflict/Public/OCVehicleBase.h",
    "Source/OsterConflict/Private/OCVehicleBase.cpp",
    "Source/OsterConflict/Public/OCCivilianVehicle.h",
    "Source/OsterConflict/Private/OCCivilianVehicle.cpp",
    "Source/OsterConflict/Public/OCVehicleSpawnPoint.h",
    "Source/OsterConflict/Private/OCVehicleSpawnPoint.cpp",
    "Source/OsterConflict/Public/OCCharacter.h",
    "Source/OsterConflict/Private/OCCharacter.cpp",
    "Source/OsterConflict/Private/OCGameMode.cpp",
    "Source/OsterConflict/Private/OCHUD.cpp",
    "Docs/SESSION_10_README_UA.md",
    "Docs/VEHICLE_ARCHITECTURE_S10.md",
    "Docs/S10_TEST_MATRIX.md",
]
for rel in REQ:
    p = PROJECT / rel
    assert p.exists(), f"missing {rel}"

blob = "\n".join((PROJECT / rel).read_text(errors="ignore") for rel in REQ if rel.endswith((".h", ".cpp")))
markers = [
    "AOCVehicleBase", "AOCCivilianVehicle", "AOCVehicleSpawnPoint",
    "ServerSetDriveInputs", "TryEnterVehicleServer", "ForceExitDriverServer",
    "SuspensionPointsLocal", "AddForceAtLocation", "AddTorqueInRadians",
    "DriveForce", "LateralGrip", "SteeringTorque", "bHandbrake",
    "InteriorCamera", "ThirdPersonCamera", "FreeLook", "ToggleCamera",
    "VehicleHealth", "EOCVehicleDamageStage", "WreckLifetimeSeconds",
    "OnVehicleWrecked", "RespawnDelaySeconds", "SpawnCivilianVehicleFleet",
    "bInVehicle", "EnterVehicleServer", "ExitVehicleServer",
    "DrawVehicleHUD", "E  ENTER VEHICLE",
]
for m in markers:
    assert m in blob, f"missing marker {m}"

# Basic delimiter sanity after stripping comments/strings.
for p in list((PROJECT / "Source/OsterConflict/Public").glob("*.h")) + list((PROJECT / "Source/OsterConflict/Private").glob("*.cpp")):
    text = p.read_text(errors="ignore")
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    stack = []
    pairs = {'}':'{', ')':'(', ']':'['}
    for ch in text:
        if ch in "{([": stack.append(ch)
        elif ch in "})]":
            assert stack and stack[-1] == pairs[ch], f"delimiter mismatch in {p.name}"
            stack.pop()
    assert not stack, f"unclosed delimiter in {p.name}"

# Server RPC declarations must have implementations.
for cls in ("OCVehicleBase", "OCCharacter"):
    h = (PROJECT / f"Source/OsterConflict/Public/{cls}.h").read_text()
    c = (PROJECT / f"Source/OsterConflict/Private/{cls}.cpp").read_text()
    rpcs = re.findall(r"UFUNCTION\(Server,[^)]*\)\s*\n\s*void\s+(\w+)\s*\(", h)
    for rpc in rpcs:
        assert f"{cls}::{rpc}_Implementation" in c, f"missing RPC impl {cls}::{rpc}"

print("S10 structural verification: PASS")
print(f"Checked {len(REQ)} required files and {len(markers)} S10 markers.")
