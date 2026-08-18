from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13MapPlacementRepairSubsystem.h"
CPP = SRC / "Private" / "OCR13MapPlacementRepairSubsystem.cpp"
GROUND = SRC / "Private" / "OCR13GroundSurfaceSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 MAP PLACEMENT REPAIR VERIFY FAIL: " + message)


for path in (H, CPP, GROUND):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
ground = GROUND.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final placement-repair header include")

for token in [
    "BenchRepairDelaySeconds = 0.95f",
    "FinalPlacementRepairDelaySeconds = 2.85f",
    "RepairCentralParkBenchOrientation",
    "RepairGeneratedVegetationGrounding",
    "SuppressMisalignedPoleAttachments",
    'FindISM(Sector, TEXT("ParkDetails"))',
    "IsCentralParkBenchProxy",
    "Delta.Y >= 0.0f ? 0.0f : 180.0f",
    "Bounds.Origin.Z - Bounds.BoxExtent.Z",
    "GroundedZ = -LocalBottom * Scale.Z",
    "UpdateInstanceTransform",
    'Value.StartsWith(TEXT("R13_Tree"))',
    'Value.StartsWith(TEXT("R13_CompanionTree"))',
    'Value.StartsWith(TEXT("R13_ExplicitPine"))',
    'Value.StartsWith(TEXT("R13_Shrub"))',
    'Value.StartsWith(TEXT("R13_WetlandReed"))',
    'Name == TEXT("R13_UtilityPoleAddons")',
    'Name == TEXT("R13_UtilityPoleLights")',
    'Name == TEXT("R13_KrushelnytskaPoleAddons")',
    'Name == TEXT("R13_KrushelnytskaPoleLights")',
    "SetHiddenInGame(true, true)",
    "GameMode->IsFrontendOnlySession()",
]:
    if token not in cpp:
        fail(f"placement repair marker missing: {token}")

for token in [
    "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
    "R13_MatteOsterGround",
    "matte non-water city floor",
    "Ground->SetMaterial(0, GroundMaterial)",
]:
    if token not in ground:
        fail(f"matte ground marker missing: {token}")
if "Diorama_Ground.Diorama_Ground" in ground:
    fail("wet-looking diorama material returned to the broad authoritative ground")

for forbidden in [
    "World.SpawnActor",
    "SetActorLocation",
    "SetCollisionEnabled",
]:
    if forbidden in cpp:
        fail(f"placement repair must not generate/move gameplay actors or mutate collision: {forbidden}")

print("R13.6 MAP PLACEMENT REPAIR VERIFY: PASS")
print("Checks matte dry city floor, main-alley-facing park benches, bounds-grounded generic vegetation and suppression of unverified pole attachments without gameplay-geometry mutation.")
