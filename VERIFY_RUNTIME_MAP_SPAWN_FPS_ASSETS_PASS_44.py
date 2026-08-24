#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
P = ROOT / "OsterConflict"
SRC = P / "Source" / "OsterConflict"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(cond: bool, message: str) -> None:
    if not cond:
        errors.append(message)


def forbid(text: str, needle: str, message: str) -> None:
    if needle in text:
        errors.append(message)


agents = read(ROOT / "AGENTS.md")
ledger = read(ROOT / "OSTER_CONFLICT_WORK_LEDGER.md")
game_h = read(SRC / "Public" / "OCGameMode.h")
game = read(SRC / "Private" / "OCGameMode.cpp")
runtime = read(SRC / "Private" / "OCGameModeRuntimeSafe.cpp")
team_spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
world = read(SRC / "Private" / "OCWorldSectorOster.cpp")
central = read(SRC / "Private" / "OCCentralPlayableAreaSubsystem.cpp")
tactical = read(SRC / "Private" / "OCTacticalMapVisual.cpp")
weapon = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
palette = read(SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp")

reference = ROOT / "REFERENCE_PHOTOS" / "map_extent" / "oster_central_playable_area_20260824.jpg"
req(reference.is_file() and reference.stat().st_size > 0,
    "compact central Oster reference image is missing/empty")

for needle in (
    "Latest user-observed runtime beats source",
    "Mandatory stale-rule retirement",
    "No compatibility resurrection",
    "Playable-map size is user-authoritative",
    "Museum BASE means actual pawn placement",
    "Runtime content truth is fail-visible",
    "Normal local game must not silently auto-fill",
):
    req(needle in agents, f"root authority policy missing: {needle}")

# Pass 44 is historical and has now been rejected by factual runtime evidence. Its verifier may only
# protect useful non-regression decisions; it must never force Pass 44 back to ACTIVE/VERIFIED status.
req("Pass 44 verdict: RUNTIME REJECTED" in ledger,
    "ledger must preserve factual Pass 44 runtime rejection")
req("Pass 45" in ledger and "ACTIVE" in ledger,
    "ledger must identify Pass 45 as the active corrective pass")
req("Pass 44 behavior retained unless disproved" in ledger,
    "ledger is missing the explicit retained Pass 44 non-regression section")

req("int32 TargetPopulation = 0" in game_h and "bool bAutoFillBots = false" in game_h,
    "implicit bot autofill defaults returned")
req("else TargetPopulation = 0;" in game,
    "normal local gameplay no longer keeps zero implicit bot population")
req("PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY" in runtime,
    "actual Museum pawn distance evidence path was removed")
req("PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY" in team_spawn,
    "Museum BASE role became coordinate-edge dependent again")
for stale in ("LegacyLocation.Y > 92000.0f", "LegacyLocation.Y < -92000.0f"):
    forbid(team_spawn, stale, "retired ±920 m BASE discriminator returned")

for needle in (
    "MinPlayableX = -78000.0f",
    "MaxPlayableX =  18000.0f",
    "MinPlayableY = -12000.0f",
    "MaxPlayableY =  82000.0f",
    "PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY",
):
    req(needle in world, f"compact primary authoring non-regression missing: {needle}")
for stale in (
    "constexpr float MapWidthCm = 240000.0f",
    "constexpr float MapHeightCm = 240000.0f",
    "FVector(-104000.0f, -92000.0f",
    "FVector( 104000.0f,  92000.0f",
):
    forbid(world, stale, f"retired peripheral world authoring returned: {stale}")

for needle in (
    "MinPlayableX = -78000.0f",
    "MaxPlayableX =  18000.0f",
    "MinPlayableY = -12000.0f",
    "MaxPlayableY =  82000.0f",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
):
    req(needle in central, f"compact playable-area safety net missing: {needle}")

for needle in (
    "Pass44PlayableMinX = -78000.0f",
    "Pass44PlayableMaxX =  18000.0f",
    "Pass44PlayableMinY = -12000.0f",
    "Pass44PlayableMaxY =  82000.0f",
    "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
):
    req(needle in tactical, f"compact tactical-map bound non-regression missing: {needle}")
for stale in (
    "FMath::Clamp(HalfSize.X, 80000.0f, 120000.0f)",
    "HalfSize += FVector2D(30000.0f, 26000.0f)",
    "AccumulateComponentBounds2D",
):
    forbid(tactical, stale, f"obsolete tactical auto-fit/minimum returned: {stale}")

for needle in (
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_READY",
    "basicshape_repair=0",
):
    req(needle in weapon, f"weapon material truth non-regression missing: {needle}")
for stale in ("MaterialRecoveryBase", "UMaterialInstanceDynamic::Create", "Component->SetMaterial(Slot"):
    forbid(weapon, stale, f"retired grey weapon material repair returned: {stale}")
req("PASS44_WEAPON_PALETTE_MUTATION_DISABLED" in palette,
    "retired palette subsystem is no longer inert")

if errors:
    print("PASS44 HISTORICAL NON-REGRESSION: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS44 HISTORICAL NON-REGRESSION: PASS")
print("- factual Pass 44 runtime rejection is preserved; this verifier cannot promote it back to active/verified")
print("- compact 960x940 m extent, zero implicit bots and actual Museum pawn proof remain protected")
print("- old edge coordinates/map auto-fit and grey weapon-material repair remain retired")
print("- tactical topology, FPS, trees, landmarks and authored weapon materials are delegated to active Pass 45")
print("STATUS: HISTORICAL SOURCE NON-REGRESSION ONLY; latest UE runtime is authoritative")
