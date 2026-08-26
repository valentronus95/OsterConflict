from pathlib import Path

ROOT = Path(__file__).resolve().parent
GAME_MODE = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameMode.cpp"
WORLD_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp"
WORLD_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCWorldSectorOster.h"

for path in (GAME_MODE, WORLD_CPP, WORLD_H):
    if not path.is_file():
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing={path.relative_to(ROOT)}")

game_mode = GAME_MODE.read_text(encoding="utf-8")
world_cpp = WORLD_CPP.read_text(encoding="utf-8")
world_h = WORLD_H.read_text(encoding="utf-8")

forbidden_game_mode = {
    'generic_enterable_house_spawn': 'SpawnActor<AOCEnterableHouse>',
    'generic_enterable_house_include': '#include "OCEnterableHouse.h"',
    'generic_house_condition_include': '#include "OCHouseTypes.h"',
    'generic_house_authored_comment': 'the lot is gameplay-authored and is not a copy of a specific residence',
}
for name, token in forbidden_game_mode.items():
    if token in game_mode:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL stale={name}")

forbidden_world = {
    'residential_grid_call': 'BuildResidentialBlocks();',
    'residential_grid_owner': 'void AOCWorldSectorOster::BuildResidentialBlocks()',
    'generic_street_house_call': 'BuildSolomiiKrushelnytskoiStreet();',
    'generic_street_house_owner': 'void AOCWorldSectorOster::BuildSolomiiKrushelnytskoiStreet()',
}
for name, token in forbidden_world.items():
    if token in world_cpp:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL stale={name}")

forbidden_header = (
    'void BuildResidentialBlocks();',
    'void BuildSolomiiKrushelnytskoiStreet();',
)
for token in forbidden_header:
    if token in world_h:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL stale_header={token}")

required_world = (
    'PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED',
    'BuildMuseumAndStadium();',
    'BuildCentralPark();',
    'BuildCollegeSector();',
    'AddBox(Fences, Museum +',
    'AddBox(Fences, Stadium +',
    'AddBox(Fences, College +',
)
for token in required_world:
    if token not in world_cpp:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing_world_contract={token}")

required_game_mode = (
    'PASS45_GENERIC_ENTERABLE_HOUSE_RETIRED',
    'GetWorld()->SpawnActor<AOCWorldSectorOster>',
    'AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor()',
)
for token in required_game_mode:
    if token not in game_mode:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing_game_contract={token}")

if 'Private generic residences are intentionally omitted' not in world_h:
    raise SystemExit('PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing_header_truth')

print('PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=PASS generic_house_spawn=0 procedural_residential_grids=0 generic_private_fences=0 reference_poi_fences_retained=1')
