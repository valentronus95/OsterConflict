from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13 LOCATION-FIRST S01 VEGETATION RUNTIME VERIFY FAIL: " + message)


if not WORLD.is_file():
    fail(f"missing file: {WORLD.relative_to(ROOT)}")

world = WORLD.read_text(encoding="utf-8", errors="replace")

required = [
    "auto ResolveS01VegetationAnchor = [&Park, &College](EOCS01VegetationAnchor Anchor)",
    "FOCLocationSectorS01Data::ProvisionalGrassPatches()",
    "ResolveS01VegetationAnchor(Patch.Anchor) + Patch.LocalOffset",
    "Patch.SizeCm, Patch.Yaw",
    "FOCLocationSectorS01Data::ProvisionalVegetationTrees()",
    "switch (Tree.Family)",
    "EOCS01TreeFamily::Poplar",
    "EOCS01TreeFamily::Birch",
    "EOCS01TreeFamily::Pine",
    "ResolveS01VegetationAnchor(Tree.Anchor) + Tree.LocalOffset, Tree.Scale, Family",
    "IsInsideKrushelnytskaCollegePark(RoughPatches[I])",
    "IsInsideKrushelnytskaCollegePark(TreeLocation)",
    # Stadium vegetation is not owned by S01 and must remain untouched by this migration.
    "AddGrassPatch(GrassMown, Stadium + FVector(0, 0, 0), FVector(14500, 9800, 4), 0.0f)",
    "AddTreeFamily(Stadium + FVector(I * 1500.0f, 5700.0f + (I % 2) * 350.0f, 0), 0.9f, Family)",
]
for token in required:
    if token not in world:
        fail(f"runtime registry contract missing: {token}")

forbidden = [
    "AddGrassPatch(GrassMown, Park + FVector(0, 0, 0)",
    "AddGrassPatch(GrassMown, College + FVector(0, 5200, 0)",
    "for (int32 Row = -3; Row <= 3; ++Row)",
    "for (int32 Col = -4; Col <= 4; ++Col)",
    "const float JitterX = static_cast<float>(((Row * 7 + Col * 3) % 5) - 2) * 180.0f",
    "const float JitterY = static_cast<float>(((Row * 5 + Col * 11) % 5) - 2) * 160.0f",
    "AddTreeFamily(College + FVector(-3800, -1100, 0)",
    "AddTreeFamily(College + FVector(3900, -950, 0)",
    "AddTreeFamily(College + FVector(-4600, 1500, 0)",
    "AddTreeFamily(College + FVector(4700, 2100, 0)",
]
for token in forbidden:
    if token in world:
        fail(f"legacy S01 vegetation placement returned: {token}")

print("R13 LOCATION-FIRST S01 VEGETATION RUNTIME VERIFY: PASS")
print("Checks registry-driven park/college rendering, preserved stadium vegetation and generic S01 exclusion guards.")
