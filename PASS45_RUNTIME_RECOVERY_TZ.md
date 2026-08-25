# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date opened: 2026-08-24  
Latest factual runtime rejection: 2026-08-25  
Status: **PASS 45 ACTIVE / PR #83 MERGED + SOURCE CI GREEN / POST-MERGE CONTENT CLOSURE IN PROGRESS / RUNTIME REJECTED**  
Merged corrective PR: `#83`  
Merged main SHA: `f5e883fb69ae8bdd35c754dc895d8b06e4843e08`  
Active post-merge branch: `fix/pass45-postmerge-content-closure-20260825`  
Active post-merge PR: `#85`  
Target: UE 5.8.x Windows  
User launcher: `START_HERE.cmd`

## 1. Authority

This document is the current Pass45 execution contract.

Priority order:

1. latest explicit user requirement;
2. latest factual local UE runtime/build evidence;
3. current source;
4. source verifiers/CI;
5. historical plans and old READY markers.

A source-only success is **not** runtime acceptance. A mesh that loads is **not** proof that its authored material/texture chain is correct. A historical verifier may not force resurrection of a behavior that later runtime rejected.

## 2. Current factual verdict

The latest local UE 5.8 gameplay from 2026-08-25 is still authoritative and remains **RUNTIME REJECTED**.

Observed rejection evidence included:

- black/invalid world or ground regions;
- multiple white/default rack weapons;
- generic village houses/fences inconsistent with Oster references;
- Museum identity/ownership wrong or visually confused with Culture House;
- unreferenced dark steep-roof tower/shack;
- distorted HMMWV;
- distorted/misoriented BTR-4 plus white/default material artifact;
- incorrect M2 mount and inverted vertical aim;
- vehicle enter/exit possession paths teleporting vehicle/player back to Museum BASE;
- normal game opening windowed;
- high instantaneous FPS while the machine heated strongly.

High FPS in an invalid/black/incomplete scene does not pass performance acceptance.

## 3. Pass44 non-regression rules retained

Pass45 must preserve these Pass44 decisions:

- compact central Oster, approximately 960×940 m, from `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`;
- never restore the historical 2.4 km gameplay/tactical map extent;
- no implicit normal-game filler bots unless explicitly requested by Bots/Population/BotFill;
- Museum BASE proof uses the actual live pawn;
- tactical map derives from the compact central area;
- missing/default weapon materials remain visible content gaps; no BasicShape/grey disguise.

## 4. Merged corrective milestone — PR #83

PR #83 is **MERGED**. Source head `f89841ca9375ed5b8da496ec36e8c2efe2a8a437` completed full relevant current-head CI green before merge. Main merge commit is `f5e883fb69ae8bdd35c754dc895d8b06e4843e08`.

All merged behavior below remains `CODED_UNTESTED` until a newer factual local run accepts it.

### 4.1 Museum ownership

- R13.7 is the single visible Museum exterior owner.
- R13.8 owns hidden collision/interactivity + final breakable glass only.
- R14.0 is additive facade detail; it must not suppress R13.7.
- R14.5 owns the current Museum tree layout.
- obsolete R14.1 Museum window replacement is physically retired.
- old Pass35/Pass37 delayed/destructive Museum recovery owners are physically retired.
- Museum layer guard is validation-only: no `SetVisibility`, `SetHiddenInGame`, `RemoveInstance`, collision repair, or rebuild mutation.
- landmark separation is validation-only; no late duplicate cleanup may mask bad primary authoring.

Required current evidence includes:

- `PASS45_LANDMARK_STARTUP_COORDINATED_READY`
- `PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED`
- `PASS45_MUSEUM_R138_COLLISION_ONLY_READY`
- `PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY`
- `PASS45_MUSEUM_LAYER_VALIDATION_READY`

Any corresponding `...FAIL` remains fail-closed.

### 4.2 Spawn and vehicle possession

- initial Museum BASE validation is for an `AOCCharacter`, once per controller;
- character -> vehicle -> character possession changes are not deployments;
- no ordinary driver/gunner path may invoke Museum respawn correction;
- vehicle enter/exit evidence must show `museum_respawn_path=0`.

Current evidence:

- `PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE` or `PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE`;
- `vehicle_revalidation=0`;
- `PASS45_VEHICLE_ENTER_TRANSFORM_READY`;
- `PASS45_VEHICLE_EXIT_TRANSFORM_READY`;
- `PASS45_GUNNER_EXIT_TRANSFORM_READY`.

### 4.3 HMMWV / BTR-4 / M2

- production HMMWV/BTR meshes use uniform proportional scale, native long-axis correction and grounded bounds;
- no per-axis stretch to proxy dimensions;
- M2 placement uses mesh/mount bounds rather than arbitrary stretched proxy fitting;
- with Invert Y OFF, mouse up raises gunner pitch.

Current evidence:

- `PASS45_HMMWV_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`;
- `PASS45_BTR4_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`;
- `PASS45_M2_MOUNT_ALIGNMENT_READY`;
- `PASS45_M2_GUNNER_PITCH_CONTRACT_READY ... mouse_up_raises=1`.

### 4.4 Production materials

Primary rule: production meshes are never repainted by legacy BasicShape tint logic.

- `/Game/Production/` bypasses legacy vehicle tint at the primary source;
- production vehicle validator is one-shot/read-only;
- no `SetMaterial` or repair polling in that validator;
- missing/default/WorldGrid/BasicShape authored slots remain **CONTENT GAP / FAIL**, not fake READY.

Required markers:

- `PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY`;
- `PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY` only when HMMWV/M2/BTR are all present with authored non-placeholder materials and zero runtime override mismatch;
- `PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP` / `...CONTENT_GAP` remain valid rejection evidence.

### 4.5 Display and thermal policy

- normal game must not force `-windowed`;
- recovery route requests fullscreen;
- normal recovery cap is `t.MaxFPS 60`;
- render scale must not be silently reduced to manufacture FPS;
- >=30 FPS remains the acceptance floor;
- sudden FPS collapse or strong rapid heating is a rejection condition; exit the game immediately and preserve the log.

## 5. Post-merge content closure — PR #85

### 5.1 Generic residential replacement

Source diagnosis traced the mass rejected village scenery to `AOCAssetModelDecorator`:

- `AdvancedVillagePack` `SM_House_Var01/02` + extras;
- `AdvancedVillagePack` `SM_Fence_Var01..04`;
- `Modular_Rural_Cabin` `Side_Shed`;
- decorator also hid semantic `ResidentialRoofs` and `ResidentialDetails` before applying those replacements.

Current PR #85 correction:

- mass generic house replacement physically removed from the decorator;
- mass generic fence replacement physically removed;
- decorator Side_Shed placement physically removed;
- semantic `ResidentialRoofs` / `ResidentialDetails` are no longer hidden by the decorator;
- unrelated active vegetation/infrastructure/ambient paths remain;
- runtime marker:
  `PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED semantic_baseline=1 advanced_village_houses=0 village_fences=0 side_sheds=0 runtime_house_replacement=0`;
- Pass45 stale-retirement verifier forbids resurrection of those decorator paths;
- Oster world-model verifier has been forward-ported to the current ownership contract rather than demanding the rejected family.

Current PR #85 head checked after that forward-port has green aggregate `Source verification`, green `Oster world models source contracts`, green Pass45 stale retirement and green relevant Pass15–44 workflows.

This does **not** certify the semantic baseline as final Oster production art. It is a truthful fallback until reference-faithful residential content exists.

### 5.2 Remaining yard owner

`OCEnterableHouse` still separately contains:

- `RealYardFence` from `Fence_Old_1_2m`;
- `RealSideShed` from `Side_Shed`;
- a runtime `BuildYard()` path that places them externally.

This remaining owner is explicitly **IN PROGRESS**. It must be safely retired or replaced only with reference-supported Oster content. Do not claim the user-rejected dark tower/shack is fully removed until this path is closed and a local screenshot confirms the result.

### 5.3 BTR/weapon material gaps

Current production importer already requests BTR FBX materials/textures and stages local BTR textures. Runtime production validation correctly rejects placeholder/missing authored slots and does not repaint them.

Therefore:

- do not create fake BTR body color to hide missing material/texture dependencies;
- do not create fake weapon palette/material repair;
- improve reporting/preflight if needed to identify exact authored slot/texture dependencies;
- close BTR white artifact and rack material gaps only with factual UE asset/runtime evidence.

M16/M4 remain **CONTENT GAP** until a verified production payload exists. Never claim them connected from source naming alone.

## 6. Mandatory execution order from current state

1. [x] Archive and preserve latest 2026-08-25 rejected runtime evidence.
2. [x] Mark Pass45 `RUNTIME REJECTED`.
3. [x] Retire stale destructive Museum/world/palette owners.
4. [x] Consolidate Museum visible/collision ownership.
5. [x] Correct initial BASE validation vs vehicle possession.
6. [x] Add vehicle/gunner transform fail-visible evidence.
7. [x] Correct HMMWV/BTR proportional fitting.
8. [x] Correct M2 mount and default gunner pitch direction.
9. [x] Make production material ownership fail-visible/read-only.
10. [x] Apply fullscreen + 60 FPS thermal recovery policy without hidden render-scale downgrade.
11. [x] Forward-port historical verifiers instead of resurrecting rejected source behavior.
12. [x] Obtain full green current-head CI for PR #83.
13. [x] Merge PR #83 to `main` at `f5e883fb69ae8bdd35c754dc895d8b06e4843e08`.
14. [x] Trace mass rejected generic house/fence/Side_Shed owner to `AOCAssetModelDecorator`.
15. [x] Retire that mass generic decorator family and restore semantic residential roof/detail visibility.
16. [x] Forward-port `VERIFY_OSTER_WORLD_MODELS_PASS.py` to current truthful contract.
17. [x] Obtain green current-head source CI for the current PR #85 head checked here.
18. [ ] Safely close remaining `OCEnterableHouse` external `RealYardFence` / `RealSideShed` owner.
19. [ ] Add `PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED` to cumulative runtime acceptance requirements.
20. [ ] Keep BTR white/default and weapon authored material gaps fail-visible; close only with asset/runtime proof.
21. [ ] Refresh PR #85 final diff and run fresh final-head CI after all remaining source/docs changes.
22. [ ] Merge PR #85 only after final-head CI is fully green.
23. [ ] Pull final `main` locally and run `START_HERE.cmd -> 1. ЗВИЧАЙНА ГРА`.
24. [ ] Review factual runtime log/screenshots and update verdict to VERIFIED only if every required gate is actually proven.

## 7. Local runtime acceptance gates

A new local UE 5.8 test must prove all of the following:

### Launch/display/performance

- current source builds successfully;
- normal game reaches gameplay;
- fullscreen recovery behavior is correct;
- expected 60 FPS cap/thermal behavior is stable;
- gameplay sustains >=30 FPS;
- no severe thermal runaway or rapid FPS collapse.

### World/landmarks

- compact central Oster remains the authored/tactical-map extent;
- no black/invalid world or ground regions;
- Museum visually matches its Oster identity and is not replaced/confused by Culture House;
- Culture House and Silpo remain separate correct sites;
- no mass generic village house/fence/Side_Shed replacement scenery;
- no unreferenced dark tower/shack remains;
- semantic residential baseline is at least visible where the rejected mass replacement was retired.

### Spawn/input/map

- actual initial pawn appears within Museum BASE acceptance radius;
- BASE correction does not re-run on ordinary vehicle possession;
- WASD/mouse input is released correctly;
- tactical map `M` shows the compact central area and player marker correctly.

### Weapons/vehicles

- all 11 BASE rack pickups are grounded;
- white/default/BasicShape weapon slots are reported as failures, not disguised;
- HMMWV/BTR proportions and orientation are visually correct;
- BTR white/default material status is factual;
- M2 is correctly mounted;
- Invert Y OFF + mouse up raises M2 aim;
- driver and gunner enter/drive/exit preserve current vehicle position and never teleport to Museum;
- production vehicle materials are authored materials, with no runtime repaint repair.

## 8. Acceptance status vocabulary

Use only these meanings:

- **CODED_UNTESTED** — source/CI only;
- **CONTENT GAP** — required content missing or materially incomplete;
- **RUNTIME REJECTED** — factual local runtime disproved acceptance;
- **VERIFIED BUILD** — local UE build proves compile/import stage;
- **VERIFIED RUNTIME** — local UE gameplay + log/screenshots prove the required behavior/visual result.

Until the next local UE run, Pass45 remains **RUNTIME REJECTED / CODED_UNTESTED**, regardless of green GitHub CI.
