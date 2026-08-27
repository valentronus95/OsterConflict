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
strict = read(SRC / "Private" / "OCProductionVehicleRuntimeValidationSubsystem.cpp")
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

for needle in (
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
    "OC_ProductionWeaponVisual",
    "OC_RealFallbackWeaponVisual",
    "exactProductionReadyNotClaimed=1",
    "validation_only=1 mutation=0",
):
    require(strict, needle, "required-available weapon runtime gate")
forbid(strict, "PASS7_PRODUCTION_WEAPONS_READY", "obsolete all-exact rack readiness")
forbid(strict, "PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL", "obsolete all-exact rack failure")

for needle in (
    "AllRequiredRackWeaponClassesMask", "OC_RuntimeBaseWeaponRack", "OC_ProductionWeaponVisual",
    "OC_RealFallbackWeaponVisual", "AOCWeapon_M14", "AOCWeapon_Mac10", "AOCWeapon_Tec9",
    "AOCWeapon_LeverAction", "AOCAntiArmorLauncher", "PrimitiveOnlyCount == 0",
    "ExactProductionCount + RealFallbackCount", "SetActorHiddenInGame(false)",
    "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL",
):
    require(pass19, needle, "playable weapon readiness")

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
print("- Gate F validates exact production OR explicit real fallback while exact payload gaps stay CONTENT GAP")
print("- Pass 19 separately proves an 11-class playable real-mesh rack")
print("- HMMWV/M2 remain independent external-source imports")
print("- BTR canonical runtime intake is repository-authored +X-forward with explicit glTF +Y-up/internal +Z-up provenance")
print("- imported vehicle meshes must reopen under R3 with authored non-placeholder materials and complete BTR axis provenance")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and rendered asset intake remain required")
