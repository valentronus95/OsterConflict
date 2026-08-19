from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

old = '''    const FVector NorthCivic = CultureParkNorthAnchor();
    const FVector Mid = (Park + NorthCivic) * 0.5f;
    const FVector Delta = NorthCivic - Park;
    const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);
'''

new = '''    const FVector NorthCivic = CultureParkNorthAnchor();
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    for (const FOCS01PathSeed& Path : FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments())
    {
        // These records are absolute world coordinates so the original Park -> CultureParkNorth contour is preserved.
        AddBox(Sidewalks, Path.LocalOffset, Path.SizeCm, Path.Yaw);
    }
'''

text = WORLD.read_text(encoding="utf-8")
count = text.count(old)
if count != 1:
    raise SystemExit(f"S01 PARK-NORTH MIGRATION FAIL: expected legacy derived path block exactly once, found {count}")

WORLD.write_text(text.replace(old, new, 1), encoding="utf-8")
print("S01 PARK-NORTH MIGRATION: PASS")
print("Replaced the final derived CentralPark -> CultureParkNorth sidewalk with explicit split ownership records.")
