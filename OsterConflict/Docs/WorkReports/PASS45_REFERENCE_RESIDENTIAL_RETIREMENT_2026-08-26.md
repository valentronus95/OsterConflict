# PASS45 — Reference-driven residential retirement — 2026-08-26

Status: **CODED_UNTESTED**  
PR: #91  
Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`

## Scope

This milestone closes the source-side portion of Pass45 corrective execution item #11. It does **not** claim factual UE 5.8 runtime acceptance.

## Source retirement completed

- normal `AOCGameMode::SpawnOsterCenterSector()` no longer spawns the explicitly gameplay-authored/non-reference-specific `AOCEnterableHouse`;
- `AOCWorldSectorOster::BuildResidentialBlocks()` was physically removed together with its procedural houses, sheds and private-fence grids;
- the generic `BuildSolomiiKrushelnytskoiStreet()` house/shed/fence visual generator was physically removed;
- Solomii Krushelnytskoi road topology remains owned by `BuildRoadNetwork()`;
- reference-driven Museum, Stadium and College fence geometry remains intact;
- runtime source markers were added: `PASS45_GENERIC_ENTERABLE_HOUSE_RETIRED` and `PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED`.

## Regression guard

`VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` and its dedicated workflow now fail if the generic runtime owners are silently reintroduced.

During PR CI, historical `VERIFY_ROAD_PROFILE_RUNTIME_PASS_11.py` exposed stale dependencies on already-deleted `OCAssetModelDecorator.cpp` and `OCRecoveredRoadsidePropsSubsystem.cpp`. The verifier was forward-ported to the current road ownership model. The deleted owners were **not** restored.

## Remaining factual Gate E

The next local UE run must still prove:

- no rejected generic house/private-fence family is visible in the tested Oster area;
- the previously observed dark steep-roof tower/shack is absent;
- Museum/Stadium/College reference-driven geometry remains present;
- no visual regression appears in road topology after generic residential retirement.

Until that factual playtest, status remains **CODED_UNTESTED**.
