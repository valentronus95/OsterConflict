from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

REPLACEMENTS = [
    (
        """    AddRoadWithWalks(FVector(-23500, 40500, RoadZ), FVector(51000, 760, 16), 18.0f);\n""",
        """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::WorldDiag01Segments())\n    {\n        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);\n    }\n""",
        "WorldDiag01",
    ),
    (
        """    AddRoadWithWalks(FVector(-5000, 33500, RoadZ), FVector(49000, 760, 16), -34.0f);\n""",
        """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::WorldDiag02Segments())\n    {\n        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);\n    }\n""",
        "WorldDiag02",
    ),
    (
        """    AddRoadWithWalks(Park + FVector(0, -8500, RoadZ), FVector(43000, 720, 16), 2.0f);\n""",
        """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::ParkSouthSegments())\n    {\n        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);\n    }\n""",
        "ParkSouth",
    ),
]


def main() -> None:
    raw = WORLD.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")
    changed = False

    for old, new, label in REPLACEMENTS:
        count = text.count(old)
        if count == 1:
            text = text.replace(old, new, 1)
            changed = True
        elif count == 0 and new in text:
            pass
        else:
            raise SystemExit(f"S01 symmetric split refused for {label}: expected one legacy call, found {count}")

    forbidden = [old.strip() for old, _new, _label in REPLACEMENTS]
    leftovers = [token for token in forbidden if token in text]
    if leftovers:
        raise SystemExit("S01 symmetric split refused: legacy direct calls survived: " + repr(leftovers))

    for token in [
        "FOCLocationSectorS01RoadData::WorldDiag01Segments()",
        "FOCLocationSectorS01RoadData::WorldDiag02Segments()",
        "FOCLocationSectorS01RoadData::ParkSouthSegments()",
    ]:
        if token not in text:
            raise SystemExit(f"S01 symmetric split refused: missing runtime contract {token}")

    if not changed:
        print("S01 symmetric crossing splits: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 symmetric crossing splits: applied")


if __name__ == "__main__":
    main()
