from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13EnterableHousePopulationSubsystem.h"
CPP = SRC / "Private" / "OCR13EnterableHousePopulationSubsystem.cpp"
ART_H = SRC / "Public" / "OCR13EnterableHouseArtSubsystem.h"
ART = SRC / "Private" / "OCR13EnterableHouseArtSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.4 ENTERABLE HOUSE POPULATION VERIFY FAIL: " + message)


for path in (H, CPP, ART_H, ART):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
art_h = ART_H.read_text(encoding="utf-8", errors="replace")
art = ART.read_text(encoding="utf-8", errors="replace")

for header_name, header_text in (("population", h), ("art", art_h)):
    includes = [line.strip() for line in header_text.splitlines() if line.strip().startswith("#include")]
    if not includes or "generated.h" not in includes[-1]:
        fail(f"{header_name} generated.h must remain the final header include")

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

# Population starts before the art bridge. The art bridge then retries for a bounded window so a slow remote client
# that receives replicated houses after the first pass still gets identical roofs/materials without duplicate roofs.
for token in [
    "FirstArtPassDelaySeconds = 2.05f",
    "ArtRetryIntervalSeconds = 0.75f",
    "MaxArtRetryPasses = 8",
    "StyledHouses.Contains(HouseKey)",
    "StyledHouses.Add(HouseKey)",
    'FindObjectFast<UInstancedStaticMeshComponent>(House, TEXT("R13_EnterableRoof"))',
    "World.GetTimerManager().ClearTimer(ArtRetryTimer)",
    "GameMode->IsFrontendOnlySession()",
]:
    if token not in art:
        fail(f"late-replication art retry marker missing: {token}")
for token in ["StyledHouses", "ArtRetryTimer", "ArtRetryPass"]:
    if token not in art_h:
        fail(f"enterable-house art retry state missing: {token}")

for forbidden in [
    "AlwaysSpawn",
    "FMath::Rand",
    "FRand",
    "MaxAdditionalEnterableHouses = 10",
]:
    if forbidden in cpp:
        fail(f"unsafe/unbounded enterable-house marker present: {forbidden}")

print("R13.4 ENTERABLE HOUSE POPULATION VERIFY: PASS")
print("Checks server-only 3-house cap, road/compact/landmark clearance, collision-safe spawning and bounded retry styling for late replicated houses.")
