from pathlib import Path
import sys

root = Path(__file__).resolve().parent / "OsterConflict"
required = [
    "OsterConflict.uproject",
    "Source/OsterConflict/Public/OCWeaponTypes.h",
    "Source/OsterConflict/Public/OCWeaponDefinition.h",
    "Source/OsterConflict/Public/OCWeaponBase.h",
    "Source/OsterConflict/Private/OCWeaponBase.cpp",
    "Source/OsterConflict/Public/OCWeaponVariants.h",
    "Source/OsterConflict/Private/OCWeaponVariants.cpp",
    "Source/OsterConflict/Public/OCAmmoBox.h",
    "Source/OsterConflict/Private/OCAmmoBox.cpp",
    "Source/OsterConflict/Public/OCCharacter.h",
    "Source/OsterConflict/Private/OCCharacter.cpp",
    "Source/OsterConflict/Private/OCHUD.cpp",
    "Docs/SESSION_04_README_UA.md",
    "Docs/S04_TEST_MATRIX.md",
    "Docs/WEAPON_ARCHITECTURE_S04.md",
]

missing = [p for p in required if not (root / p).exists()]
if missing:
    print("S04 structural verification: FAIL")
    print("Missing:", *missing, sep="\n - ")
    sys.exit(1)

checks = {
    "PrimaryDataAsset": (root / "Source/OsterConflict/Public/OCWeaponDefinition.h", "public UPrimaryDataAsset"),
    "Primary slot": (root / "Source/OsterConflict/Public/OCCharacter.h", "PrimaryWeapon"),
    "Secondary slot": (root / "Source/OsterConflict/Public/OCCharacter.h", "SecondaryWeapon"),
    "Inventory replication": (root / "Source/OsterConflict/Private/OCCharacter.cpp", "DOREPLIFETIME(AOCCharacter, ActiveWeaponSlot)"),
    "Server interaction": (root / "Source/OsterConflict/Private/OCCharacter.cpp", "ServerInteract_Implementation"),
    "Server drop": (root / "Source/OsterConflict/Private/OCCharacter.cpp", "ServerDropCurrentWeapon_Implementation"),
    "Server slot switch": (root / "Source/OsterConflict/Private/OCCharacter.cpp", "ServerEquipWeaponSlot_Implementation"),
    "World pickup state": (root / "Source/OsterConflict/Private/OCWeaponBase.cpp", "DropToWorldServer"),
    "Ammo box": (root / "Source/OsterConflict/Private/OCAmmoBox.cpp", "AddAmmoFromBoxServer"),
    "Attachments replicate": (root / "Source/OsterConflict/Private/OCWeaponBase.cpp", "DOREPLIFETIME(AOCWeaponBase, Attachments)"),
    "Red dot modifier": (root / "Source/OsterConflict/Private/OCWeaponBase.cpp", "RedDot"),
    "Extended magazine": (root / "Source/OsterConflict/Private/OCWeaponBase.cpp", "ExtendedMag"),
    "Shotgun pellets": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "PelletsPerShot = 8"),
    "Assault rifle": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_AssaultRifle::AOCWeapon_AssaultRifle"),
    "SMG": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_SMG::AOCWeapon_SMG"),
    "Pistol": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_Pistol::AOCWeapon_Pistol"),
    "Sniper": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_Sniper::AOCWeapon_Sniper"),
    "Shotgun": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_Shotgun::AOCWeapon_Shotgun"),
    "LMG": (root / "Source/OsterConflict/Private/OCWeaponVariants.cpp", "AOCWeapon_LMG::AOCWeapon_LMG"),
    "Pickup prompt": (root / "Source/OsterConflict/Private/OCHUD.cpp", "InteractionPrompt"),
}

for name, (path, marker) in checks.items():
    text = path.read_text(encoding="utf-8")
    if marker not in text:
        print(f"S04 structural verification: FAIL — {name}")
        sys.exit(1)

# Lightweight source sanity: delimiters and stale S01 single-weapon markers.
for path in (root / "Source").rglob("*"):
    if path.suffix not in {".h", ".cpp", ".cs"}:
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for left, right in (("{", "}"), ("(", ")"), ("[", "]")):
        if text.count(left) != text.count(right):
            print(f"S04 structural verification: FAIL — delimiter mismatch in {path}")
            sys.exit(1)

character = (root / "Source/OsterConflict/Private/OCCharacter.cpp").read_text(encoding="utf-8")
if "StarterWeaponClass" in character or "AttachCurrentWeapon" in character:
    print("S04 structural verification: FAIL — stale single-weapon implementation remains")
    sys.exit(1)

print("S04 structural verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} weapon/inventory markers.")
