from pathlib import Path

ROOT = Path(__file__).resolve().parent
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13ResidentialInfillSubsystem.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13ResidentialInfillSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 RESIDENTIAL INFILL VERIFY FAIL: " + message)


if not PUB.is_file() or not CPP.is_file():
    fail("residential infill header/source missing")

h = PUB.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "MaxInfillHouses = 18",
    "CandidateSpacingCm = 8200.0f",
    "ExistingHouseClearanceCm = 3900.0f",
    "NewHouseClearanceCm = 5600.0f",
    "IsInsideCompactBounds",
    "IsInsideKrushelnytskaReserved",
    "IsNearLandmark",
    "HasNearbySourceBuilding",
    "HasNearbyNewBuilding",
    "AddHiddenSourceFootprint",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    'FindISM(WorldSector, TEXT("Roads"))',
    'FindISM(WorldSector, TEXT("Buildings"))',
    "SM_House_Var01.SM_House_Var01",
    "SM_House_Var02.SM_House_Var02",
    'TEXT("R13_House01")',
    'TEXT("R13_House02")',
    "SetCollisionProfileName(TEXT(\"BlockAll\"))",
    "SetCanEverAffectNavigation(true)",
    "Buildings->SetVisibility(false, true)",
    "Buildings->SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "MeshSize.X * HouseScale / 100.0f",
    "MeshSize.Y * HouseScale / 100.0f",
    "GameMode->IsFrontendOnlySession()",
    "RoadLengthCm < 16000.0f",
    "RoadWidthCm > 1800.0f",
    "AcceptedLocations.Num() < MaxInfillHouses",
    "canonical house families feed EnvironmentDressing",
    "hidden source footprints reserve grass/yard space",
]
for token in required:
    if token not in cpp:
        fail(f"missing bounded-infill marker: {token}")

for forbidden in [
    "FMath::Rand",
    "FRand",
    "while (true)",
    "MaxInfillHouses = 50",
    'TEXT("R13_InfillHouse01")',
    'TEXT("R13_InfillHouse02")',
]:
    if forbidden in cpp:
        fail(f"unsafe/stale infill marker present: {forbidden}")

for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
    if cpp.count(left) != cpp.count(right):
        fail(f"delimiter mismatch {left}{right}")

print("R13.4 RESIDENTIAL INFILL VERIFY: PASS")
print("Checks deterministic road-derived placement, hard 18-house cap, compact/landmark/Krushelnytska exclusions, existing/new-house spacing, canonical EnvironmentDressing integration, hidden grass/yard footprints, gameplay collision/navigation and frontend guard.")
