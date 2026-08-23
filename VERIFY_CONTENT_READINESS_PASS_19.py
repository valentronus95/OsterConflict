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

# Focused Museum/FPS recovery must not claim exact production readiness.
require(launcher, "PASS19_PLAYABLE_WEAPON_SET_READY", "focused launcher playable gate")
require(launcher, "PASS19_PLAYABLE_WEAPON_SET_FAIL", "focused launcher failure gate")
forbid(launcher, "PASS7_PRODUCTION_WEAPONS_READY", "focused launcher exact-art false certification")

# Production fleet source intake is intentionally strict and requires real local source binaries.
for needle in (
    "ukrainian_hmmwv_mk_19.glb", "m2_50cal_machinegun_cc0.glb", "BTR4_Bucephalus.fbx",
    "ensure_sources_exist()",
):
    require(vehicle_import, needle, "production vehicle source truth")
require(m2_launcher, "source_kind=downloaded", "real M2 source requirement")
require(btr_launcher, "source_kind=local_user_fbx", "real BTR4 source requirement")

print("CONTENT READINESS PASS 19 SOURCE CONTRACT PASS")
print("- generic M249/Remington/etc fallback meshes no longer impersonate production art")
print("- Pass 7 remains strict exact-production certification")
print("- Pass 19 separately proves an 11-class playable real-mesh rack")
print("- HMMWV/M2/BTR4 production intake still requires real source binaries")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime and exact asset intake remain required")
