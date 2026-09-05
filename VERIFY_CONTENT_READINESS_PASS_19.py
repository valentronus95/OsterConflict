#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS19 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS19 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS19 VERIFY FAIL: {label}: forbidden {needle!r}")


fallback = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
pass19 = read(SRC / "Private" / "OCContentReadinessPass19Subsystem.cpp")
vehicle_validation = read(SRC / "Private" / "OCProductionVehicleRuntimeValidationSubsystem.cpp")
catalog = read(SRC / "Private" / "OCPass45WeaponCatalogSpawnSubsystem.cpp")
launcher = read(ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd")
vehicle_import = read(ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py")
vehicle_cmd = read(ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_try = read(ROOT / "OsterConflict" / "TRY_PRODUCTION_VEHICLES_UE58.cmd")
vehicle_fresh = read(ROOT / "OsterConflict" / "Scripts" / "verify_production_vehicle_fresh_load.py")
m2_launcher = read(ROOT / "RUN_IMPORT_M2_PRODUCTION.cmd")
btr_launcher = read(ROOT / "RUN_IMPORT_BTR4_PRODUCTION.cmd")

require(fallback, 'RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"))', "fallback identity")
require(fallback, "exact_production=0 playable_fallback=1", "fallback truth log")
forbid(fallback, "Visual->ComponentTags.Add(ProductionVisualTag);", "generic fallback pretending to be production")

# Vehicle validator now owns vehicles only. Weapon completeness/exact-visual truth
# moved to the single current full-catalog owner; do not resurrect a second weapon gate here.
require(
    vehicle_validation,
    "UOCPass45WeaponCatalogSpawnSubsystem is the single current owner of the complete weapon catalog and exact-visual validation.",
    "vehicle-to-weapon ownership delegation",
)
for needle in (
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
):
    forbid(vehicle_validation, needle, "obsolete weapon gate in vehicle validator")

for needle in (
    "const FWeaponCatalogEntry WeaponCatalog[]",
    "constexpr int32 CoreRackEntryCount = 7",
    "OC_ProductionWeaponVisual",
    "PASS45_COMPLETE_WEAPON_RACK_READY",
    "PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_READY",
    "PASS45_COMPLETE_WEAPON_CATALOG_VISUAL_GAP",
    "missing_ids=0",
    "missing_exact_visuals=0",
    "duplicate_weapon_ids=0",
    "wrong_identity_substitution=0",
    "runtime_acceptance=0",
    "AOCWeapon_M4A1::StaticClass()",
    "AOCWeapon_AR15::StaticClass()",
):
    require(catalog, needle, "current full weapon-catalog owner")

# The older Pass19 11-class focused recovery rack remains a compatibility route,
# but it no longer owns the complete PASS45 catalog.
for needle in (
    "AllRequiredRackWeaponClassesMask", "OC_RuntimeBaseWeaponRack", "OC_ProductionWeaponVisual",
    "OC_RealFallbackWeaponVisual", "AOCWeapon_M14", "AOCWeapon_Mac10", "AOCWeapon_Tec9",
    "AOCWeapon_LeverAction", "AOCAntiArmorLauncher", "PrimitiveOnlyCount == 0",
    "ExactProductionCount + RealFallbackCount", "SetActorHiddenInGame(false)",
    "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL",
):
    require(pass19, needle, "focused playable weapon readiness")

require(launcher, "PASS19_PLAYABLE_WEAPON_SET_READY", "focused launcher playable gate")
require(launcher, "PASS19_PLAYABLE_WEAPON_SET_FAIL", "focused launcher failure gate")
forbid(launcher, "PASS7_PRODUCTION_WEAPONS_READY", "focused launcher exact-art false certification")

for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'attempt("HMMWV"',
    'attempt("M2"',
    "def import_btr4(",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual_canonical_plus_x",
    "M_BTR4_OC_Authored",
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "canonical import skips it",
    "other independent assets will continue",
    "CONTENT_GAP=",
):
    require(vehicle_import, needle, "current production vehicle source truth")
forbid(vehicle_import, 'attempt("BTR4"', "obsolete BTR source-missing attempt path")
forbid(vehicle_import, "ensure_sources_exist()", "obsolete all-or-nothing production source gate")
forbid(vehicle_import, "PASS45_MATERIAL_CLOSURE_20260826_R1", "stale pre-axis BTR import revision")
forbid(vehicle_import, "PASS45_BTR_AXIS_OPTIC_20260827_R2", "sideways BTR R2 import revision")

for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    'set "BTR_AXIS_READY=0"',
    'set "BTR_GLTF_UP_READY=0"',
    'set "BTR_INTERNAL_UP_READY=0"',
    "canonical BTR authored fallback remains available.",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "PASS45_BTR_GLTF_Y_UP_20260827_R3",
):
    require(vehicle_cmd, needle, "independent production vehicle command truth")
forbid(vehicle_cmd, "PASS45_MATERIAL_CLOSURE_20260826_R1", "stale pre-axis command revision")
forbid(vehicle_cmd, "PASS45_BTR_AXIS_OPTIC_20260827_R2", "sideways BTR R2 command revision")

for needle in (
    "PASS45_BTR_GLTF_Y_UP_20260827_R3",
    "production_import_success.txt",
    "production_fresh_load_success.txt",
    "Existing .uasset files are not sufficient proof of current materials/orientation.",
    "FRESH_LOADED=/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
):
    require(vehicle_try, needle, "current production freshness gate")
forbid(vehicle_try, "PASS45_MATERIAL_CLOSURE_20260826_R1", "stale pre-axis quick-intake revision")
forbid(vehicle_try, "PASS45_BTR_AXIS_OPTIC_20260827_R2", "sideways BTR R2 quick-intake revision")

for needle in (
    "AUTHORED_MATERIALS_READY",
    "placeholder_slots",
    "basicshapematerial",
    "defaultmaterial",
    "_defaultmat",
    "PASS45_BTR_GLTF_Y_UP_20260827_R3",
    "M_BTR4_OC_Authored",
    "BTR4_FORWARD_AXIS",
    "BTR4_GLTF_UP_AXIS",
    "BTR4_INTERNAL_UP_AXIS",
    'source_kind != "authored_external_visual_canonical_plus_x"',
    "SOURCE_KIND=BTR4:",
):
    require(vehicle_fresh, needle, "fresh-load authored material/axis truth")
forbid(vehicle_fresh, "PASS45_MATERIAL_CLOSURE_20260826_R1", "stale pre-axis fresh-load revision")
forbid(vehicle_fresh, "PASS45_BTR_AXIS_OPTIC_20260827_R2", "sideways BTR R2 fresh-load revision")

require(m2_launcher, "source_kind=downloaded", "real M2 source requirement")
require(btr_launcher, "source_kind=local_user_fbx", "dedicated local BTR4 source helper")

print("CONTENT READINESS PASS 19 + PASS45 BTR R3 MATERIAL/AXIS INTAKE CONTRACT PASS")
print("- generic weapon fallback meshes do not impersonate production art")
print("- full weapon-catalog exact-visual validation is owned only by OCPass45WeaponCatalogSpawnSubsystem")
print("- Pass 19 separately preserves its focused 11-class playable real-mesh compatibility rack")
print("- HMMWV/M2 remain independent external-source imports")
print("- BTR canonical runtime intake is repository-authored +X-forward with explicit glTF +Y-up/internal +Z-up provenance")
print("- imported vehicle meshes must reopen under R3 with authored non-placeholder materials and complete BTR axis provenance")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and rendered asset intake remain required")
