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

# Generic real meshes are explicitly fallback-only and can never manufacture a production-ready tag.
require(fallback, 'RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"))', "fallback identity")
require(fallback, "exact_production=0 playable_fallback=1", "fallback truth log")
forbid(fallback, "Visual->ComponentTags.Add(ProductionVisualTag);", "generic fallback pretending to be production")

# Exact production validation remains present and independent.
for needle in ("PASS7_PRODUCTION_WEAPONS_READY", "PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL", "OC_ProductionWeaponVisual"):
    require(strict, needle, "strict production gate")

# Pass 19 proves all 11 classes are playable through exact OR explicit real fallback visuals.
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

# Pass45 supersedes the old all-or-nothing / absent-BTR contract. HMMWV and M2 remain independent external
# sources. BTR resolves through a dedicated canonical path: local user FBX when available, otherwise the
# repository-safe authored GLB with an explicit PBR material. Neither path is runtime acceptance by itself.
for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'attempt("HMMWV"',
    'attempt("M2"',
    "def import_btr4(",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual",
    "M_BTR4_OC_Authored",
    'IMPORT_CONTRACT_REVISION = "PASS45_MATERIAL_CLOSURE_20260826_R1"',
    "other independent assets will continue",
    "CONTENT_GAP=",
):
    require(vehicle_import, needle, "current production vehicle source truth")
forbid(vehicle_import, 'attempt("BTR4"', "obsolete BTR source-missing attempt path")
forbid(vehicle_import, "ensure_sources_exist()", "obsolete all-or-nothing production source gate")

# The command wrapper still exposes independent model outcomes. BTR is expected to be importable through the
# repository-safe authored fallback even if the local FBX is absent.
for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    "Continuing independent intake for any available source files",
):
    require(vehicle_cmd, needle, "independent production vehicle command truth")

# Normal intake freshness is revision-based, not file-existence based.
for needle in (
    "PASS45_MATERIAL_CLOSURE_20260826_R1",
    "production_import_success.txt",
    "production_fresh_load_success.txt",
    "Existing .uasset files are not sufficient proof of current materials.",
    "FRESH_LOADED=/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
):
    require(vehicle_try, needle, "current production freshness gate")

for needle in (
    "AUTHORED_MATERIALS_READY",
    "placeholder_slots",
    "basicshapematerial",
    "defaultmaterial",
    "_defaultmat",
    "PASS45_MATERIAL_CLOSURE_20260826_R1",
    "M_BTR4_OC_Authored",
    "SOURCE_KIND=BTR4:",
):
    require(vehicle_fresh, needle, "fresh-load authored material truth")

# Standalone asset-intake helpers retain their narrower truth semantics. The M2 helper requires downloaded source;
# the dedicated BTR user-source helper proves only the local-user-FBX route, not the canonical fallback route.
require(m2_launcher, "source_kind=downloaded", "real M2 source requirement")
require(btr_launcher, "source_kind=local_user_fbx", "dedicated local BTR4 source helper")

print("CONTENT READINESS PASS 19 + PASS45 MATERIAL INTAKE CONTRACT PASS")
print("- generic weapon fallback meshes do not impersonate production art")
print("- Pass 7 remains strict exact-production certification")
print("- Pass 19 separately proves an 11-class playable real-mesh rack")
print("- HMMWV/M2 remain independent external-source imports")
print("- BTR canonical intake prefers local FBX and otherwise uses the repository-safe authored GLB material path")
print("- imported vehicle meshes must reopen under the current revision with authored non-placeholder materials")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and rendered asset intake remain required")
