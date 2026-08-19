from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"

OLD_GRASS = """    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();
    AddGrassPatch(GrassMown, Park + FVector(0, 0, 0), FVector(19000, 14500, 4), 6.0f);
    AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
    AddGrassPatch(GrassMown, College + FVector(0, 5200, 0), FVector(12500, 7600, 4), 2.0f);
"""

NEW_GRASS = """    const FVector Park = ParkAnchor();
    const FVector College = CollegeAnchor();
    const FVector Stadium = StadiumAnchor();

    auto ResolveS01VegetationAnchor = [&Park, &College](EOCS01VegetationAnchor Anchor)
    {
        return Anchor == EOCS01VegetationAnchor::College ? College : Park;
    };

    for (const FOCS01GrassPatchSeed& Patch : FOCLocationSectorS01Data::ProvisionalGrassPatches())
    {
        AddGrassPatch(GrassMown,
            ResolveS01VegetationAnchor(Patch.Anchor) + Patch.LocalOffset,
            Patch.SizeCm, Patch.Yaw);
    }

    AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f);
"""

OLD_TREES = """    for (int32 Row = -3; Row <= 3; ++Row)
    {
        for (int32 Col = -4; Col <= 4; ++Col)
        {
            if (FMath::Abs(Row) <= 1 && FMath::Abs(Col) <= 1) continue;
            const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f;
            const float JitterY = static_cast<float>(((Row * 5 + Col * 11) % 5) - 2) * 160.0f;
            const int32 Roll = FMath::Abs(Row * 9 + Col * 5) % 12;
            ETreeProxy Family = ETreeProxy::Broadleaf;
            if (Roll <= 2) Family = ETreeProxy::Poplar;
            else if (Roll == 3 || Roll == 4) Family = ETreeProxy::Birch;
            else if (Roll == 5) Family = ETreeProxy::Pine;
            AddTreeFamily(Park + FVector(Col * 1850.0f + JitterX, Row * 1700.0f + JitterY, 0),
                0.85f + 0.05f * static_cast<float>((Row + Col + 8) % 4), Family);
        }
    }

    AddTreeFamily(College + FVector(-3800, -1100, 0), 1.2f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(3900, -950, 0), 1.15f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(-4600, 1500, 0), 1.0f, ETreeProxy::Pine);
    AddTreeFamily(College + FVector(4700, 2100, 0), 0.9f, ETreeProxy::Birch);
"""

NEW_TREES = """    for (const FOCS01TreeSeed& Tree : FOCLocationSectorS01Data::ProvisionalVegetationTrees())
    {
        ETreeProxy Family = ETreeProxy::Broadleaf;
        switch (Tree.Family)
        {
            case EOCS01TreeFamily::Poplar: Family = ETreeProxy::Poplar; break;
            case EOCS01TreeFamily::Birch:  Family = ETreeProxy::Birch; break;
            case EOCS01TreeFamily::Pine:   Family = ETreeProxy::Pine; break;
            default: break;
        }

        AddTreeFamily(ResolveS01VegetationAnchor(Tree.Anchor) + Tree.LocalOffset, Tree.Scale, Family);
    }
"""


def replace_exactly_once(text: str, old: str, new: str, label: str) -> tuple[str, bool]:
    count = text.count(old)
    if count == 1:
        return text.replace(old, new, 1), True
    if count == 0 and new in text:
        return text, False
    raise SystemExit(f"S01 vegetation migration refused: expected exactly one {label} block, found {count}")


def main() -> None:
    if not WORLD.is_file():
        raise SystemExit(f"S01 vegetation migration refused: missing {WORLD}")

    raw = WORLD.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8")
    normalized = text.replace("\r\n", "\n")

    normalized, grass_changed = replace_exactly_once(normalized, OLD_GRASS, NEW_GRASS, "park/college grass")
    normalized, trees_changed = replace_exactly_once(normalized, OLD_TREES, NEW_TREES, "park/college tree")

    forbidden = [
        "AddGrassPatch(GrassMown, Park + FVector(0, 0, 0)",
        "AddGrassPatch(GrassMown, College + FVector(0, 5200, 0)",
        "for (int32 Row = -3; Row <= 3; ++Row)",
        "const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f;",
        "AddTreeFamily(College + FVector(-3800, -1100, 0)",
        "AddTreeFamily(College + FVector(3900, -950, 0)",
        "AddTreeFamily(College + FVector(-4600, 1500, 0)",
        "AddTreeFamily(College + FVector(4700, 2100, 0)",
    ]
    leftovers = [token for token in forbidden if token in normalized]
    if leftovers:
        raise SystemExit("S01 vegetation migration refused: legacy S01 runtime placement survived: " + repr(leftovers))

    required = [
        "FOCLocationSectorS01Data::ProvisionalGrassPatches()",
        "FOCLocationSectorS01Data::ProvisionalVegetationTrees()",
        "ResolveS01VegetationAnchor(Patch.Anchor) + Patch.LocalOffset",
        "ResolveS01VegetationAnchor(Tree.Anchor) + Tree.LocalOffset",
        "Tree.Scale",
        "Tree.Family",
    ]
    missing = [token for token in required if token not in normalized]
    if missing:
        raise SystemExit("S01 vegetation migration refused: registry runtime contract incomplete: " + repr(missing))

    if not grass_changed and not trees_changed:
        print("S01 vegetation runtime migration: already applied")
        return

    output = normalized if newline == "\n" else normalized.replace("\n", "\r\n")
    WORLD.write_bytes(output.encode("utf-8"))
    print("S01 vegetation runtime migration: applied")
    print(f"Updated: {WORLD.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
