#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 PARK SEMANTIC SOURCE VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 PARK SEMANTIC SOURCE VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 PARK SEMANTIC SOURCE VERIFY FAIL: {label}: forbidden {needle!r}")


header = read(SRC / "Public" / "OCParkSemanticAuthoredUpgradeSubsystem.h")
impl = read(SRC / "Private" / "OCParkSemanticAuthoredUpgradeSubsystem.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")

bench_asset = (
    ROOT
    / "OsterConflict"
    / "Content"
    / "Mega_Street_Props_Pack"
    / "Street_Props_pack_V2"
    / "Meshes"
    / "SM_Bench_1.uasset"
)
if not bench_asset.is_file():
    raise SystemExit(
        "PASS45 PARK SEMANTIC SOURCE VERIFY FAIL: tracked SM_Bench_1 asset is missing"
    )

for needle in (
    "UOCParkSemanticAuthoredUpgradeSubsystem",
    "homogeneous semantic families",
    "must not blanket-remap",
):
    require(header, needle, "semantic upgrade ownership")

for needle in (
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bench_1.SM_Bench_1",
    "ExpectedBenchInstances = 14",
    "DesiredBenchLengthCm = 180.0f",
    "IsEngineCube",
    "Component->GetInstanceCount() != ExpectedBenchInstances",
    "Component->SetStaticMesh(AuthoredMesh)",
    "Component->EmptyOverrideMaterials()",
    "const float UniformScale",
    "native proportions",
    "SourceBottomZ",
    "NewLocation.Z = SourceBottomZ - NewBottomOffsetZ",
    "ElapsedSeconds < 0.75f",
    "PASS45_AUTHORED_PARK_SEMANTIC_CONTENT_GAP",
    "PASS45_AUTHORED_PARK_SEMANTIC_FAIL",
    "PASS45_AUTHORED_PARK_BENCHES_READY",
    "bench_instances=14",
    "semantic_owner=ParkBenches",
    "basicshape_meshes=0",
    "basicshape_material_overrides=0",
    "uniform_scale=1",
    "native_proportions_preserved=1",
    "ground_bottom_preserved=1",
    "bounds_aware_upgrade=1",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(impl, needle, "ParkBenches authored replacement")

forbid(impl, 'FindISM(Sector, TEXT("ParkDetails"))', "legacy mixed ParkDetails mutation")
forbid(impl, 'FindISM(Sector, TEXT("ParkMemorialPlaza"))', "unverified memorial blanket mutation")
forbid(impl, 'FindISM(Sector, TEXT("ParkMemorialApproach"))', "unverified memorial-step blanket mutation")
forbid(impl, 'FindISM(Sector, TEXT("ParkSkateFitness"))', "unverified skate blanket mutation")

park_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
park_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", park_begin)
if park_begin < 0 or park_end < 0:
    raise SystemExit("PASS45 PARK SEMANTIC SOURCE VERIFY FAIL: cannot isolate BuildCentralPark")
park_source = world[park_begin:park_end]
for needle in (
    "ExpectedBenches = 14",
    "static_assert(ExpectedSemanticDetails == 23",
    'ParkBenches = MakeISM(TEXT("ParkBenches")',
    "AddBox(ParkBenches, Park + FVector(I * 1900.0f, -850.0f, 60.0f), FVector(180, 55, 120))",
    "AddBox(ParkBenches, Park + FVector(I * 1900.0f, 850.0f, 60.0f), FVector(180, 55, 120))",
    "LegacyCount == 0",
    "BenchCount == ExpectedBenches",
    "PASS45_GATE_K_PARK_SEMANTIC_SPLIT_READY",
):
    require(world, needle, "canonical ParkBenches source contract")
forbid(park_source, "AddBox(ParkDetails,", "legacy ParkDetails resurrection")

# Runtime Gate K remains observation-only. This slice removes one semantic BasicShape family but must not
# promote Gate K until every remaining visible BasicShape family and direct UE 5.8 visual evidence are clean.
for needle in (
    "CountVisibleBasicShapes",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "PASS45_GATE_K_RUNTIME_READY",
    "visible_basicshape_components=0",
    "gate_k_complete=1",
):
    require(gate, needle, "Gate K final-world observation")
forbid(gate, "SetVisibility(false", "Gate K mutation")

print("PASS45 PARK SEMANTIC AUTHORED SOURCE PASS")
print("- ParkBenches remains a distinct semantic owner with exactly 14 canonical source proxies")
print("- tracked SM_Bench_1 replaces only ParkBenches before Gate K / Pass12")
print("- authored bench native proportions and source ground contact are preserved")
print("- ParkDetails, memorial and skate/fitness groups are not blanket-remapped")
print("- Gate K remains incomplete and runtime visual acceptance remains pending")
