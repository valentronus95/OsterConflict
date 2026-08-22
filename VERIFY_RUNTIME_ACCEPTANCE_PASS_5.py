from pathlib import Path

ROOT = Path(__file__).resolve().parent

VARIANTS = ROOT / "OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp"
VALIDATOR = ROOT / "OsterConflict/Source/OsterConflict/Private/OCProductionWeaponRuntimeValidationSubsystem.cpp"
PREFLIGHT = ROOT / "OsterConflict/Scripts/verify_required_weapon_assets.py"


def read(path):
    if not path.is_file():
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 5 FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text, needle, where):
    if needle not in text:
        raise SystemExit(f"RUNTIME ACCEPTANCE PASS 5 FAIL: {where}: missing {needle!r}")


variants = read(VARIANTS)
validator = read(VALIDATOR)
preflight = read(PREFLIGHT)

# AK is the one verified animated skeletal asset in this family.
require(preflight, '("AK-47", "/Game/AK-47/Mesh/SKM_AK-47", unreal.SkeletalMesh)', "AK preflight")
require(validator, 'TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"), EExpectedWeaponMeshKind::Skeletal', "AK runtime validator")

# UE 5.8 local runtime reports these restored Stein assets as StaticMesh despite the SKM_* names.
static_assets = (
    ("MP5", "/Game/R13/Weapons/Stein/MP5/SKM_MP5", "SKM_MP5.SKM_MP5"),
    ("M1911", "/Game/R13/Weapons/Stein/1911/SKM_1911", "SKM_1911.SKM_1911"),
    ("M700", "/Game/R13/Weapons/Stein/M700/SKM_M700", "SKM_M700.SKM_M700"),
    ("M14", "/Game/R13/Weapons/Stein/M14/SKM_M14", "SKM_M14.SKM_M14"),
    ("MAC-10", "/Game/R13/Weapons/Stein/Mac10/SKM_Mac10", "SKM_Mac10.SKM_Mac10"),
    ("TEC-9", "/Game/R13/Weapons/Stein/Tec9/SKM_Tec9", "SKM_Tec9.SKM_Tec9"),
    ("Lever Action", "/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction", "SKM_LeverAction.SKM_LeverAction"),
)

for label, asset_path, object_tail in static_assets:
    require(preflight, f'("{label}", "{asset_path}", unreal.StaticMesh)', f"{label} preflight StaticMesh contract")
    # The runtime validator uses the object path form with .ObjectName and must expect Static.
    object_path = asset_path + "." + object_tail.split(".")[-1]
    require(validator, f'TEXT("{object_path}"), EExpectedWeaponMeshKind::Static', f"{label} runtime StaticMesh contract")

# The weapon BeginPlay path still prefers a true skeletal asset when present, but now falls back to
# loading the exact same canonical object as StaticMesh before any primitive fallback can remain visible.
require(variants, "UPrimitiveComponent* ApplySkeletalProductionWeapon", "dual-class production weapon loader")
require(variants, "return ApplyStaticProductionWeapon(Owner, Root, AssetPath, ComponentBaseName, DesiredLengthCm);", "StaticMesh fallback for SKM-named R13 assets")
require(variants, "OC_ProductionWeaponVisual", "production visual tag")

if 'unreal.SkeletalMesh),\n    ("MP5"' in preflight:
    raise SystemExit("RUNTIME ACCEPTANCE PASS 5 FAIL: MP5 reverted to false SkeletalMesh preflight contract")

print("RUNTIME ACCEPTANCE PASS 5 SOURCE CONTRACT PASS")
print("- local UE 5.8 asset classes, not SKM_ filename prefixes, define the R13 Stein mesh contract")
print("- AK remains skeletal; MP5/1911/M700/M14/MAC10/TEC9/LeverAction are validated as static")
print("- weapon BeginPlay accepts either a genuine skeletal object or the exact canonical static object")
print("STATUS: CODED_UNTESTED; local UE 5.8 preflight/build remains authoritative")
