from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

OLD = """    AddRoadWithWalks(FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f);
"""

NEW = """    for (const FOCS01RoadCorridorSeed& Segment : FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments())
    {
        AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks);
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
        raise SystemExit(f"Krushelnytska spine split refused: expected one legacy call, found {count}")

    if "FVector(-33500, 25000, RoadZ), FVector(112000, 920, 16), 91.5f" in text:
        raise SystemExit("Krushelnytska spine split refused: legacy unsplit corridor survived")
    for token in [
        "FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments()",
        "AddRoadWithWalks(Segment.LocalOffset, Segment.SizeCm, Segment.Yaw, Segment.bTwoWalks)",
    ]:
        if token not in text:
            raise SystemExit(f"Krushelnytska spine split refused: missing runtime contract {token}")

    if not changed:
        print("Krushelnytska spine split: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("Krushelnytska spine split: applied")


if __name__ == "__main__":
    main()
