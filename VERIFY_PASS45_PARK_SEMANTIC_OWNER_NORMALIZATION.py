#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 PARK SEMANTIC OWNER NORMALIZATION VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(
            f"PASS45 PARK SEMANTIC OWNER NORMALIZATION VERIFY FAIL: {label}: missing {needle!r}"
        )


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(
            f"PASS45 PARK SEMANTIC OWNER NORMALIZATION VERIFY FAIL: {label}: forbidden {needle!r}"
        )


header = read(SRC / "Public" / "OCParkSemanticOwnerNormalizationSubsystem.h")
impl = read(SRC / "Private" / "OCParkSemanticOwnerNormalizationSubsystem.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
gate = read(SRC / "Private" / "OCVisualFidelityGateKSubsystem.cpp")

for needle in (
    "UOCParkSemanticOwnerNormalizationSubsystem",
    "fail-closed, geometry-preserving re-home",
    "primary_authoring=0",
    "must be retired when AOCWorldSectorOster directly authors",
    "may never hide rejected geometry",
):
    require(header, needle, "migration-bridge ownership")

for needle in (
    "ExpectedLegacyMemorialInstances = 2",
    "ExpectedLegacySkateInstances = 3",
    "NormalizationDelaySeconds = 0.35f",
    'FindISM(Sector, TEXT("ParkMemorialPlaza"))',
    'FindISM(Sector, TEXT("ParkSkateFitness"))',
    "IsEngineCube",
    'TEXT("ParkMemorialSurface")',
    'TEXT("ParkMemorialMonument")',
    'TEXT("ParkSkateSurface")',
    'TEXT("ParkSkateRamps")',
    "MemorialTransforms[0]",
    "MemorialTransforms[1]",
    "SkateTransforms[0]",
    "SkateTransforms[1]",
    "SkateTransforms[2]",
    "source_instances_preserved=1",
    "LegacyMemorial->ClearInstances()",
    "LegacySkate->ClearInstances()",
    "LegacyMemorial->GetInstanceCount() == 0",
    "LegacySkate->GetInstanceCount() == 0",
    "MemorialSurface->GetInstanceCount() == 1",
    "MemorialMonument->GetInstanceCount() == 1",
    "SkateSurface->GetInstanceCount() == 1",
    "SkateRamps->GetInstanceCount() == 2",
    "PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_FAIL",
    "PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION_READY",
    "source_instance_total=5",
    "destination_instance_total=5",
    "geometry_preserved=1",
    "park_details_mutation=0",
    "memorial_approach_mutation=0",
    "benches_mutation=0",
    "primary_authoring=0",
    "migration_bridge=1",
    "gate_k_complete=0",
    "runtime_acceptance=0",
):
    require(impl, needle, "exact semantic normalization")

forbid(impl, 'FindISM(Sector, TEXT("ParkDetails"))', "ParkDetails mutation")
forbid(impl, 'FindISM(Sector, TEXT("ParkMemorialApproach"))', "authored memorial-approach mutation")
forbid(impl, 'FindISM(Sector, TEXT("ParkBenches"))', "authored bench mutation")
forbid(impl, "SetVisibility(false", "hidden-geometry false pass")
forbid(impl, "SetHiddenInGame(true", "hidden-geometry false pass")

park_begin = world.find("void AOCWorldSectorOster::BuildCentralPark()")
park_end = world.find("\nvoid AOCWorldSectorOster::BuildCollegeSector()", park_begin)
if park_begin < 0 or park_end < 0:
    raise SystemExit(
        "PASS45 PARK SEMANTIC OWNER NORMALIZATION VERIFY FAIL: cannot isolate BuildCentralPark"
    )
park_source = world[park_begin:park_end]

# The bridge is indexed to the exact current mixed source contract. If primary authoring changes, this verifier must
# fail until the bridge is deliberately retired or updated rather than silently reclassifying arbitrary instances.
for needle in (
    "ExpectedMemorialPlaza = 2",
    "ExpectedSkateFitness = 3",
    'AddBox(ParkMemorialPlaza, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56))',
    'AddBox(ParkMemorialPlaza, Park + FVector(-600, 200, 230), FVector(260, 260, 400))',
    'AddBox(ParkSkateFitness, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36))',
    'AddBoxRotated(ParkSkateFitness, Park + FVector(6100, -4100, 120), FVector(1200, 600, 35)',
    'AddBoxRotated(ParkSkateFitness, Park + FVector(7400, -3500, 95), FVector(950, 500, 30)',
    "static_assert(ExpectedSemanticDetails == 23",
):
    require(park_source, needle, "indexed legacy mixed-source contract")

# Gate K waits until after this 0.35 s bridge and ignores zero-instance quarantine ISMs, but remains observation-only.
for needle in (
    "ElapsedSeconds < 3.0f",
    "if (ISM->GetInstanceCount() <= 0) continue;",
    "CountVisibleBasicShapes",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "visible_basicshape_components=0",
):
    require(gate, needle, "Gate K ordering/observation")
forbid(gate, "SetVisibility(false", "Gate K mutation")

print("PASS45 PARK SEMANTIC OWNER NORMALIZATION SOURCE PASS")
print("- mixed memorial 2-instance family is re-homed exactly as surface=1 + monument=1")
print("- mixed skate 3-instance family is re-homed exactly as surface=1 + ramps=2")
print("- all five source transforms are preserved before legacy buckets are cleared")
print("- ParkDetails, ParkMemorialApproach and ParkBenches remain outside this migration owner")
print("- bridge is explicitly temporary primary_authoring=0 and cannot close Gate K/runtime acceptance")
