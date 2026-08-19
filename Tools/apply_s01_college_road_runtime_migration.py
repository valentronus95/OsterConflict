from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

OLD_INCLUDE = '#include "OCLocationSectorS01Data.h"\n'
NEW_INCLUDE = '#include "OCLocationSectorS01Data.h"\n#include "OCLocationSectorS01RoadData.h"\n'

OLD_BLOCK = """    const FVector College = CollegeAnchor();
    AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);
"""

NEW_BLOCK = """    const FVector College = CollegeAnchor();
    auto ResolveS01RoadAnchor = [&Park, &College](EOCS01RoadAnchor Anchor)
    {
        if (Anchor == EOCS01RoadAnchor::College) return College;
        if (Anchor == EOCS01RoadAnchor::CentralPark) return Park;
        return FVector::ZeroVector;
    };

    for (const FOCS01RoadCorridorSeed& Road : FOCLocationSectorS01RoadData::OwnedInsideCorridors())
    {
        AddRoadWithWalks(ResolveS01RoadAnchor(Road.Anchor) + Road.LocalOffset,
            Road.SizeCm, Road.Yaw, Road.bTwoWalks);
    }
"""


def replace_once(text: str, old: str, new: str, label: str) -> tuple[str, bool]:
    count = text.count(old)
    if count == 1:
        return text.replace(old, new, 1), True
    if count == 0 and new in text:
        return text, False
    raise SystemExit(f"S01 college road migration refused: expected exactly one {label}, found {count}")


def main() -> None:
    raw = WORLD.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8").replace("\r\n", "\n")

    text, include_changed = replace_once(text, OLD_INCLUDE, NEW_INCLUDE, "road-data include")
    text, block_changed = replace_once(text, OLD_BLOCK, NEW_BLOCK, "college approach road block")

    forbidden = "AddRoadWithWalks(College + FVector(-13500, 0, RoadZ), FVector(30000, 660, 14), 0.0f);"
    if forbidden in text:
        raise SystemExit("S01 college road migration refused: direct college road call survived")

    for required in [
        'OCLocationSectorS01RoadData.h',
        'FOCLocationSectorS01RoadData::OwnedInsideCorridors()',
        'ResolveS01RoadAnchor(Road.Anchor) + Road.LocalOffset',
        'Road.SizeCm, Road.Yaw, Road.bTwoWalks',
    ]:
        if required not in text:
            raise SystemExit(f"S01 college road migration refused: missing runtime contract {required}")

    if not include_changed and not block_changed:
        print("S01 college road runtime migration: already applied")
        return

    output = text if newline == "\n" else text.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 college road runtime migration: applied")


if __name__ == "__main__":
    main()
