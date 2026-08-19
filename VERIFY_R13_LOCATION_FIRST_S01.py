from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
DOC = ROOT / "OsterConflict" / "Docs" / "OSTER_SECTOR_S01_KRUSHELNYTSKA_COLLEGE_PARK.md"
PLAN_H = SRC / "Public" / "OCLocationSectorPlan.h"
PLAN_CPP = SRC / "Private" / "OCLocationSectorPlan.cpp"
DATA_H = SRC / "Public" / "OCLocationSectorS01Data.h"
DATA_CPP = SRC / "Private" / "OCLocationSectorS01Data.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"
INFILL = SRC / "Private" / "OCR13ResidentialInfillSubsystem.cpp"
DRESSING = SRC / "Private" / "OCR13EnvironmentDressingSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 VERIFY FAIL: " + message)


for path in (DOC, PLAN_H, PLAN_CPP, DATA_H, DATA_CPP, WORLD, INFILL, DRESSING):
    if not path.is_file():
        fail(f"missing file: {path.relative_to(ROOT)}")

plan_h = PLAN_H.read_text(encoding="utf-8", errors="replace")
plan_cpp = PLAN_CPP.read_text(encoding="utf-8", errors="replace")
data_h = DATA_H.read_text(encoding="utf-8", errors="replace")
data_cpp = DATA_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")
infill = INFILL.read_text(encoding="utf-8", errors="replace")
dressing = DRESSING.read_text(encoding="utf-8", errors="replace")
doc = DOC.read_text(encoding="utf-8", errors="replace")

for token in [
    "KrushelnytskaCollegePark()",
    "IsInsideKrushelnytskaCollegePark",
    'TEXT("S01_Krushelnytska_College_Park")',
    "FOCGeoReference::College()",
    "FOCGeoReference::CentralPark()",
]:
    if token not in plan_h + plan_cpp:
        fail(f"sector-plan contract missing: {token}")

for token in [
    "FOCS01ResidentialPlotSeed", "FOCS01FrontageSeed", "FOCS01RoadSeed",
    "ProvisionalResidentialPlots()", "ProvisionalFrontages()", "ProvisionalServiceRoads()",
    "EOCReferenceConfidence::C", "VisualVariant", "bHasPrimaryHouse",
    "bHasOutbuilding", "bOutbuildingHasRoof",
]:
    if token not in data_h + data_cpp:
        fail(f"S01 data-registry contract missing: {token}")

plot_ids = re.findall(r'TEXT\("(S01_KR_[WE]_\d\d)"\)', data_cpp)
if len(plot_ids) != 16 or len(set(plot_ids)) != 16:
    fail(f"expected 16 unique individually addressable migrated plots, found {len(plot_ids)}")
for side in ("W", "E"):
    expected = {f"S01_KR_{side}_{i:02d}" for i in range(1, 9)}
    actual = {value for value in plot_ids if f"_{side}_" in value}
    if actual != expected:
        fail(f"{side}-side plot IDs do not cover 01..08")

frontage_ids = re.findall(r'TEXT\("(S01_KR_FRONT_\d\d)"\)', data_cpp)
if frontage_ids != [f"S01_KR_FRONT_{i:02d}" for i in range(1, 9)]:
    fail(f"expected ordered frontage IDs 01..08, found {frontage_ids}")

service_ids = re.findall(r'TEXT\("(S01_KR_SERVICE_[WE])"\)', data_cpp)
if set(service_ids) != {"S01_KR_SERVICE_W", "S01_KR_SERVICE_E"} or len(service_ids) != 2:
    fail(f"expected two explicit S01 service roads, found {service_ids}")

# East slot 03 existed only as a plot/outbuilding in the old blockout. Preserve that fact explicitly.
e03 = re.search(
    r'TEXT\("S01_KR_E_03"\).*?-88\.0f,\s*2,\s*false,\s*\n\s*FVector\(-26200,\s*31850,\s*140\).*?-88\.0f,\s*true,\s*false,',
    data_cpp,
    flags=re.S,
)
if not e03:
    fail("legacy east-side slot 03 primary-house absence/outbuilding state was not preserved explicitly")

# Topology records are counted separately from the later vegetation registry.
vegetation_start = data_cpp.find("ProvisionalVegetationTrees()")
if vegetation_start < 0:
    fail("S01 vegetation registry boundary is missing")
topology_section = data_cpp[:vegetation_start]
if topology_section.count("Provisional,") != 26:
    fail("all 26 migrated topology records must remain explicitly provisional C-confidence")

# The authoritative world builder must consume explicit topology records, not regenerate coordinates from slot arithmetic.
for token in [
    '#include "OCLocationSectorS01Data.h"',
    "FOCLocationSectorS01Data::ProvisionalResidentialPlots()",
    "Plot.HouseCenter", "Plot.HouseSizeCm", "Plot.VisualVariant",
    "Plot.OutbuildingCenter", "Plot.bOutbuildingHasRoof",
    "FOCLocationSectorS01Data::ProvisionalFrontages()",
    "Frontage.WestFenceCenter", "Frontage.EastFenceCenter",
    "Frontage.WestWalkCenter", "Frontage.EastWalkCenter",
    "FOCLocationSectorS01Data::ProvisionalServiceRoads()",
    "AddBox(Roads, Road.Center, Road.SizeCm, Road.Yaw)",
]:
    if token not in world:
        fail(f"world builder does not consume explicit S01 topology data: {token}")
for forbidden in [
    "WestHouseX", "EastHouseX", "const float StartY = 20500.0f",
    "BoundaryStartY", "static_cast<float>(Slot) * 4800.0f",
    "FVector(-43000.0f, 36000.0f, RoadZ)", "FVector(-24200.0f, 37000.0f, RoadZ)",
]:
    if forbidden in world:
        fail(f"legacy arithmetic/direct S01 topology placement survived registry migration: {forbidden}")

# Location-first guardrails: no city-wide generator is allowed to fill or dress S01.
if "procedural residential infill disabled" not in infill:
    fail("procedural residential infill is not disabled")
if "FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Block.Origin)" not in world:
    fail("generic source residential blocks are not excluded from S01")
for token in [
    '#include "OCLocationSectorPlan.h"',
    "IsProtectedFromGenericDressing",
    "FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Location)",
]:
    if token not in dressing:
        fail(f"generic environment dressing is not guarded from S01: {token}")

# Generic source vegetation points must be rejected before they are rendered inside S01.
for token in [
    "IsInsideKrushelnytskaCollegePark(RoughPatches[I])",
    "IsInsideKrushelnytskaCollegePark(TreeLocation)",
]:
    if token not in world:
        fail(f"generic source vegetation is not excluded from S01: {token}")

for token in [
    "ACTIVE RECONSTRUCTION SECTOR",
    "confidence C",
    "S01.1 — topology lock",
    "S01.2 — plot registry",
    "S01.5 — vegetation",
    "S01 LOCKED",
]:
    if token not in doc:
        fail(f"S01 execution document missing: {token}")

print("R13 LOCATION-FIRST S01 VERIFY: PASS")
print("Checks canonical sector ownership, 26 explicit C-confidence topology records, direct topology consumption and generic infill/dressing/vegetation exclusion.")
