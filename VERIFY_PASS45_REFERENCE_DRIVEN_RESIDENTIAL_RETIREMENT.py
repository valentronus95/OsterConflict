from pathlib import Path

ROOT = Path(__file__).resolve().parent
GAME_MODE = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGameMode.cpp"
WORLD_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp"
WORLD_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCWorldSectorOster.h"
VALIDATOR_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCReferenceDrivenResidentialValidationSubsystem.cpp"
VALIDATOR_H = ROOT / "OsterConflict/Source/OsterConflict/Public/OCReferenceDrivenResidentialValidationSubsystem.h"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
STRICT = ROOT / "VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py"

for path in (GAME_MODE, WORLD_CPP, WORLD_H, VALIDATOR_CPP, VALIDATOR_H, EVIDENCE, STRICT):
    if not path.is_file():
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing={path.relative_to(ROOT)}")

game_mode = GAME_MODE.read_text(encoding="utf-8")
world_cpp = WORLD_CPP.read_text(encoding="utf-8")
world_h = WORLD_H.read_text(encoding="utf-8")
validator_cpp = VALIDATOR_CPP.read_text(encoding="utf-8")
validator_h = VALIDATOR_H.read_text(encoding="utf-8")
evidence = EVIDENCE.read_text(encoding="utf-8")
strict = STRICT.read_text(encoding="utf-8")

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

# Gate E runtime validator is observation-only. It scans the final world once after startup and rejects any
# resurrection of procedural residential/fence instances or the specifically rejected village/tower/shack family.
for token in (
    'UOCReferenceDrivenResidentialValidationSubsystem : public UWorldSubsystem',
    'ValidateReferenceDrivenResidentialWorld',
):
    if token not in validator_h:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing_validator_header={token}")

for token in (
    'ValidationDelaySeconds = 2.0f',
    'PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_SCHEDULED',
    'PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY',
    'PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL',
    'AdvancedVillagePack',
    'OCEnterableHouse',
    'SteepRoof',
    'Shack',
    'Tower',
    'Name == TEXT("Buildings")',
    'Name == TEXT("ResidentialRoofs")',
    'Name == TEXT("ResidentialDetails")',
    'Name == TEXT("WoodFences")',
    'Name == TEXT("MetalFences")',
    'Name == TEXT("LightSheetFences")',
    'GenericBuildingInstances == 0',
    'GenericRoofInstances == 0',
    'GenericDetailInstances == 0',
    'GenericPrivateFenceInstances == 0',
    'RejectedNamedActors == 0',
    'RejectedMeshComponents == 0',
):
    if token not in validator_cpp:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL missing_runtime_validator_contract={token}")

for forbidden in (
    'Destroy(',
    'RemoveInstance(',
    'SetVisibility(',
    'SetHiddenInGame(',
    'SetMaterial(',
    'SetStaticMesh(',
    'SpawnActor<',
):
    if forbidden in validator_cpp:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL mutating_validator={forbidden}")

for token in (
    'PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY',
    'PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL',
    'REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_CONTRACT=PASS',
):
    if token not in evidence:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL evidence_not_wired={token}")
    if token not in strict:
        raise SystemExit(f"PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=FAIL strict_harness_not_wired={token}")

print('PASS45_REFERENCE_RESIDENTIAL_RETIREMENT=PASS generic_house_spawn=0 procedural_residential_grids=0 generic_private_fences=0 runtime_gate_e_fail_visible=1 reference_poi_fences_retained=1')
