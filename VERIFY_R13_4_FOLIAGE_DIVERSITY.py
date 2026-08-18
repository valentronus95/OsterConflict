from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCR13FoliageDiversitySubsystem.h"
CPP = SRC / "Private" / "OCR13FoliageDiversitySubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 FOLIAGE DIVERSITY VERIFY FAIL: " + message)


for path in (HEADER, CPP):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

for token in [
    "public UWorldSubsystem",
    "virtual void OnWorldBeginPlay(UWorld& InWorld) override;",
    "void ApplyFoliageDiversity(UWorld& World);",
    "bool bApplied = false;",
]:
    if token not in header:
        fail(f"subsystem declaration missing: {token}")

includes = [line.strip() for line in header.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must be last include")

for token in [
    "FoliageDiversityDelaySeconds = 1.75f",
    "GameMode->IsFrontendOnlySession()",
    "OsterConflict_Runtime",
    "CreateWeakLambda",
    "MaxWetlandReedsPerZone = 180",
]:
    if token not in cpp:
        fail(f"runtime safety marker missing: {token}")

for index in range(1, 6):
    token = f"SM_Pine_Tree_{index:02d}.SM_Pine_Tree_{index:02d}"
    if token not in cpp:
        fail(f"explicit pine asset missing: {token}")

for token in [
    "Shrubs_1.Shrubs_1",
    "Shrubs_1_Single.Shrubs_1_Single",
    "Bush_1.Bush_1",
    "Cat_Tail.Cat_Tail",
    "Cat_Tail_2.Cat_Tail_2",
    'FindISM(Sector, TEXT("PineTrunks"))',
    'FindISM(Sector, TEXT("TreeTrunks"))',
    'FindISM(Sector, TEXT("SovietPoplarTrunks"))',
    'FindISM(Sector, TEXT("BirchTrunks"))',
    'FindISM(Sector, TEXT("GrassWetland"))',
]:
    if token not in cpp:
        fail(f"foliage placement marker missing: {token}")

for token in [
    "ECollisionEnabled::NoCollision",
    "SetCanEverAffectNavigation(false)",
    "FoliageRoot->SetActorEnableCollision(false)",
    "R13 foliage diversity: companion pines=%d shrubs=%d wetland reeds=%d.",
]:
    if token not in cpp:
        fail(f"visual-only foliage contract missing: {token}")

print("R13.4 FOLIAGE DIVERSITY VERIFY: PASS")
print("Checks explicit pine assets, restrained shrubs/wetland reeds, delayed ownership and zero gameplay collision/navigation impact.")
