from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

OLD_PARK = """    AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));
    AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));
    AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);
    AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);
"""

NEW_PARK = """    for (const FOCS01PathSeed& Path : FOCLocationSectorS01RoadData::OwnedCentralParkPaths())
    {
        AddBox(Sidewalks, Park + Path.LocalOffset, Path.SizeCm, Path.Yaw);
    }
"""

OLD_COLLEGE = """    AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);
"""

NEW_COLLEGE = """    for (const FOCS01PathSeed& Path : FOCLocationSectorS01RoadData::OwnedCollegePaths())
    {
        AddBox(Sidewalks, College + Path.LocalOffset, Path.SizeCm, Path.Yaw);
    }
"""


def replace_once(text: str, old: str, new: str, label: str) -> tuple[str, bool]:
    count = text.count(old)
    if count == 1:
        return text.replace(old, new, 1), True
    if count == 0 and new in text:
        return text, False
    raise SystemExit(f"S01 inside-path migration refused: expected exactly one {label}, found {count}")


def main() -> None:
    raw = WORLD.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")

    text, park_changed = replace_once(text, OLD_PARK, NEW_PARK, "Central Park path block")
    text, college_changed = replace_once(text, OLD_COLLEGE, NEW_COLLEGE, "College path block")

    forbidden = [
        "AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));",
        "AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));",
        "AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);",
        "AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);",
        "AddBox(Sidewalks, College + FVector(900, 5200, 12), FVector(8000, 5900, 18), Yaw);",
    ]
    leftovers = [token for token in forbidden if token in text]
    if leftovers:
        raise SystemExit("S01 inside-path migration refused: legacy direct path calls survived: " + repr(leftovers))

    for required in [
        "FOCLocationSectorS01RoadData::OwnedCentralParkPaths()",
        "AddBox(Sidewalks, Park + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
        "FOCLocationSectorS01RoadData::OwnedCollegePaths()",
        "AddBox(Sidewalks, College + Path.LocalOffset, Path.SizeCm, Path.Yaw)",
        # Crossing derived link must stay independent.
        "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
    ]:
        if required not in text:
            raise SystemExit(f"S01 inside-path migration refused: missing runtime contract {required}")

    if not park_changed and not college_changed:
        print("S01 inside-path runtime migration: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 inside-path runtime migration: applied")


if __name__ == "__main__":
    main()
