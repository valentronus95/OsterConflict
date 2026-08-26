# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state with only the historical rejection/build facts that must remain explicit.
> Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

## 1. Current context — 2026-08-26

- Repository: `valentronus95/OsterConflict`.
- Current `main` baseline: `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f` (PR #93 merged; current R14 verifier forward-ported without claiming runtime acceptance).
- Active corrective branch: `fix/pass45-weapon-material-closure-20260826`, branched from current `main` specifically for Pass45 item 12 material/content closure.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Latest factual evidence: `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`.
- Previous rejected evidence: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x Windows.
- User launcher: `START_HERE.cmd` only.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Current status token: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-25 / PR #93 IN MAIN / WEAPON MATERIAL CLOSURE CODED_UNTESTED / RUNTIME PENDING**.

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure is incomplete.
- `CODED_UNTESTED` — source correction exists but factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `RUNTIME REJECTED` — factual local gameplay disproved the result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds.
- `VERIFIED RUNTIME` — factual local UE/user playtest proves the behavior/appearance.
- Green source CI is not UE compile/runtime acceptance.
- Mesh-load success is weaker than authored material/texture truth.
- A historical verifier may never force a rejected owner/behavior back into current source.

## 3. Latest authoritative runtime — 2026-08-25

The current factual gameplay reaches runtime but **Pass 45 is rejected**. Latest screenshots/user observation prove:

- large world/ground areas rendered black;
- multiple rack weapons remained white/default while AK-47 appeared materially correct;
- generic village fences/houses did not match Oster references;
- Museum identity was not clear/correct; a six-column Culture-House-like facade dominated the Museum test view;
- an unreferenced dark steep-roof tower/shack was visible;
- HMMWV was non-uniformly deformed;
- BTR-4 was non-uniformly deformed/oriented incorrectly and had a white/default material artifact;
- M2 Browning mount transform was visibly wrong;
- mounted vertical aim was reported inverted;
- vehicle possession/exit could teleport vehicle/player back to Museum BASE;
- normal route opened forced windowed;
- runtime reached roughly 100–156 FPS while the machine heated strongly;
- high FPS in a visually broken/black scene is not acceptance.

**Pass 45 factual verdict: RUNTIME REJECTED.**

### Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass44 as a complete solution: spawn/result framing was wrong, the map was still perceived as excessively large/empty, weapon visuals/materials were not production-ready, production-model claims were unreliable, and FPS could collapse severely. Pass45 supersedes Pass44 as the active corrective pass; this historical rejection may not be erased by later source fixes.

### Pass 44 behavior retained unless disproved

The following Pass44 decisions remain protected as non-regression because later evidence did not invalidate them:

- compact central-Oster hard extent: approximately 960×940 m, never restore the historical 2.4 km battlefield;
- normal local gameplay defaults to zero implicit filler bots unless explicitly requested;
- Museum BASE acceptance must be based on the actual live pawn, not source-only spawnpoint existence;
- tactical-map bounds follow the compact central-Oster reference rather than legacy peripheral component auto-fit;
- grey/BasicShape weapon material repair is forbidden; authored material gaps remain fail-visible;
- the retired Pass37 weapon palette compatibility owner stays physically deleted, not preserved as an inert shell.

Pass44 historical non-regression does **not** authorize resurrection of any owner/repair path that Pass45 physically retired.

## 4. Confirmed corrective source work on active branch

All items below remain **CODED_UNTESTED** until local UE acceptance.

### 4.1 Stale runtime owner retirement

Physically deleted because they were runtime-rejected, destructive late owners, or inert compatibility shells:

- `OCWorldProductionVisualsSubsystem.h/.cpp` — rejected B2 black/generic world visual owner;
- `OCMuseumCoreRecoverySubsystem.h/.cpp` — delayed recovery carrier + R13.8 rebuild/detail replay owner;
- `OCMuseumVisibilityPass37Subsystem.h/.cpp` — late polling/destructive architecture rebuild/visibility owner;
- `OCLandmarkShellOwnershipGuardSubsystem.h/.cpp` — late duplicate-destroy owner with stale R13.7/R13.8 shell semantics;
- `OCR137MuseumSiteReplacementSubsystem.h/.cpp` — retired compatibility shell;
- `OCR13MuseumStadiumPhotoFidelitySubsystem.h/.cpp` — retired compatibility shell;
- `OCWeaponPalettePass37Subsystem.h/.cpp` — retired palette compatibility owner, physically absent;
- obsolete R14.1 Museum window replacement owner — physically deleted;
- `VERIFY_PASS45_COMPLETION_AUDIT.py` + `.github/workflows/pass45-completion-audit.yml` — stale CI contract that required a rejected owner back.

Replacement/guardrails:

- root `AGENTS.md` requires physical retirement when a rejected legacy mutating owner has no required data/collision role;
- `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` + workflow fail if retired owners are resurrected;
- historical verifiers are forward-ported to current behavior instead of forcing old READY markers/owners back.

### 4.2 Museum / landmark ownership

Current source contract:

- `OCLandmarkStartupCoordinatorSubsystem` owns the single startup orchestration window;
- historical stage timers are cancelled before current authoritative stage calls;
- R13.7 is the **single visible Museum exterior owner**;
- R13.8 owns **hidden collision/interactivity + final breakable glass**, not a competing visible shell;
- R14.0 no longer suppresses/removes R13.7 exterior content;
- R14.5 owns the current Museum tree layout;
- `OCMuseumLayerPerformanceGuardSubsystem` is **validation-only**, one delayed observation, `mutation=0`;
- layer failures emit `PASS45_MUSEUM_LAYER_VALIDATION_FAIL ... primary_authoring_fix_required=1`, never late repair;
- success emits `PASS45_MUSEUM_LAYER_VALIDATION_READY`;
- `OCR146LandmarkSeparationSubsystem` is validation-only: no `Destroy()`, no `RemoveInstance()`, no actor-spawn repair.

Runtime Museum identity, Culture House separation, Silpo separation and tree quality still require factual screenshots/playtest.

### 4.3 Vehicle BASE teleport correction

Exact root cause found: Museum BASE guard previously treated every newly possessed `APawn` as a new BASE deployment. `character -> vehicle -> character` possession transitions could therefore trigger Museum correction.

Current source:

- validates only `AOCCharacter`;
- validates at most once per player controller;
- vehicle pawn is never BASE deployment evidence;
- markers: `PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE` / `PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE`;
- marker field `vehicle_revalidation=0` is mandatory;
- focused and full runtime acceptance accept either factual initial terminal result but reject `PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL`.

Runtime vehicle enter/drive/exit still requires factual test.

### 4.4 HMMWV / BTR / M2 transforms and material ownership

- HMMWV production mesh: independent X/Y/Z fitting removed; uniform scale + native long-axis correction + grounded bounds.
- BTR-4 production mesh: same proportional correction.
- markers: `PASS45_HMMWV_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`, `PASS45_BTR4_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`.
- M2 visual: uniform fit + bottom-of-bounds to mount plane; marker `PASS45_M2_MOUNT_ALIGNMENT_READY ... bottom_on_mount=1`.
- default M2 gunner pitch direction corrected: Invert Y OFF + mouse up must raise aim; marker `PASS45_M2_GUNNER_PITCH_CONTRACT_READY`.
- `AOCVehicleBase` now bypasses legacy BasicShape tint for `/Game/Production/` components at the primary source.
- former production visual repair guard is read-only validation-only: no material mutation and no polling repair loop.
- vehicle enter/exit transform telemetry records the normal path and must prove `museum_respawn_path=0`.

The repository-safe BTR-4 fallback now carries the explicit authored `M_BTR4_OC_Authored` PBR material contract from PR #87. The previously observed white/default BTR artifact nevertheless remains a **runtime acceptance gap** until local UE import/render proves that authored slot is what actually reaches the vehicle.

### 4.5 Display / thermal recovery

- normal route no longer forces `-windowed`;
- recovery route requests fullscreen;
- normal recovery cap: `t.MaxFPS 60`;
- render scale is not lowered;
- marker: `PASS45_NORMAL_DISPLAY_THERMAL_GUARD fullscreen=1 max_fps=60 render_scale_mutation=0`.

### 4.6 Runtime acceptance scripts

`RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd` is current cumulative acceptance:

- deleted Museum rebuild/palette markers are not required;
- requires coordinated landmark startup and validation-only Museum ownership;
- requires proportional HMMWV/BTR and M2 mount evidence;
- requires current actual-pawn/compact-map/rack/material/FPS evidence;
- vehicle teleport and M2 pitch direction remain mandatory runtime gates;
- sampled gameplay floor remains >=30 FPS.

`RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd` is forward-ported to current initial-only Museum deployment markers and may not treat vehicle possession as a new BASE deployment.

### 4.7 Reference-driven residential visual retirement — 2026-08-26

Current status: **CODED_UNTESTED**.

- normal `OCGameMode` no longer spawns the gameplay-authored/non-reference-specific `AOCEnterableHouse`;
- `AOCWorldSectorOster` no longer owns `BuildResidentialBlocks()` procedural houses/sheds/private fences;
- the generic Krushelnytska house/shed/fence generator is physically removed while `BuildRoadNetwork()` retains road topology;
- Museum/Stadium/College reference-driven fence geometry remains;
- runtime markers: `PASS45_GENERIC_ENTERABLE_HOUSE_RETIRED` and `PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED`;
- `VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` + workflow guard against resurrection;
- old Pass11 road verifier was forward-ported when CI exposed stale dependencies on already-deleted `OCAssetModelDecorator` / `OCRecoveredRoadsidePropsSubsystem`; those deleted owners are not restored.

Runtime still must prove the rejected dark tower/shack and generic visual family are absent.

### 4.8 Weapon authored-material closure continuation — 2026-08-26

Current status: **CODED_UNTESTED / UE EDITOR REIMPORT + RUNTIME VISUAL PENDING**.

- active branch: `fix/pass45-weapon-material-closure-20260826`;
- source audit found that committed Stein weapon folders contain authored external PNG texture sources (`T_*`), while the already imported `/Game/R13/Weapons/Stein/*` destinations did not reliably contain those texture assets;
- `OsterConflict/Scripts/pass45_reimport_stein_weapon_materials.py` now explicitly imports committed PNG textures first, then reimports each current Stein runtime FBX with materials/textures enabled;
- the corrective importer rejects missing/BasicShape/Default/WorldGrid/`_defaultMat` material slots and rejects a material chain with zero used textures or no local weapon texture dependency;
- `OsterConflict/PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd` runs that import through UE 5.8 Python commandlet and writes only `STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING`, never a runtime READY claim;
- `VALIDATE_PRODUCTION_MODELS_UE58.cmd` now runs the corrective Stein reimport before the weapon runtime report and gates every canonical weapon whose exact repository content exists;
- absent Remington870/M249 production payload remains explicit `CONTENT GAP`, is printed as not READY, and is no longer conflated with a white/default-material failure on available content;
- `VERIFY_PASS45_WEAPON_MATERIAL_DEPENDENCY_AUDIT.py` is now part of `RUN_ALL_VERIFY.py` and guards texture-first ordering, placeholder rejection, current source payload visibility, UE wrapper wiring and available-vs-content-gap acceptance semantics.

This does **not** close runtime Gate F. A rendered local UE rack screenshot must still prove that every available required weapon is visibly authored rather than white/default.

## 5. Active requirements

| ID | Requirement | Repeat | Status | Current action |
|---|---|---:|---|---|
| STALE-OWNER-001 | Old code/verifiers must not overwrite or resurrect newer runtime behavior | 1 | CODED_UNTESTED | PR #93 is current `main`; physical-retirement and forward-ported verifier guards remain authoritative. |
| PERF-COLLAPSE-001 | No severe FPS/thermal collapse | ≥7 | CODED_UNTESTED | Fullscreen + 60 FPS recovery cap; destructive Museum loops removed; local runtime required. |
| VIS-BLACK-WORLD-001 | No black ground/world corruption | 1 | CODED_UNTESTED | Runtime-rejected B2 visual owner deleted; readable baseline must be retested. |
| LOC-MUSEUM-001 | Correct visible Oster Local History Museum near BASE | ≥11 | CODED_UNTESTED | R13.7 single visible exterior; R13.8 hidden collision/interactivity/glass; Pass32 validation-only. |
| LOC-CULTURE-001 | Culture House separate from Museum | ≥3 | IN_PROGRESS / CODED_UNTESTED | R14.6 separate geo anchor; validation-only parcel check. |
| LOC-SILPO-001 | Silpo one correct site owner | ≥2 | CODED_UNTESTED | Coordinated startup retained; parcel overlap fail-visible. |
| LOC-TOWER-001 | Remove unreferenced dark steep-roof tower/shack | 1 | CODED_UNTESTED / RUNTIME CHECK | Pass35 roof/recovery owner plus traced generic residential source owners are removed; next runtime must prove the artifact is absent. |
| VIS-GENERIC-RESIDENTIAL-001 | No unreferenced generic house/private-fence family in normal runtime | 1 | CODED_UNTESTED | `AOCEnterableHouse` normal spawn, `BuildResidentialBlocks`, and generic Krushelnytska house/fence generator retired; reference-driven POI fences retained. |
| GAME-SPAWN-001 | Initial live pawn spawns near Museum BASE | ≥10 | CODED_UNTESTED | Initial-character-only validation/correction; no vehicle revalidation. |
| GAME-VEHICLE-TELEPORT-001 | Enter/drive/exit never teleports back to Museum | 1 | CODED_UNTESTED | Possession-triggered BASE cause removed; local car/HMMWV/BTR test required. |
| VEH-HMMWV-001 | HMMWV proportions/orientation correct | ≥6 | CODED_UNTESTED | Uniform scale + axis correction; runtime multi-angle screenshot required. |
| VEH-BTR-001 | BTR-4 proportions/orientation correct | ≥6 | CODED_UNTESTED | Uniform scale + axis correction; authored BTR fallback material source contract exists, but white runtime artifact still requires factual rerun. |
| VEH-M2-MOUNT-001 | M2 aligned on HMMWV mount | ≥2 | CODED_UNTESTED | Bottom-on-mount bounds alignment coded. |
| VEH-M2-PITCH-001 | Mouse up raises M2 when Invert Y OFF | 1 | CODED_UNTESTED | Source contract corrected; runtime input proof required. |
| VEH-MATERIAL-OWNER-001 | Base vehicle tint must not overwrite production authored materials | 1 | CODED_UNTESTED | VehicleBase bypasses `/Game/Production/`; validation guard is read-only. |
| WEAPON-MATERIAL-001 | Required rack weapons use authored material + texture dependencies | ≥11 | CODED_UNTESTED / CONTENT CHECK | Texture-first Stein reimport + used-texture validation coded; available weapons remain hard-fail on white/default; Remington870/M249 exact payload remains explicit CONTENT GAP. |
| GAME-WEAPONS-001 | 11 grounded pickups near Museum BASE | ≥9 | CODED_UNTESTED | 12 cm ground clearance retained. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | ≥4 | CODED_UNTESTED | Hard reference topology retained; runtime screenshot required. |
| MAP-EXTENT-001 | Keep compact central Oster battlefield | ≥2 | CODED_UNTESTED / RETAIN | 960×940 m hard extent retained; never restore 2.4 km world. |
| VIS-TREES-001 | No primitive/fantasy visible tree forest | ≥2 | CODED_UNTESTED / CONTENT GAP | Primitive proxy retirement retained; real pines known; oak unverified. |
| ASSET-M16-M4-001 | M16/M4 production visuals | ≥2 | CONTENT GAP | No verified payload; do not claim connected. |

## 6. Behavior that must not return

1. runtime-rejected `OCWorldProductionVisualsSubsystem` or equivalent renamed behavior;
2. Pass35 Museum recovery carrier/replay layer;
3. Pass37 destructive Museum visibility rebuild/polling layer;
4. late duplicate-destroy landmark ownership guard;
5. retired weapon palette mutation/no-op compatibility owner;
6. verifier/workflow that requires any retired owner/READY marker;
7. ordinary vehicle possession/unpossession treated as BASE deployment;
8. non-uniform X/Y/Z fitting of production HMMWV/BTR meshes;
9. forced normal `-windowed` launch;
10. uncapped recovery normal route;
11. implicit normal-game bot autofill;
12. historical 2.4 km gameplay/tactical map;
13. grey/BasicShape weapon material repair;
14. late landmark `Destroy/RemoveInstance` cleanup used to hide primary-authoring errors;
15. late Museum layer `SetVisibility/RemoveInstance/collision` repair;
16. production vehicle material repair that hides a primary material-owner bug;
17. procedural `BuildResidentialBlocks`, generic Krushelnytska house/fence generation, or normal spawn of a non-reference-specific `AOCEnterableHouse`.

## 7. Current execution order

1. [x] Latest screenshots archived; Pass45 marked `RUNTIME REJECTED`.
2. [x] Canonical TZ updated for latest defects.
3. [x] Runtime-rejected B2 world visual owner physically deleted.
4. [x] Stale B2 completion verifier/workflow deleted and replaced with retirement gate.
5. [x] Pass35 CoreRecovery and Pass37 Visibility rebuild owners physically deleted.
6. [x] Inert Museum site / museum-stadium / weapon palette compatibility shells physically deleted.
7. [x] Late landmark duplicate-destroy ownership guard physically deleted.
8. [x] Landmark separation converted from late mutation to validation-only.
9. [x] Museum BASE guard converted to initial-character-only deployment validation.
10. [x] HMMWV/BTR proportional fitting + M2 bottom-on-mount alignment coded.
11. [x] Normal forced windowed removed; 60 FPS recovery cap coded.
12. [x] Museum visual ownership consolidated: R13.7 visible exterior; R13.8 hidden interaction/collision + final glass; obsolete R14.1 owner deleted.
13. [x] Pass32 Museum layer guard converted to validation-only; late scene mutation retired.
14. [x] Default M2 gunner pitch direction corrected in source.
15. [x] `AOCVehicleBase` legacy tint stopped from touching production assets; production guard validation-only.
16. [x] Pass15/33/35/37/40/41/42/44/45 and related historical contracts forward-ported away from retired behavior as discovered.
17. [ ] Close BTR white material artifact and remaining weapon authored material/texture gaps supported by current content. Texture-first Stein reimport and BTR authored source material contract are coded; UE editor reimport + rendered runtime remain pending.
18. [x] Source-trace and retire remaining generic residential house/fence owners; dark tower/shack absence remains a factual runtime gate.
19. [x] Full PR #91 current-head source CI completed green; S07/S08/S09/S16A/S16B and Pass11 stale contracts were forward-ported instead of restoring rejected behavior.
20. [x] PR #91 scope/status recorded as `CODED_UNTESTED`; post-merge truth synchronized separately.
21. [x] PR #91 merged only after current-head source CI green -> `main` `c4712144efede68b3d80475bec64ea9c8e400fc4`.
22. [x] PR #93 forward-ported current R14 verifier without resurrecting retired landmark assumptions -> `main` `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f`; runtime remained pending.
23. [ ] Local `START_HERE.cmd -> 1. ЗВИЧАЙНА ГРА` factual UE build/runtime acceptance.

## 8. Next factual runtime gates

### Build
- UBT/UE 5.8 build exits 0.

### World / landmarks
- no large black world/ground regions;
- no late scenery pop/rebuild caused by deleted recovery owners;
- Museum visually matches Museum identity, not Culture House;
- Culture House and Silpo visibly separate;
- `PASS45_LANDMARK_STARTUP_COORDINATED_READY`;
- `PASS45_MUSEUM_LAYER_VALIDATION_READY` with `mutation=0`;
- `PASS45_LANDMARK_SEPARATION_VALIDATION_READY` with zero generic parcel overlap.

### Spawn / vehicles
- actual initial pawn near Museum BASE;
- civilian vehicle enter does not move vehicle to Museum;
- drive away, exit beside current vehicle transform;
- repeat HMMWV/BTR;
- enter/exit evidence shows `museum_respawn_path=0`;
- HMMWV/BTR proportions correct from multiple sides;
- M2 mount aligned and default vertical control non-inverted;
- no white BTR material artifact.

### Weapons
- all available required rack weapons show authored materials/textures;
- white/default/BasicShape slot = FAIL;
- unresolved assets remain explicit CONTENT GAP.

### Performance / display
- intended fullscreen recovery route;
- cap approximately 60 FPS;
- >=30 FPS minimum;
- no severe progressive heat/FPS collapse;
- no render-scale downgrade.

### Tactical map
- compact central Oster topology, north-up, player marker visible, Museum/Culture/Silpo/Stadium distinct.

**Current overall status: PASS 45 ACTIVE / RUNTIME REJECTED / WEAPON MATERIAL CLOSURE CODED_UNTESTED / ACTIVE BRANCH NOT MERGED.**

## 9. Pass45 corrective milestone — 2026-08-25 Museum/vehicle ownership

Status: **CODED_UNTESTED / latest factual runtime remains RUNTIME REJECTED until a new local UE 5.8 run**.

- Branch: `fix/pass45-runtime-rejection-20260825`.
- Museum visible ownership collapsed to R13.7; R13.8 is hidden collision/interactivity + final breakable glass only.
- obsolete R14.1 Museum window replacement header/source deleted; coordinator no longer invokes it.
- R14.0 late R13.7 suppression/instance removal retired; R14.5 owns the only current Museum tree layout.
- Pass32 Museum layer guard is validation-only and cannot mutate the scene.
- production VehicleBase no longer applies legacy BasicShape tint to `/Game/Production/`; validation guard is read-only.
- M2 gunner default pitch source corrected: invert off => mouse up raises aim.
- driver/gunner vehicle enter-exit transform telemetry added; normal path records `museum_respawn_path=0`.
- Work report: `OsterConflict/Docs/WorkReports/PASS45_RUNTIME_RECOVERY_CORRECTIVE_2026-08-25_MUSEUM_VEHICLE.md`.

Remaining factual acceptance: local UE build, Museum screenshot, vehicle drive/exit away from Museum, M2 pitch, HMMWV/M2/BTR materials/proportions, weapon material gaps, invalid Oster generic visuals, thermal/FPS behavior.

## 10. Pass45 preserved local UE build/import rejection — 2026-08-25

Status of this historical attempt: **LOCAL UE BUILD REJECTED**. This is preserved as factual chronology even though a later corrected build reached gameplay.

- UE 5.8/MSVC rejected the tactical-map `FVector2D` road table with **C2131** while it was `constexpr`; source now uses a normal `const` table.
- UE 5.8 Interchange rejected the deprecated `auto_detect_mesh_type` property during HMMWV/M2 GLB intake; current importer explicitly forces StaticMesh through the supported Interchange API.
- These corrections do not erase the failed attempt.
- A later run reaching gameplay does not erase the earlier build/import rejection or the newer Pass45 runtime rejection.

**Current truth remains: PASS 45 ACTIVE / RUNTIME REJECTED / WEAPON MATERIAL CLOSURE CODED_UNTESTED / ACTIVE BRANCH NOT MERGED / RUNTIME PENDING.**
