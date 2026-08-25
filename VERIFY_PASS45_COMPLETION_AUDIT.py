#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
CONTENT = ROOT / "OsterConflict" / "Content"
REPORT = ROOT / "OsterConflict" / "Docs" / "WorkReports" / "PASS45_COMPLETION_AUDIT_2026-08-25.md"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(cond: bool, message: str) -> None:
    if not cond:
        errors.append(message)


impl = read(SRC / "Private" / "OCWorldProductionVisualsSubsystem.cpp")
header = read(SRC / "Public" / "OCWorldProductionVisualsSubsystem.h")
report = read(REPORT)
tz = read(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")
ledger = read(ROOT / "OSTER_CONFLICT_WORK_LEDGER.md")

required_assets = [
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_House_Var01.uasset",
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_House_Var02.uasset",
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_Fence_Var01.uasset",
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_Fence_Var02.uasset",
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_Fence_Var03.uasset",
    CONTENT / "AdvancedVillagePack" / "Meshes" / "SM_Fence_Var04.uasset",
    CONTENT / "AdvancedVillagePack" / "Materials" / "M_Inst_Landscape.uasset",
    CONTENT / "Scene_RoadsideConstruction" / "Materials" / "MaterialInstances" / "MI_Urb_Roa_Asphalt_01.uasset",
    CONTENT / "Scene_RoadsideConstruction" / "Materials" / "MaterialInstances" / "MI_Urb_Roa_Sidewalk_01.uasset",
]
for path in required_assets:
    req(path.is_file(), f"verified imported B2 dependency is missing: {path.relative_to(ROOT)}")

for needle in (
    "single generic environment visual-conversion owner",
    "no full-world repeated scan",
    "polling_after_ready=0",
):
    req(needle in report, f"completion audit report missing ownership/budget statement: {needle}")

for needle in (
    "UOCWorldProductionVisualsSubsystem",
    "Pass 45 B2 visual owner",
    "TryBuildProductionVisuals",
):
    req(needle in header, f"B2 subsystem header missing: {needle}")

for path_token in (
    "SM_House_Var01",
    "SM_House_Var02",
    "SM_Fence_Var01",
    "SM_Fence_Var02",
    "SM_Fence_Var03",
    "SM_Fence_Var04",
    "M_Inst_Landscape",
    "MI_Urb_Roa_Asphalt_01",
    "MI_Urb_Roa_Sidewalk_01",
):
    req(path_token in impl, f"B2 production owner does not use verified imported dependency: {path_token}")

for needle in (
    'FindISM(Sector, TEXT("Buildings"))',
    'FindISM(Sector, TEXT("ResidentialRoofs"))',
    'FindISM(Sector, TEXT("ResidentialDetails"))',
    'FindISM(Sector, TEXT("Fences"))',
    'FindISM(Sector, TEXT("WoodFences"))',
    'FindISM(Sector, TEXT("MetalFences"))',
    'FindISM(Sector, TEXT("LightSheetFences"))',
    "HideVisualKeepCollision(Buildings)",
    "HideVisualKeepCollision(Roofs)",
    "HideVisualKeepCollision(Details)",
    "ReplaceFenceFamily",
):
    req(needle in impl, f"B2 primitive visual retirement path missing: {needle}")

# Preserve authored model proportions instead of independently stretching a real house to the old cube dimensions.
req("FMath::Min(DesiredBoxCm.X / MeshSize.X, DesiredBoxCm.Y / MeshSize.Y)" in impl,
    "real house visual does not preserve authored proportions")
req("collision_backstop_retained=1" in impl,
    "B2 owner no longer documents hidden source collision/backstop ownership")
req("SetCollisionEnabled(ECollisionEnabled::NoCollision)" in impl and "SetCanEverAffectNavigation(false)" in impl,
    "production visual ISMs can duplicate collision/navigation cost")

# Performance contract for the newly introduced real mesh layer.
for needle in (
    "ProductionHousesA" , "30000, 65000, true",
    "ProductionHousesB", "ProductionFencesA", "6000, 28000, false",
    "house_cull_m=300_650", "fence_cull_m=60_280",
):
    req(needle in impl, f"B2 production visual cull budget missing: {needle}")

# The conversion must happen after the source actor's BeginPlay tint and must stop after success.
req("0.05f, false" in impl, "B2 production conversion is not delayed beyond source actor BeginPlay")
req("post_actor_beginplay=1" in impl, "B2 ready marker does not prove post-BeginPlay ordering")
req("if (bBuilt) return;" in impl and "bBuilt = true;" in impl,
    "B2 owner lacks one-shot completion latch")
req("0.10f, false" in impl, "sector retry should be one-shot/rescheduled rather than a permanent repeating timer")
req("polling_after_ready=0" in impl, "B2 ready marker does not prove no post-ready polling")

# Fail-visible content truth.
for needle in (
    "PASS45_B2_PRODUCTION_VISUALS_FAIL",
    "PASS45_B2_RESIDENTIAL_VISUAL_GAP",
    "PASS45_B2_FENCE_FAMILY_GAP",
    "PASS45_B2_PRODUCTION_VISUALS_READY",
    "PASS45_B2_REMAINING_CONTENT_GAPS",
):
    req(needle in impl, f"B2 fail-visible marker missing: {needle}")

# B2 is source-completed as an inventory/ownership decision, not falsely runtime-verified.
req("World proxy truth" in tz, "Pass45 TZ lost B2 World proxy truth section")
req("PASS45_COMPLETION_AUDIT_2026-08-25.md" in tz,
    "Pass45 TZ does not link the B2 completion audit")
req("B2" in ledger and "CODED_UNTESTED" in ledger,
    "ledger does not retain B2 as coded but runtime-unverified")

if errors:
    print("PASS45 COMPLETION AUDIT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 COMPLETION AUDIT: PASS")
print("- remaining visible BasicShape families were audited instead of silently called production-ready")
print("- imported houses/fences now own generic residential visuals; old boxes remain hidden collision backstops")
print("- ground/road/sidewalk use imported non-BasicShape materials after actor BeginPlay")
print("- new real visual ISMs have compact cull budgets and no collision/navigation duplication")
print("- College/park art and local UE acceptance remain explicit gaps rather than fabricated readiness")
print("STATUS: SOURCE COMPLETION ONLY; UE 5.8 runtime remains authoritative")
