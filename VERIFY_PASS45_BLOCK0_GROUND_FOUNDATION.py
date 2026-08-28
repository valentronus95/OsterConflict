from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCBlock0GroundFoundationSubsystem.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCBlock0GroundFoundationSubsystem.cpp"
FOLIAGE_HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCFoliageRuntimeGuardSubsystem.h"
FOLIAGE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp"
LATE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAuthoredWorldSurfaceUpgradeSubsystem.cpp"
ACCEPTANCE = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
PLAN = ROOT / "PASS45_BLOCK_EXECUTION_PLAN.md"
RETIRED_COVERAGE_HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCBlock0FoliageCoverageValidationSubsystem.h"
RETIRED_COVERAGE_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCBlock0FoliageCoverageValidationSubsystem.cpp"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.exists():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        errors.append(f"missing {label}: {needle}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        errors.append(f"forbidden {label}: {needle}")


header = read(HEADER)
cpp = read(CPP)
foliage_header = read(FOLIAGE_HEADER)
foliage_cpp = read(FOLIAGE_CPP)
late_cpp = read(LATE_CPP)
acceptance = read(ACCEPTANCE)
plan = read(PLAN)

for needle in (
    "class OSTERCONFLICT_API UOCBlock0GroundFoundationSubsystem : public UWorldSubsystem",
    "virtual void OnWorldBeginPlay(UWorld& InWorld) override;",
):
    require(header, needle, "Block0 pre-tick subsystem contract")

for needle in (
    "/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1",
    "/Game/AdvancedVillagePack/Materials/M_Inst_Landscape.M_Inst_Landscape",
    "ApplyAuthoredGroundBeforeFirstTick",
    "/Engine/BasicShapes/Cube",
    "EmptyOverrideMaterials()",
    "SetMaterial(0, AuthoredMaterial)",
    "DesiredSizeCm.X / NewNativeSize.X",
    "DesiredSizeCm.Y / NewNativeSize.Y",
    "OldTopZ",
    "NewTopOffsetZ",
    "PASS45_BLOCK0_PRETICK_GROUND_READY",
    "authored_before_first_tick=1",
    "delayed_ground_mutation_required=0",
    "runtime_acceptance=0",
):
    require(cpp, needle, "Block0 authored ground pre-tick implementation")

for needle in (
    "PASS45_BLOCK0_PRETICK_GROUND_FAIL",
    "PASS45_BLOCK0_PRETICK_GROUND_CONTENT_GAP",
):
    require(cpp, needle, "Block0 fail-visible ground evidence")

for forbidden_new_owner_term in (
    "Tick(float",
    "FTimerHandle",
    "SetTimer(",
):
    forbid(cpp, forbidden_new_owner_term, "delayed/timer-based Block0 ground ownership")

# The historical world-surface upgrader may continue owning roads/sidewalks/fences, but Ground must be
# idempotent when the pre-tick owner has already installed the exact authored mesh/material.
for needle in (
    "if (CurrentMesh == AuthoredMesh)",
    "if (Component->GetMaterial(0) != AuthoredMaterial)",
    "ground_authored_material_contract_drift",
):
    require(late_cpp, needle, "late world-surface Ground idempotence")

# Spatial grass coverage belongs to the existing strict foliage runtime guard. A second tick subsystem would
# duplicate a full HISM scan and could disagree with the PASS10/PASS36 gate consumed by runtime acceptance.
for retired in (RETIRED_COVERAGE_HEADER, RETIRED_COVERAGE_CPP):
    if retired.exists():
        errors.append(f"duplicate Block0 foliage coverage tick owner still exists: {retired.relative_to(ROOT)}")

for needle in (
    "ValidateDenseFoliage(",
    "int32& OutOccupiedBins",
    "int32 OutQuadrantOccupied[4]",
    "bool& bOutEdgeReach",
):
    require(foliage_header, needle, "single-owner spatial foliage guard contract")

for needle in (
    "CompactMinX = -78000.0f",
    "CompactMaxX =  18000.0f",
    "CompactMinY = -12000.0f",
    "CompactMaxY =  82000.0f",
    "CoverageBinsPerAxis = 4",
    "MinOccupiedBins = 12",
    "MinOccupiedBinsPerQuadrant = 2",
    "EdgeToleranceFraction = 0.20f",
    "OC_Block0FullMapGrassComplete",
    "GetInstanceTransform(Index, InstanceTransform, true)",
    "OutOccupiedBins >= MinOccupiedBins",
    "bOutEdgeReach",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY",
    "full_playable_distribution=1",
    "strict_runtime_owner=OCFoliageRuntimeGuard",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "spatial_coverage=1",
    "block0_spatial_grass_distribution_insufficient",
):
    require(foliage_cpp, needle, "Block0 full-map grass distribution strict gate")

# Runtime acceptance already consumes PASS36 READY and rejects PASS10 FAIL. Because both are emitted only after
# the integrated spatial test, Block0 distribution can no longer fail silently behind an unrelated marker.
for needle in (
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    'findstr /C:"PASS10_FOLIAGE_RUNTIME_FAIL"',
):
    require(acceptance, needle, "Block0 runtime acceptance wiring")

for needle in (
    "| 0 | Ground + grass foundation | **ACTIVE** |",
    "Block 0 cannot close from CI alone.",
):
    require(plan, needle, "canonical Block0 execution authority")

if errors:
    print("PASS45 BLOCK0 GROUND + SPATIAL GRASS FOUNDATION: FAIL")
    for error in errors:
        print(f"- {error}")
    raise SystemExit(1)

print("PASS45 BLOCK0 GROUND + SPATIAL GRASS FOUNDATION: PASS")
print("- tracked authored ground mesh/material is applied in UWorld::OnWorldBeginPlay")
print("- compact source footprint and top-Z are preserved with bounds-aware conversion")
print("- the new Ground owner contains no Tick/timer delay")
print("- later world-surface Ground handling remains idempotent when authored state already exists")
print("- the existing foliage runtime guard is the single strict owner for 4x4 spatial grass distribution")
print("- PASS36 READY cannot emit until bin/quadrant/edge coverage passes; spatial failure also emits PASS10 hard FAIL")
print("- duplicate Block0 coverage tick subsystem is physically absent")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile, first-frame visual evidence, spatial coverage log and Block0 screenshots remain authoritative")
