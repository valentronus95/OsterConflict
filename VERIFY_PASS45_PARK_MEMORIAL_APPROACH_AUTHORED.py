#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 PARK MEMORIAL APPROACH VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 PARK MEMORIAL APPROACH VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 PARK MEMORIAL APPROACH VERIFY FAIL: {label}: forbidden {needle!r}")


header = read(SRC / "Public" / "OCParkMemorialApproachAuthoredUpgradeSubsystem.h")
impl = read(SRC / "Private" / "OCParkMemorialApproachAuthoredUpgradeSubsystem.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")

step_asset = (
    ROOT
    / "OsterConflict"
    / "Content"
    / "Mega_Street_Props_Pack"
    / "Street_Props_pack_V2"
    / "Meshes"
    / "SM_Curb_1.uasset"
)
if not step_asset.is_file():
    raise SystemExit(
        "PASS45 PARK MEMORIAL APPROACH VERIFY FAIL: tracked SM_Curb_1 asset is missing"
    )

for needle in (
    "UOCParkMemorialApproachAuthoredUpgradeSubsystem",
    "ParkMemorialApproach",
    "ParkMemorialPlaza, ParkSkateFitness, ParkBenches and legacy ParkDetails remain separate ownership domains",
):
    require(header, needle, "dedicated semantic ownership")

for needle in (
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1",
    "ExpectedMemorialApproachInstances = 4",
    "IsEngineCube",
    "Component->GetInstanceCount() != ExpectedMemorialApproachInstances",
    "Component->SetStaticMesh(AuthoredMesh)",
    "Component->EmptyOverrideMaterials()",
    "SourceSize",
    "SourceCenter",
    "SourceBottomZ",
    "NewLocation.Z = SourceBottomZ - NewBottomOffsetZ",
    "FindISM(Sector, TEXT(\"ParkMemorialApproach\"))",
    "PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_CONTENT_GAP",
    "PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_FAIL",
    "PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_READY",
    "step_mesh=SM_Curb_1",
    "instances=4",
    "semantic_owner=ParkMemorialApproach",
    "basicshape_meshes=0",
    "basicshape_material_overrides=0",
    "bounds_aware_box_fit=1",
    "source_bottom_preserved=1",
    "family_scope_exact=1",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(impl, needle, "authored memorial approach replacement")

for needle in (
    'FindISM(Sector, TEXT("ParkDetails"))',
    'FindISM(Sector, TEXT("ParkMemorialPlaza"))',
    'FindISM(Sector, TEXT("ParkSkateFitness"))',
    'FindISM(Sector, TEXT("ParkBenches"))',
):
    forbid(impl, needle, "cross-family mutation")

park_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
park_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", park_begin)
if park_begin < 0 or park_end < 0:
    raise SystemExit("PASS45 PARK MEMORIAL APPROACH VERIFY FAIL: cannot isolate BuildCentralPark")
park_source = world[park_begin:park_end]

for needle in (
    "ExpectedMemorialApproach = 4",
    'ParkMemorialApproach = MakeISM(TEXT("ParkMemorialApproach")',
    "for (int32 Step = 0; Step < ExpectedMemorialApproach; ++Step)",
    "AddBox(ParkMemorialApproach, Park + FVector(-6100 + Step * 150.0f, -4900, 18 + Step * 14.0f)",
    "MemorialApproachCount == ExpectedMemorialApproach",
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY",
):
    require(world, needle, "canonical four-step source contract")

forbid(park_source, "AddBox(ParkDetails,", "legacy ParkDetails resurrection")

# Gate K remains observation-only. One more BasicShape family is retired here, but global completion is still
# forbidden until the remaining park/world families and direct current-head UE 5.8 visual evidence are clean.
for needle in (
    "CountVisibleBasicShapes",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "PASS45_GATE_K_RUNTIME_READY",
    "visible_basicshape_components=0",
    "gate_k_complete=1",
):
    require(gate, needle, "Gate K final-world observation")
forbid(gate, "SetVisibility(false", "Gate K mutation")

print("PASS45 PARK MEMORIAL APPROACH AUTHORED SOURCE PASS")
print("- ParkMemorialApproach remains a dedicated semantic owner with exactly four canonical steps")
print("- tracked SM_Curb_1 replaces only that four-step family before Gate K / Pass12")
print("- source box footprint and bottom contact are preserved through bounds-aware authored fitting")
print("- memorial plaza, skate/fitness, benches and legacy ParkDetails are not mutated")
print("- Gate K remains incomplete and runtime visual acceptance remains pending")
