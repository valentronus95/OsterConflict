from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
RAW = PROJECT / "Content" / "Raw" / "R13" / "Weapons" / "SteinClassicWeapons" / "WeaponsPack"
IMPORTER = PROJECT / "Scripts" / "R13" / "IMPORT_R13_CONTENT.py"
RUNTIME = PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13WeaponArtSubsystem.cpp"
READY = ROOT / "PC_TEST" / "CHECK_R13_LAUNCH_READY.ps1"
DOWNLOAD = ROOT / "R13_DOWNLOAD_AND_IMPORT_CONTENT.cmd"

weapons = {
    "1911": "SKM_1911.fbx",
    "AK47": "SKM_AK47.fbx",
    "LeverAction": "SKM_LeverAction.fbx",
    "M14": "SKM_M14.fbx",
    "M700": "SKM_M700.fbx",
    "MP5": "SKM_MP5.fbx",
    "Mac10": "SKM_Mac10.fbx",
    "Tec9": "SKM_Tec9.fbx",
}

missing = [str((RAW / folder / filename).relative_to(ROOT)) for folder, filename in weapons.items()
           if not (RAW / folder / filename).exists()]
if missing:
    print("R13 Stein weapons verification: FAIL")
    print("Missing source FBX:", *missing, sep="\n - ")
    sys.exit(1)

license_path = RAW / "license.txt"
if not license_path.exists():
    raise SystemExit("R13 Stein weapons verification: FAIL - license.txt missing")
license_text = license_path.read_text(encoding="utf-8", errors="replace")
if "CC0 1.0" not in license_text or "Stein Games" not in license_text:
    raise SystemExit("R13 Stein weapons verification: FAIL - expected Stein Games CC0 1.0 license marker missing")

importer = IMPORTER.read_text(encoding="utf-8")
runtime = RUNTIME.read_text(encoding="utf-8")
ready = READY.read_text(encoding="utf-8")
download = DOWNLOAD.read_text(encoding="utf-8")

checks = [
    ("FBX importer explicitly selects static mesh", "FBXIT_STATIC_MESH" in importer and "import_as_skeletal" in importer),
    ("FBX importer combines weapon mesh parts", 'set_editor_property("combine_meshes", True)' in importer),
    ("all eight Stein source folders are imported", all(folder in importer and filename in importer for folder, filename in weapons.items())),
    ("runtime pistol uses Stein 1911", "/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911" in runtime),
    ("runtime SMG uses Stein MP5", "/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5" in runtime),
    ("runtime sniper uses Stein M700", "/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700" in runtime),
    ("working Fab AK is intentionally retained", "/Game/AK-47/Mesh/SM_AK-47.SM_AK-47" in runtime),
    ("Stein meshes do not receive old x100 placeholder scale", "IsSteinMesh" in runtime and "FVector(1.0f)" in runtime),
    ("V3 launch state is required", "R13_STEIN_WEAPONS_V3" in ready and "R13_STEIN_WEAPONS_V3" in download),
    ("launch gate checks Stein asset folders", "SteinWeapons" in ready and "SKM_1911.uasset" in ready and "SKM_MP5.uasset" in ready),
]
failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 Stein weapons verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 Stein weapons verification: PASS")
print(f"Checked {len(weapons)} committed FBX models, CC0 provenance and {len(checks)} import/runtime gates.")
