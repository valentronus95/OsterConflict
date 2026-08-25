# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest factual local UE runtime/user evidence overrides source claims and historical verifier assumptions.

## 1. Current context — 2026-08-25

- Repository: `valentronus95/OsterConflict`.
- UE target: 5.8.x Windows.
- User launcher: `START_HERE.cmd` only.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Pass 45 corrective PR #83: **MERGED**.
- PR #83 source head: `f89841ca9375ed5b8da496ec36e8c2efe2a8a437`.
- PR #83 merge on `main`: `f5e883fb69ae8bdd35c754dc895d8b06e4843e08`.
- PR #83 current-head CI: **GREEN** across all 39 PR-triggered workflows at final source head.
- Active post-merge closure branch: `fix/pass45-postmerge-content-closure-20260825`.
- Active follow-up PR: #85.
- Latest factual evidence remains `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`.
- Historical Pass44 rejection remains `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- Current status token: **PASS 45 ACTIVE / PR #83 MERGED + FINAL PR HEAD SOURCE CI GREEN / PR #85 POST-MERGE CONTENT CLOSURE CODED_UNTESTED / LATEST LOCAL UE RUNTIME REJECTED 2026-08-25**.

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure incomplete.
- `CODED_UNTESTED` — source correction exists but factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `RUNTIME REJECTED` — factual local gameplay disproved the result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds.
- `VERIFIED RUNTIME` — factual local UE/user playtest proves behavior/appearance.
- Green source CI is not UE compile/runtime acceptance.
- Mesh-load success is weaker than authored material/texture truth.
- Historical verifiers may not resurrect runtime-rejected owners or behavior.

## 3. Latest authoritative runtime — 2026-08-25

Pass 45 remains **RUNTIME REJECTED** until a newer factual local run supersedes this evidence. Latest rejected runtime showed:

- large black world/ground regions;
- multiple rack weapons white/default while AK-47 appeared materially correct;
- generic village houses/fences not matching Oster references;
- Museum identity unclear/wrong, with a Culture-House-like six-column facade dominating the Museum test view;
- an unreferenced dark steep-roof tower/shack;
- HMMWV non-uniform deformation;
- BTR-4 deformation/orientation errors plus white/default material artifact;
- M2 mount transform error and inverted vertical aim;
- vehicle possession/exit could teleport vehicle/player back to Museum BASE;
- forced windowed normal route;
- roughly 100–156 FPS while the machine heated strongly.

High FPS in a visually broken/black scene is not acceptance.

### Historical local UE build/import rejection retained

**LOCAL UE BUILD REJECTED — 2026-08-25** is retained as factual history even though later source fixes exist:

- MSVC / UE 5.8 rejected the tactical reference-road table with `C2131` while `FVector2D` data were declared `constexpr`; current source uses a normal `const` table and that correction remains `CODED_UNTESTED` until a later factual local build verifies it.
- UE 5.8 Interchange rejected the deprecated `auto_detect_mesh_type` property during HMMWV/M2 GLB intake; current importer explicitly forces StaticMesh and disables skeletal import without `auto_detect_mesh_type`, but that correction also remains `CODED_UNTESTED` until a later factual local import verifies it.

These failures are historical evidence and must not disappear merely because source CI later becomes green.

### Historical Pass44 rejection retained

Pass44 was also factually rejected. Its retained non-regression decisions are:

- compact central Oster roughly 960×940 m; never restore the historical 2.4 km battlefield;
- zero implicit normal-game filler bots unless explicitly requested;
- Museum BASE acceptance uses the actual live pawn;
- tactical map follows compact central-Oster bounds;
- grey/BasicShape weapon material repair remains forbidden.

## 4. Merged Pass45 corrective source milestone — PR #83

All items below are merged but remain `CODED_UNTESTED` until local UE acceptance.

### 4.1 Retired stale owners

Physically retired/replaced where appropriate:

- rejected B2 world production visual owner;
- Pass35 Museum recovery/replay owner;
- Pass37 destructive Museum visibility/rebuild owner;
- late landmark duplicate-destroy ownership guard;
- obsolete R14.1 Museum window replacement owner;
- retired weapon palette compatibility owner;
- stale completion/patch workflows that required rejected behavior.

### 4.2 Museum ownership

- R13.7 is the single visible Museum exterior owner.
- R13.8 owns hidden collision/interactivity + final breakable glass only.
- R14.0 is additive final facade detail and does not suppress R13.7.
- R14.5 owns current Museum tree layout.
- Museum layer guard is validation-only with `mutation=0`.
- landmark separation is validation-only; no late Destroy/RemoveInstance cleanup.
- success/fail evidence is explicit through current `PASS45_MUSEUM_*` and landmark validation markers.

### 4.3 Spawn/vehicle possession

- Museum BASE correction validates an initial `AOCCharacter` once per controller.
- ordinary vehicle possession/unpossession is not deployment evidence.
- current markers include `PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE` / `PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE` with `vehicle_revalidation=0`.
- vehicle/gunner enter/exit telemetry must prove `museum_respawn_path=0`.

### 4.4 HMMWV / BTR-4 / M2

- HMMWV/BTR use uniform proportional scale + native long-axis correction + grounded bounds.
- M2 uses bottom-of-bounds mount alignment.
- Invert Y OFF + mouse up must raise M2 aim.
- production vehicle material validation is read-only; runtime recolor repair is forbidden.

### 4.5 Display/thermal

- normal route no longer forces `-windowed`;
- fullscreen recovery requested;
- normal recovery cap `t.MaxFPS 60`;
- render scale is not silently reduced.

## 5. Post-merge Pass45 content closure — PR #85

Current branch source work:

- mass `AdvancedVillagePack` residential house replacement retired from `AOCAssetModelDecorator`;
- mass `SM_Fence_Var01..04` residential fence replacement retired;
- decorator `Modular_Rural_Cabin` `Side_Shed` placement retired;
- decorator no longer hides semantic `ResidentialRoofs` / `ResidentialDetails` baseline;
- retained unrelated vegetation/infrastructure/ambient model paths;
- runtime evidence marker added:
  `PASS45_GENERIC_RESIDENTIAL_REPLACEMENT_RETIRED semantic_baseline=1 advanced_village_houses=0 village_fences=0 side_sheds=0 runtime_house_replacement=0`;
- separate `OCEnterableHouse` `RealYardFence` / `RealSideShed` owner is now also retired; semantic yard fence boundary, gate and path remain;
- `PASS45_ENTERABLE_HOUSE_YARD_REFERENCE_GUARD_READY semantic_fence_baseline=1 real_yard_fence=0 side_shed=0 unreferenced_shed=0` records that owner when an enterable-house actor is present;
- cumulative `RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd` now requires the global generic-residential retirement marker;
- `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` and `VERIFY_PASS45_CONTENT_DEPENDENCIES.py` block resurrection of rejected generic exterior replacements;
- BTR fresh-load validation now requires a real imported Material -> Texture dependency under `/Game/Production/Vehicles/BTR4`; a non-placeholder material name alone cannot certify the white/default artifact as fixed;
- BTR dependency success marker is `BTR4_TEXTURE_DEPENDENCIES_READY ... white_default_guard=1`;
- dedicated `Pass 45 content dependency closure` workflow validates the new source gate and UE Python syntax.

Current limitation remains factual: source can prove that rejected owners are removed and that the BTR intake gate is stricter, but only a local UE import/runtime screenshot can prove the dark tower/shack and white BTR artifacts are visually gone.

## 6. Active requirements

| ID | Requirement | Status | Current action |
|---|---|---|---|
| STALE-OWNER-001 | Old code/verifiers must not resurrect rejected behavior | CODED_UNTESTED | Retirement gates active; final PR #85 CI pending. |
| PERF-COLLAPSE-001 | No severe FPS/thermal collapse | CODED_UNTESTED | Fullscreen + 60 FPS recovery cap; local runtime required. |
| VIS-BLACK-WORLD-001 | No black ground/world corruption | CODED_UNTESTED | Rejected B2 visual owner deleted; retest required. |
| LOC-MUSEUM-001 | Correct visible Oster Local History Museum near BASE | CODED_UNTESTED | R13.7 visible owner; R13.8 collision/interactivity only. |
| LOC-CULTURE-001 | Culture House separate from Museum | CODED_UNTESTED | Separate owner/validation; screenshot required. |
| LOC-SILPO-001 | Silpo one correct site owner | CODED_UNTESTED | Current specific owner retained; screenshot required. |
| LOC-TOWER-001 | Remove unreferenced dark tower/shack | CODED_UNTESTED | Decorator + EnterableHouse `Side_Shed` owners retired; runtime screenshot still required. |
| VIS-GENERIC-RESIDENTIAL-001 | No rejected generic village house/fence replacement | CODED_UNTESTED | Mass decorator + enterable-house exterior replacements retired; semantic baseline retained. |
| GAME-SPAWN-001 | Initial live pawn near Museum BASE | CODED_UNTESTED | Initial-character-only validation. |
| GAME-VEHICLE-TELEPORT-001 | Enter/drive/exit never teleports to Museum | CODED_UNTESTED | Possession-driven BASE cause removed; local test required. |
| VEH-HMMWV-001 | HMMWV proportions/orientation correct | CODED_UNTESTED | Uniform scale + axis correction. |
| VEH-BTR-001 | BTR-4 proportions/orientation/material correct | CODED_UNTESTED / CONTENT CHECK | Uniform transform retained; fresh-load now rejects missing BTR Material->Texture dependency; local import/runtime required. |
| VEH-M2-MOUNT-001 | M2 correctly aligned | CODED_UNTESTED | Bottom-on-mount alignment. |
| VEH-M2-PITCH-001 | Mouse up raises M2 with Invert Y OFF | CODED_UNTESTED | Source corrected; runtime proof required. |
| VEH-MATERIAL-OWNER-001 | No production asset repaint | CODED_UNTESTED | Primary bypass + read-only validation. |
| WEAPON-MATERIAL-001 | Rack weapons have authored materials/textures | CODED_UNTESTED / CONTENT CHECK | Missing/default slots fail visibly; factual local asset evidence still required. |
| GAME-WEAPONS-001 | 11 grounded pickups near Museum BASE | CODED_UNTESTED | Grounding contract retained. |
| UI-TACTICAL-MAP-001 | `M` matches compact central Oster | CODED_UNTESTED | Runtime screenshot required. |
| MAP-EXTENT-001 | Keep compact central Oster | CODED_UNTESTED / RETAIN | 960×940 m hard extent retained. |
| VIS-TREES-001 | No primitive/fantasy forest | CODED_UNTESTED / CONTENT GAP | Current real-tree paths retained; oak still unverified. |
| ASSET-M16-M4-001 | M16/M4 production visuals | CONTENT GAP | No verified payload; never claim connected. |

## 7. Behavior that must not return

1. rejected B2 world visual owner or renamed equivalent;
2. Pass35/Pass37 delayed/destructive Museum recovery layers;
3. late landmark duplicate-destroy repair;
4. weapon palette mutation/grey disguise;
5. vehicle possession treated as BASE deployment;
6. non-uniform production HMMWV/BTR fitting;
7. forced normal `-windowed` launch;
8. uncapped normal recovery route;
9. implicit normal-game bot autofill;
10. historical 2.4 km gameplay/tactical map;
11. late Museum/landmark visibility/collision/instance repair;
12. production vehicle material repair hiding authored-content defects;
13. mass generic `AdvancedVillagePack` residential house/fence replacement in `AOCAssetModelDecorator`;
14. decorator or enterable-house `Side_Shed` replacement;
15. unreferenced rural-cabin yard fence replacing the semantic yard boundary;
16. BTR fresh-load success based only on a non-placeholder material asset with no imported texture dependency.

## 8. Current execution order

1. [x] Archive latest rejected runtime evidence.
2. [x] Mark Pass45 factual verdict `RUNTIME REJECTED`.
3. [x] Retire stale destructive world/Museum/palette owners.
4. [x] Consolidate Museum visible/collision ownership.
5. [x] Correct initial BASE validation and vehicle-possession teleport cause.
6. [x] Correct HMMWV/BTR proportional fitting, M2 mount and M2 pitch direction.
7. [x] Stop production material repaint and use validation-only evidence.
8. [x] Apply fullscreen/60 FPS thermal recovery policy.
9. [x] Forward-port historical verifier contracts.
10. [x] PR #83 final source head passed all 39 PR-triggered workflows.
11. [x] Merge PR #83 to `main` at `f5e883fb69ae8bdd35c754dc895d8b06e4843e08`.
12. [x] Trace and retire mass generic residential house/fence/Side_Shed owner in `AOCAssetModelDecorator`.
13. [x] Restore semantic residential roof/detail baseline.
14. [x] Trace remaining `OCEnterableHouse` `RealYardFence` / `RealSideShed` owner.
15. [x] Retire those unreferenced enterable-house yard replacements while preserving semantic boundary/gate/path.
16. [x] Add generic-residential retirement marker to cumulative runtime acceptance.
17. [x] Add fail-closed BTR Material->Texture fresh-load dependency audit and dedicated content-dependency CI.
18. [x] Preserve factual `LOCAL UE BUILD REJECTED` history including `C2131` and `auto_detect_mesh_type` regressions.
19. [ ] Obtain fresh final-head PR #85 CI after source/docs closure.
20. [ ] Merge PR #85 only after final-head CI is green.
21. [ ] Run local `START_HERE.cmd -> 1. ЗВИЧАЙНА ГРА` factual UE 5.8 acceptance.

## 9. Next factual runtime gates

A new local run must prove, not merely imply:

- successful UE build and normal launch;
- fullscreen recovery and expected 60 FPS cap behavior;
- actual initial pawn near Museum BASE;
- correct Museum identity and separation from Culture House/Silpo;
- no rejected generic mass village houses/fences/Side_Shed scenery;
- no remaining unreferenced dark tower/shack;
- compact tactical map;
- 11 grounded rack weapons and truthful authored material status;
- BTR local fresh-load emits `BTR4_TEXTURE_DEPENDENCIES_READY` if BTR production content is present; otherwise the content gap remains explicit;
- HMMWV/BTR/M2 proportions/material status;
- vehicle driver/gunner enter/drive/exit with no Museum teleport;
- M2 mouse-up raises aim with Invert Y OFF;
- sustained gameplay >=30 FPS without severe thermal runaway.

If FPS collapses or the machine rapidly overheats, exit immediately. That failure is authoritative runtime evidence.