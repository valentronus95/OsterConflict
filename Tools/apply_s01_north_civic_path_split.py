from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

OLD = """    const FVector NorthCivic = CultureParkNorthAnchor();
    const FVector Mid = (Park + NorthCivic) * 0.5f;
    const FVector Delta = NorthCivic - Park;
    const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);
"""

NEW = """    const FVector NorthCivic = CultureParkNorthAnchor();
    AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));
    for (const FOCS01PathSeed& Path : FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments())
    {
        AddBox(Sidewalks, Path.LocalOffset, Path.SizeCm, Path.Yaw);
    }
"""


def main() -> None:
    raw = WORLD.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")

    count = text.count(OLD)
    if count == 1:
        text = text.replace(OLD, NEW, 1)
        changed = True
    elif count == 0 and NEW in text:
        changed = False
    else:
        raise SystemExit(f"S01 north-civic path split refused: expected one legacy block, found {count}")

    for forbidden in [
        "const FVector Mid = (Park + NorthCivic) * 0.5f;",
        "const FVector Delta = NorthCivic - Park;",
        "const float LinkYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));",
        "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
    ]:
        if forbidden in text:
            raise SystemExit(f"S01 north-civic path split refused: legacy derived path survived: {forbidden}")

    for required in [
        "FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments()",
        "AddBox(Sidewalks, Path.LocalOffset, Path.SizeCm, Path.Yaw);",
        "AddBox(ParkGeometry, NorthCivic + FVector(0,0,4), FVector(8500, 7200, 8));",
    ]:
        if required not in text:
            raise SystemExit(f"S01 north-civic path split refused: missing runtime contract {required}")

    if not changed:
        print("S01 north-civic path split: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 north-civic path split: applied")


if __name__ == "__main__":
    main()
