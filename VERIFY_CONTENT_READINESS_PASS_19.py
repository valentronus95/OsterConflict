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
start_here = read(ROOT / "START_HERE.cmd")
runtime_evidence = read(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")
vehicle_import = read(ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py")
vehicle_cmd = read(ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd")
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

# The old RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd was retired. START_HERE + the canonical Python
# evidence verifier now own the runtime acceptance path, and Pass19 readiness remains an explicit gate.
for needle in (
    'set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"',
    "call :full_runtime_test",
    '%PY_CMD% "%EVIDENCE_VERIFY%"',
):
    require(start_here, needle, "canonical runtime launcher route")
for needle in ("PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL"):
    require(runtime_evidence, needle, "canonical Pass19 runtime evidence")
forbid(start_here, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "retired Pass15 acceptance launcher")

# Current production intake keeps HMMWV/M2 independent and gives BTR-4 two factual paths: local user FBX
# when present, otherwise the Oster-authored generated fallback. A failure in one family cannot hide another.
for needle in (
    "ukrainian_hmmwv_mk_19.glb",
    "m2_50cal_machinegun_cc0.glb",
    "BTR4_Bucephalus.fbx",
    'attempt("HMMWV"',
    'attempt("M2"',
    "if BTR_SOURCE.exists():",
    "build_btr4_glb(authored_btr)",
    "BTR4 local FBX missing; generated and imported Oster-authored fallback",
    "BTR4_IMPORT_FAILED=",
    "other independent assets will continue",
    "CONTENT_GAP=",
):
    require(vehicle_import, needle, "independent production vehicle source truth")
forbid(vehicle_import, "ensure_sources_exist()", "obsolete all-or-nothing production source gate")

for needle in (
    'set "HMMWV_IMPORTED=0"',
    'set "M2_IMPORTED=0"',
    'set "BTR_IMPORTED=0"',
    "Continuing independent intake for available source files",
    "CONTENT GAP: BTR-4 production source/import is still unavailable",
):
    require(vehicle_cmd, needle, "independent production vehicle command truth")

for needle in (
    "AUTHORED_MATERIALS_READY",
    "placeholder_slots",
    "basicshapematerial",
    "defaultmaterial",
):
    require(vehicle_fresh, needle, "fresh-load authored material truth")

require(m2_launcher, "source_kind=downloaded", "real M2 source requirement")
require(btr_launcher, "source_kind=local_user_fbx", "exact local BTR4 source requirement")

print("CONTENT READINESS PASS 19 + PASS 44 INDEPENDENT INTAKE CONTRACT PASS")
print("- generic weapon fallback meshes do not impersonate production art")
print("- Pass 7 remains strict exact-production certification")
print("- Pass 19 separately proves an 11-class playable real-mesh rack")
print("- Pass19 runtime acceptance is carried by START_HERE + canonical Pass45 evidence, not a deleted CMD")
print("- HMMWV/M2 remain independent real-source imports; BTR4 may use local FBX or the authored generated fallback")
print("- imported vehicle meshes must reopen with authored materials, not Default/BasicShape placeholders")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and exact asset intake remain required")
