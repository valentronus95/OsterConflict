# Oster Conflict — content readiness audit — 2026-08-23

This document records repository-proven state after Passes 15–21. Runtime screenshots/logs always override source-only claims.

## Current main/source state

- Pass 15: server UI recovery, host/join fallback presentation, Museum BASE pawn recovery, physical 11-class rack, emergency performance recovery.
- Pass 16: one-time safe graphics ceiling plus real GPU/RHI logging.
- Pass 17: source-world ISM culling/shadow budget with collision preserved.
- Pass 18: world-density/navigation/RAM diagnostics beside final FPS evidence.
- Pass 19: playable real-mesh weapon readiness is separated from exact production-art readiness.
- Pass 20: `START_HERE.cmd` normal game remains on `RUN_R14_CURRENT_GAMEPLAY.cmd`; exact HMMWV/M2/BTR source intake is required only for strict acceptance and no longer blocks the normal frontend.
- Pass 21: current Museum/Silpo/Culture landmark owners are guarded against late duplicate shell rebuilds and receive explicit final runtime ownership evidence after the historical startup window.

Passes through 20 are merged/source-CI verified. Pass 21 is source work pending PR/CI at the time of this document update. None of these source changes is automatically `VERIFIED RUNTIME` until the local UE 5.8 acceptance route is run and the resulting gameplay log passes.

## Weapon truth

- Gameplay classes/types: 11 required rack classes exist in source and are spawned by the Museum BASE rack.
- Exact production art and playable real-mesh fallback are separate contracts.
- Exact M249 production model is NOT present in the committed raw R13 weapon pack. Current playable fallback is the generic R13 `machinegun` mesh.
- Exact Remington 870 production model is NOT present in the committed raw R13 weapon pack. Current playable fallback is the generic R13 `shotgun` mesh.
- Generic fallback components use `OC_RealFallbackWeaponVisual` only and must never receive `OC_ProductionWeaponVisual`.
- `PASS7_PRODUCTION_WEAPONS_READY` means strict exact-production art only.
- `PASS19_PLAYABLE_WEAPON_SET_READY` means all 11 required classes are physically present and each has either exact production art or an explicit real-mesh fallback, with no primitive-only item.

## HMMWV / M2 Browning / BTR-4 truth

The repository contains import code and source metadata, but the real production source binaries are not committed in Git:

- HMMWV required source: `OsterConflict/SourceAssets/Production/Vehicles/HMMWV/ukrainian_hmmwv_mk_19.glb`
- M2 Browning required source: `OsterConflict/SourceAssets/Production/Weapons/M2/m2_50cal_machinegun_cc0.glb`
- BTR-4 required source: `OsterConflict/SourceAssets/Production/Vehicles/BTR4/BTR4_Bucephalus.fbx`

Current Git tree and the older production/import branches inspected on 2026-08-23 contain metadata for these sources, not the source binaries themselves. Repository inspection therefore cannot certify real HMMWV, M2 or BTR-4 production art as installed locally.

- M2 importer can create an authored approximation when the downloaded source is absent, but `RUN_IMPORT_M2_PRODUCTION.cmd` intentionally rejects that approximation for strict gameplay acceptance and requires `source_kind=downloaded`.
- BTR-4 strict import requires `source_kind=local_user_fbx`.
- Full production vehicle import fails if any HMMWV/M2/BTR source binary is missing.
- Pass 20 intentionally allows the normal game/frontend to run without this exact vehicle intake. Strict production acceptance still fails closed until the source binaries are supplied and validated in UE.

## Landmark ownership truth

Canonical geo points are separate, not overlapping: Museum/Silpo/Culture House are hundreds of metres apart except Museum–Culture House, which is still roughly 178 m. The observed overlap cannot be explained by the canonical coordinates themselves.

The early runtime landmark stages still contain historical delayed callbacks in addition to the current startup coordinator. Audit found that:

- R13.7 Museum can create `R137_MuseumPhotoModel` again without its own duplicate guard;
- R13.8 Museum can create `R138_MuseumHighFidelityArchitecture` again without its own duplicate guard;
- a late R13.7 rebuild is especially dangerous because R13.8 suppresses/upgrades the first R13.7 prototype, while a newly spawned late prototype is not automatically re-suppressed;
- R14.0 Silpo can rebuild `R140_SilpoPhotoModel`; its cleanup identifies `R140Silpo_*` components as legacy, which can hide the previous current shell before the replacement is spawned;
- Culture House R14.6 already refuses a second `R146_CultureHouseAuthoritative` owner.

Pass 21 adds a lifetime runtime ownership guard with site-specific repair policy, final owner counts, canonical-site instance checks and markers:

- `PASS21_LANDMARK_DUPLICATE_REPAIRED`
- `PASS21_LANDMARK_OWNERSHIP_READY`
- `PASS21_LANDMARK_OWNERSHIP_FAIL`

This is an ownership/overlap correction and diagnostic. It does NOT certify photo fidelity or exact facade appearance.

## Runtime issues still requiring local UE 5.8 evidence

- server fields/opaque setup UI;
- Create Server travel flow;
- Join pending/error presentation;
- actual player spawn at Museum BASE;
- physical weapon rack beside Museum;
- playable real-mesh rack validation;
- measured FPS >= 30 after Passes 15–17;
- Pass 18 diagnostic evidence;
- Pass 21 final landmark owner uniqueness/anchor evidence;
- visual confirmation that Museum/Silpo/Culture overlap no longer reproduces;
- road profile, distant flicker and shadow stability after the existing source fixes.

Use `RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd` for the performance chain. After Pass 21 reaches `main`, use `RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd` for focused landmark ownership evidence.

## Still-open content work

- exact M249 model;
- exact Remington 870 model;
- local real production intake for HMMWV/M2/BTR-4;
- Museum/Silpo/Culture House/Stadium detailed photo fidelity beyond ownership;
- real terrain/elevation data;
- broader real Oster house set;
- any visual flicker/road-profile issue that still reproduces after the next local runtime acceptance.

Do not mark these items complete based on source code or CI alone.
