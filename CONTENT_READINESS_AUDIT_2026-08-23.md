# Oster Conflict — content readiness audit — 2026-08-23

This document records repository-proven state after Passes 15–19. Runtime screenshots/logs always override source-only claims.

## Source fixes merged before this audit

- Pass 15: server UI recovery, host/join fallback presentation, Museum BASE pawn recovery, physical 11-class rack, emergency performance recovery.
- Pass 16: one-time safe graphics ceiling plus real GPU/RHI logging.
- Pass 17: source-world ISM culling/shadow budget with collision preserved.
- Pass 18: world-density/navigation/RAM diagnostics beside final FPS evidence.

All of the above are source/CI verified. None of them is automatically `VERIFIED RUNTIME` until the local UE 5.8 acceptance launcher is run and the resulting gameplay log passes.

## Weapon truth

- Gameplay classes/types: 11 required rack classes exist in source and are spawned by the Museum BASE rack.
- Exact production art and playable real-mesh fallback are now separate contracts.
- Exact M249 production model is NOT present in the committed raw R13 weapon pack. Current fallback is the generic R13 `machinegun` mesh.
- Exact Remington 870 production model is NOT present in the committed raw R13 weapon pack. Current fallback is the generic R13 `shotgun` mesh.
- Generic fallback components use `OC_RealFallbackWeaponVisual` only and must never receive `OC_ProductionWeaponVisual`.
- `PASS7_PRODUCTION_WEAPONS_READY` means strict exact-production art only.
- `PASS19_PLAYABLE_WEAPON_SET_READY` means all 11 required classes are physically present and each has either exact production art or an explicit real-mesh fallback, with no primitive-only item.

## HMMWV / M2 Browning / BTR-4 truth

The repository contains import code and source metadata, but the real production source binaries are not committed in Git:

- HMMWV required source: `OsterConflict/SourceAssets/Production/Vehicles/HMMWV/ukrainian_hmmwv_mk_19.glb`
- M2 Browning required source: `OsterConflict/SourceAssets/Production/Weapons/M2/m2_50cal_machinegun_cc0.glb`
- BTR-4 required source: `OsterConflict/SourceAssets/Production/Vehicles/BTR4/BTR4_Bucephalus.fbx`

Current Git tree contains metadata files for these sources, not the binaries themselves. Therefore repository inspection alone cannot certify the real HMMWV, real M2 or real BTR-4 as installed locally.

- M2 importer can create an authored approximation when the downloaded source is absent, but `RUN_IMPORT_M2_PRODUCTION.cmd` intentionally rejects that approximation for strict gameplay acceptance and requires `source_kind=downloaded`.
- BTR-4 strict import requires `source_kind=local_user_fbx`.
- Full production vehicle import fails if any HMMWV/M2/BTR source binary is missing.
- Strict vehicle acceptance remains open until local source intake and UE runtime validation pass.

## Runtime issues from the latest user playtest

The following are source-fixed but still require a new local UE 5.8 runtime test:

- server fields/opaque setup UI;
- Create Server travel flow;
- Join pending/error presentation;
- actual player spawn at Museum BASE;
- physical weapon rack beside Museum;
- playable real-mesh rack validation;
- measured FPS >= 30 after Passes 15–17;
- Pass 18 diagnostic evidence.

Use `RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd` on updated `main`. It chains the focused frontend/Museum/FPS acceptance and additionally requires Pass 17 and Pass 18 runtime evidence.

## Still-open content work

- exact M249 model;
- exact Remington 870 model;
- local real production intake for HMMWV/M2/BTR-4;
- Museum/Silpo/Culture House/Stadium detailed runtime fidelity gaps;
- real terrain/elevation data;
- broader real Oster house set;
- any visual flicker/road-profile issue that still reproduces after the next local runtime acceptance.

Do not mark these items complete based on source code or CI alone.
