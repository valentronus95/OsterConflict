from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
REQ = [
    "Source/OsterConflict/Public/OCDamageTypes.h",
    "Source/OsterConflict/Private/OCDamageTypes.cpp",
    "Source/OsterConflict/Public/OCArmedVehicleBase.h",
    "Source/OsterConflict/Private/OCArmedVehicleBase.cpp",
    "Source/OsterConflict/Public/OCPickupGunTruck.h",
    "Source/OsterConflict/Private/OCPickupGunTruck.cpp",
    "Source/OsterConflict/Public/OCBTR.h",
    "Source/OsterConflict/Private/OCBTR.cpp",
    "Source/OsterConflict/Public/OCCombatVehicleSpawnPoints.h",
    "Source/OsterConflict/Private/OCCombatVehicleSpawnPoints.cpp",
    "Source/OsterConflict/Public/OCCharacter.h",
    "Source/OsterConflict/Private/OCCharacter.cpp",
    "Source/OsterConflict/Private/OCGameMode.cpp",
    "Source/OsterConflict/Private/OCHUD.cpp",
    "Docs/SESSION_11_README_UA.md",
    "Docs/ARMED_VEHICLE_ARCHITECTURE_S11.md",
    "Docs/S11_TEST_MATRIX.md",
    "Docs/ROADMAP_SESSIONS.md",
]
for rel in REQ:
    assert (PROJECT / rel).exists(), f"missing {rel}"

blob = "\n".join((PROJECT / rel).read_text(encoding="utf-8", errors="ignore") for rel in REQ)
markers = [
    "AOCArmedVehicleBase", "AOCPickupGunTruck", "AOCBTR",
    "AOCPickupGunTruckSpawnPoint", "AOCBTRSpawnPoint",
    "GunnerCharacter", "OccupantTeam", "TurretYaw", "TurretPitch",
    "TurretAmmoInMagazine", "TurretReserveAmmo", "bTurretReloading",
    "SetGunnerAimServer", "SetGunnerFireHeldServer", "RequestGunnerReloadServer",
    "EnterVehicleGunnerServer", "ExitVehicleGunnerServer", "bVehicleGunner",
    "ServerSetVehicleGunnerAim", "ServerSetVehicleGunnerFireHeld", "ServerReloadVehicleTurret",
    "HasDriver()", "E  СІСТИ ЗА КЕРМО", "E  СІСТИ ЗА КУЛЕМЕТ",
    "UOCBallisticDamageType", "UOCVehicleCannonDamageType", "UOCAntiArmorDamageType",
    "CanHullAcceptDamage", "GetCollisionDamageScale() const override { return 0.0f; }",
    "SpawnCombatVehicleFleet", "DrawGunnerHUD",
    "S11 — Armed vehicles [ВИКОНАНО В ЦЬОМУ АРХІВІ]",
]
for marker in markers:
    assert marker in blob, f"missing marker {marker}"

# Ensure BTR's core rule is actually coded, not only documented.
btr_cpp = (PROJECT / "Source/OsterConflict/Private/OCBTR.cpp").read_text(encoding="utf-8")
assert "IsChildOf(UOCAntiArmorDamageType::StaticClass())" in btr_cpp

# Source-only delimiter sanity for all headers/cpps.
for p in list((PROJECT / "Source/OsterConflict/Public").glob("*.h")) + list((PROJECT / "Source/OsterConflict/Private").glob("*.cpp")):
    text = p.read_text(encoding="utf-8", errors="ignore")
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

# Character client->server gunner RPCs must have implementations.
ch = (PROJECT / "Source/OsterConflict/Public/OCCharacter.h").read_text(encoding="utf-8")
cc = (PROJECT / "Source/OsterConflict/Private/OCCharacter.cpp").read_text(encoding="utf-8")
for rpc in re.findall(r"UFUNCTION\(Server,[^)]*\)\s*\n\s*void\s+(\w+)\s*\(", ch):
    assert f"AOCCharacter::{rpc}_Implementation" in cc, f"missing Character RPC impl {rpc}"

print("S11 structural verification: PASS")
print(f"Checked {len(REQ)} required files and {len(markers)} S11 markers.")
