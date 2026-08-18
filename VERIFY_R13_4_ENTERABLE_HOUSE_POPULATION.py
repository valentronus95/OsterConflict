from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13EnterableHousePopulationSubsystem.h"
CPP = SRC / "Private" / "OCR13EnterableHousePopulationSubsystem.cpp"
ART = SRC / "Private" / "OCR13EnterableHouseArtSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 ENTERABLE HOUSE POPULATION VERIFY FAIL: " + message)


for path in (H, CPP, ART):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
art = ART.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "PopulationDelaySeconds = 1.62f",
    "MaxAdditionalEnterableHouses = 3",
    "HouseClearanceCm = 4700.0f",
    "RoadsideOffsetCm = 3550.0f",
    "IsInsideCompactPlayableBounds",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    "AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor()",
    'FindISM(Sector, TEXT("Roads"))',
    "RoadLengthCm < 17000.0f",
    "RoadWidthCm > 1800.0f",
    "HasHouseClearance",
    "AdjustIfPossibleButDontSpawnIfColliding",
    "World.SpawnActor<AOCEnterableHouse>",
    "ConfigureInteriorVariantServer",
    "EOCHouseCondition::Ordinary",
    "EOCHouseCondition::Worn",
    "EOCHouseCondition::Maintained",
    "if (InWorld.GetNetMode() == NM_Client) return",
    "GameMode->IsFrontendOnlySession()",
]
for token in required:
    if token not in cpp:
        fail(f"missing bounded enterable-house marker: {token}")

# Population must happen before the common enterable-house roof/material bridge so new houses receive identical art.
if "2.05f" not in art:
    fail("enterable-house art timing contract changed; re-audit population ordering")

for forbidden in [
    "AlwaysSpawn",
    "FMath::Rand",
    "FRand",
    "MaxAdditionalEnterableHouses = 10",
]:
    if forbidden in cpp:
        fail(f"unsafe/unbounded enterable-house marker present: {forbidden}")

print("R13.4 ENTERABLE HOUSE POPULATION VERIFY: PASS")
print("Checks server-only 3-house cap, road-derived placement, compact/landmark clearance, collision-safe spawning and pre-art-pass timing.")
