# Pass 45 corrective source milestone — Museum / vehicles / materials

Date: 2026-08-25
Branch: `fix/pass45-runtime-rejection-20260825`
Status: **CODED_UNTESTED / factual local UE 5.8 runtime still required**

## Factual starting point

The latest local normal-route runtime reached gameplay but was rejected. The user-visible failures included Museum identity/layer overlap, vehicle/character teleport around possession/exit, distorted production vehicles, wrong M2 vertical aim direction, production material corruption/default slots, and strong thermal load despite high FPS.

A green source verifier is not runtime acceptance. This milestone only records the current corrective source state.

## Corrective source changes in this milestone

### Museum ownership consolidation

- R13.7 is the single visible Museum exterior owner.
- R13.7 no longer authors temporary site trees, static window glass, static prototype door slabs, the wrong service canopy/door prototype, or the obsolete temporary gable that later stages replaced.
- R13.8 is hidden collision plus final breakable glass/interactivity only; its collision components are hidden and material-free.
- R13.8 no longer spawns generic prototype doors.
- `AOCMuseumBreakableWindow` is glass/debris-only at runtime; R13.7 owns visible frame/grille detail.
- R13.9 owns the final replicated main entrance door.
- R14.0 owns final service-gable/canopy/service-door and upper-window detail without late R13.7 hiding or `RemoveInstance` repair.
- R14.5 is the sole current Museum tree-layout owner and no longer hides an R13.7 tree pass.
- obsolete `OCR141MuseumWindowReplacementSubsystem.h/.cpp` was physically deleted.
- the startup coordinator no longer runs the retired R14.1 Museum window replacement stage.

Primary source markers include:

- `PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY`
- `PASS45_MUSEUM_R138_COLLISION_ONLY_READY`
- `PASS45_MUSEUM_INTERACTIVE_OPENINGS_READY ... final_window_class=1 prototype_doors=0`
- `PASS45_MUSEUM_WINDOW_GLASS_ONLY_READY`
- `PASS45_MUSEUM_R140_DETAIL_ONLY_READY`
- `PASS45_MUSEUM_TREE_SINGLE_OWNER_READY`
- `PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY`

### Production vehicle material ownership

- `AOCVehicleBase::BeginPlay()` now skips legacy BasicShape tinting for any mesh under `/Game/Production/`.
- the historical production-vehicle visual guard is now read-only validation; it no longer calls `EmptyOverrideMaterials()` or repairs materials on a timer.
- production HMMWV/M2/BTR material mismatch is fail-visible instead of silently recoloured.

Markers:

- `PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY`
- `PASS45_PRODUCTION_VEHICLE_VALIDATION_SCHEDULED`
- `PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY`
- failure markers remain explicit for material override/content gaps.

### Vehicle possession/exit transform evidence

- driver enter records the vehicle transform before/after possession and fails if possession moves the vehicle unexpectedly;
- driver exit records current vehicle location, requested exit location and resulting pawn location;
- gunner exit records the same transform evidence;
- normal vehicle exit evidence explicitly declares `museum_respawn_path=0`.

Markers:

- `PASS45_VEHICLE_ENTER_TRANSFORM_READY/FAIL`
- `PASS45_VEHICLE_EXIT_TRANSFORM_READY/FAIL`
- `PASS45_GUNNER_EXIT_TRANSFORM_READY/FAIL`

### M2 gunner pitch contract

Default non-inverted mouse input now increases mounted gunner pitch when the input value is positive. Optional invert-Y is the only path that reverses this behavior.

Marker:

- `PASS45_M2_GUNNER_PITCH_CONTRACT_READY default_invert=0 mouse_up_raises=1`

## Stale-rule retirement

`VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` now rejects resurrection of:

- obsolete R14.1 Museum window replacement source;
- temporary patch workflows;
- late R14.0 Museum suppression/removal;
- R13.7 prototype trees/static glass/door/gable ownership;
- runtime production-material repair;
- inverted M2 pitch;
- missing vehicle transform telemetry.

## Remaining Pass 45 work

Still unresolved or runtime-unproven:

1. local UE 5.8 compile/build after these changes;
2. factual normal-route runtime Museum screenshot proving the six-column/Culture-House-like overlap is gone;
3. factual vehicle drive/exit test away from Museum proving no teleport;
4. factual M2 mounted aim test proving mouse-up raises the gun with invert-Y off;
5. factual HMMWV/M2/BTR authored-material appearance;
6. white/default required weapon material/texture gaps;
7. rejected generic Oster house/fence/tower visuals;
8. thermal behavior and sustained >=30 FPS under visually valid gameplay;
9. final current-head pull-request CI before merge.

## Verdict

**SOURCE CORRECTIVE MILESTONE: CODED_UNTESTED.**

Do not promote Pass 45 to runtime verified until the local UE 5.8 acceptance run proves the current binary behavior.
