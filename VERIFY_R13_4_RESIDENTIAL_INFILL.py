from pathlib import Path

ROOT = Path(__file__).resolve().parent
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13ResidentialInfillSubsystem.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13ResidentialInfillSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 RESIDENTIAL INFILL VERIFY FAIL: " + message)


if not PUB.is_file() or not CPP.is_file():
    fail("residential infill migration-stub header/source missing")

h = PUB.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

# Location-first R13 deliberately retired the old road-derived house generator. The class remains only so older
# maps/build references do not break while explicit Oster street/block registries become authoritative.
required = [
    "UOCR13ResidentialInfillSubsystem::ShouldCreateSubsystem",
    "WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE",
    "GameMode->IsFrontendOnlySession()",
    "BuildResidentialInfill(InWorld);",
    "if (bApplied) return;",
    "bApplied = true;",
    "procedural residential infill disabled",
    "houses must come from explicit Oster street/block placement",
    "(void)World;",
]
for token in required:
    if token not in cpp:
        fail(f"missing location-first infill migration marker: {token}")

# These markers belonged to the retired synthetic road-sampling generator. Their return would mean the project is
# inventing up to 18 Oster houses again instead of consuming explicit location data.
for forbidden in [
    "MaxInfillHouses = 18",
    "CandidateSpacingCm = 8200.0f",
    "ExistingHouseClearanceCm = 3900.0f",
    "NewHouseClearanceCm = 5600.0f",
    "AcceptedLocations",
    "HasNearbySourceBuilding",
    "HasNearbyNewBuilding",
    "RoadLengthCm < 16000.0f",
    "SM_House_Var01.SM_House_Var01",
    "SM_House_Var02.SM_House_Var02",
    "FMath::Rand",
    "FRand",
    "while (true)",
]:
    if forbidden in cpp:
        fail(f"retired procedural infill logic returned: {forbidden}")

for left, right in (("(", ")"), ("{", "}"), ("[", "]")):
    if cpp.count(left) != cpp.count(right):
        fail(f"delimiter mismatch {left}{right}")

print("R13.4 RESIDENTIAL INFILL VERIFY: PASS")
print("Checks that the historical subsystem is a safe migration stub and cannot invent road-derived houses while explicit location-first Oster topology owns residential placement.")
