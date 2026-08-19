from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

REPLACEMENTS = [
    (
        """    AddRoadWithWalks(FVector(-48000, 51000, RoadZ), FVector(52000, 720, 16), 63.0f, false);\n""",
        """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::WorldNW01Segments())\n    {\n        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);\n    }\n""",
        "WorldNW01",
    ),
    (
        """    AddRoadWithWalks(Park + FVector(-9000, 13500, RoadZ), FVector(37000, 700, 16), 79.0f, false);\n""",
        """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::ParkNorthLinkSegments())\n    {\n        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);\n    }\n""",
        "ParkNorthLink",
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
            raise SystemExit(f"S01 one-sided split refused for {label}: expected one legacy call, found {count}")

    leftovers = [old.strip() for old, _new, _label in REPLACEMENTS if old.strip() in text]
    if leftovers:
        raise SystemExit("S01 one-sided split refused: legacy direct calls survived: " + repr(leftovers))

    for token in [
        "FOCLocationSectorS01RoadData::WorldNW01Segments()",
        "FOCLocationSectorS01RoadData::ParkNorthLinkSegments()",
    ]:
        if token not in text:
            raise SystemExit(f"S01 one-sided split refused: missing runtime contract {token}")

    if not changed:
        print("S01 one-sided crossing splits: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 one-sided crossing splits: applied")


if __name__ == "__main__":
    main()
