# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state only. Historical Pass 1–44/early Pass45 states remain in Git history and reports.
> Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

## 1. Current context — 2026-08-25

- Repository: `valentronus95/OsterConflict`
- `main` runtime baseline before current corrective branch: `1d6f57227bb15e84d7df911c192f53685b08544f` (PR #82).
- Active corrective branch: `fix/pass45-runtime-rejection-20260825`.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Latest factual evidence: `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`.
- Previous rejected evidence: `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x Windows.
- User launcher: `START_HERE.cmd` only.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Current status token: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-25 / CORRECTIVE SOURCE WORK CODED_UNTESTED / NOT MERGED**.

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

## 4. Confirmed corrective source work on active branch

All items below remain **CODED_UNTESTED** until local UE acceptance.

### 4.1 Stale runtime owner retirement

Physically deleted because they were runtime-rejected, destructive late owners, or inert compatibility shells:

- `OCWorldProductionVisualsSubsystem.h/.cpp` — rejected B2 black/generic world visual owner;
- `OCMuseumCoreRecoverySubsystem.h/.cpp` — delayed recovery carrier + R13.8 rebuild/detail replay owner;
- `OCMuseumVisibilityPass37Subsystem.h/.cpp` — late polling/destructive architecture rebuild/visibility owner;
- `OCLandmarkShellOwnershipGuardSubsystem.h/.cpp` — late duplicate-destroy owner with stale R13.7/R13.8 shell semantics;
- `OCR137MuseumSiteReplacementSubsystem.h/.cpp` — already-inert retired compatibility shell;
- `OCR13MuseumStadiumPhotoFidelitySubsystem.h/.cpp` — already-inert retired compatibility shell;
- `OCWeaponPalettePass37Subsystem.h/.cpp` — retired no-op palette compatibility shell;
- `VERIFY_PASS45_COMPLETION_AUDIT.py` + `.github/workflows/pass45-completion-audit.yml` — stale CI contract that required the rejected B2 owner back.

Replacement/guardrails:

- root `AGENTS.md` now requires physical retirement when a rejected legacy mutating owner has no required data/collision role;
- `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` + workflow fail if retired owners are resurrected;
- Pass35/37/38/42 and canonical Pass45 verifiers were forward-ported to current behavior instead of historical READY markers.

### 4.2 Landmark runtime mutation ordering

- `OCLandmarkStartupCoordinatorSubsystem` is the current startup orchestration window.
- Historical stage timers are cancelled before immediate authoritative stage calls.
- marker: `PASS45_LANDMARK_STARTUP_COORDINATED_READY ... legacy_core_recovery=0 destructive_visibility_rebuild=0`.
- `OCR146LandmarkSeparationSubsystem` is now **validation-only**:
  - no `Destroy()`;
  - no `RemoveInstance()`;
  - no actor-spawn mutation guard;
  - one bounded validation after startup;
  - overlap/legacy errors produce `PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL ... primary_authoring_fix_required=1` rather than late repair.

Important unresolved Museum architecture fact:

- R13.7 is **not** merely a reference parent. Its code builds a complete visible museum exterior (brick body, roofs, timber upper room, vestibule, annex, windows/details).
- R13.8 also builds visible architecture/interactivity.
- therefore historical verifier/comments claiming `R13.8 single Museum shell + R13.7 reference-only` are invalid.
- required next migration: **R13.7 = visible photo-driven exterior; R13.8 = interaction/collision owner without competing visible shell**, unless runtime/reference evidence proves a better consolidated ownership model.

### 4.3 Vehicle BASE teleport correction

Exact root cause found: Museum BASE guard previously treated every new possessed `APawn` as a new BASE deployment. `character -> vehicle -> character` possession transitions could therefore trigger Museum correction.

Current source:

- validates only `AOCCharacter`;
- at most once per player controller;
- vehicle pawn is never BASE deployment evidence;
- markers: `PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE` / `PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE`, `vehicle_revalidation=0`.

Runtime vehicle enter/drive/exit still requires factual test.

### 4.4 HMMWV / BTR / M2 transforms

- HMMWV production mesh: independent X/Y/Z fitting removed; uniform scale + native long-axis correction + grounded bounds.
- BTR-4 production mesh: same proportional correction.
- markers: `PASS45_HMMWV_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`, `PASS45_BTR4_PROPORTIONAL_VISUAL_READY ... nonuniform_stretch=0`.
- M2 visual: uniform fit + bottom-of-bounds to mount plane; marker `PASS45_M2_MOUNT_ALIGNMENT_READY ... bottom_on_mount=1`.

Open source conflict:

- `AOCVehicleBase::BeginPlay()` still applies legacy BasicShape MID tint after derived `ApplyVehicleStyle()` and can overwrite production authored materials;
- `OCProductionVehicleVisualGuardSubsystem` currently restores those slots after the fact;
- correct next migration is to stop legacy tint from touching `/Game/Production/` components at the primary source, then retire the repair guard if no other responsibility remains.

### 4.5 Display / thermal recovery

- normal route no longer forces `-windowed`;
- recovery route requests `-fullscreen`;
- normal recovery cap: `t.MaxFPS 60`;
- render scale is not lowered;
- marker: `PASS45_NORMAL_DISPLAY_THERMAL_GUARD fullscreen=1 max_fps=60 render_scale_mutation=0`.

### 4.6 Runtime acceptance script

`RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd` was forward-ported:

- deleted Pass37 Museum rebuild / retired palette markers are no longer required;
- requires coordinated landmark startup;
- requires proportional HMMWV/BTR and M2 mount markers;
- requires current actual-pawn/compact-map/rack/material/FPS evidence;
- vehicle teleport and M2 pitch direction remain mandatory manual runtime gates until direct log evidence is added.

## 5. Active requirements

| ID | Requirement | Repeat | Status | Current action |
|---|---|---:|---|---|
| STALE-OWNER-001 | Old code/verifiers must not overwrite or resurrect newer runtime behavior | 1 | CODED_UNTESTED | Physical retirement policy active; old B2/Museum rebuild/palette/ownership shells deleted; audit continues. |
| PERF-COLLAPSE-001 | No severe FPS/thermal collapse | ≥7 | CODED_UNTESTED | Fullscreen + 60 FPS recovery cap; destructive Museum loops removed; local runtime required. |
| VIS-BLACK-WORLD-001 | No black ground/world corruption | 1 | CODED_UNTESTED | Runtime-rejected B2 material/house/fence owner physically deleted; readable baseline must be retested. |
| LOC-MUSEUM-001 | Correct visible Oster Local History Museum near BASE | ≥11 | CODED_UNTESTED | R13.7 is the only visible exterior owner; R13.8 is hidden collision/final glass; late Pass32 repair mutation retired; factual screenshot still required. |
| LOC-CULTURE-001 | Culture House separate from Museum | ≥3 | IN_PROGRESS / CODED_UNTESTED | R14.6 stays at separate geo anchor; validation-only parcel check replaces late repair. |
| LOC-SILPO-001 | Silpo one correct site owner | ≥2 | CODED_UNTESTED | Coordinated startup retained; parcel overlap now fail-visible. |
| LOC-TOWER-001 | Remove unreferenced dark steep-roof tower/shack | 1 | CODED_UNTESTED / INVESTIGATE | Pass35 recovery carrier/roof owner removed; verify next runtime whether artifact disappears, then trace remaining owner if not. |
| GAME-SPAWN-001 | Initial live pawn spawns near Museum BASE | ≥10 | CODED_UNTESTED | Initial-character-only correction coded; no vehicle pawn revalidation. |
| GAME-VEHICLE-TELEPORT-001 | Enter/drive/exit never teleports back to Museum | 1 | CODED_UNTESTED | Exact possession-triggered BASE guard cause removed; local car/HMMWV/BTR test required. |
| VEH-HMMWV-001 | HMMWV proportions/orientation correct | ≥6 | CODED_UNTESTED | Uniform scale + axis correction; runtime multi-angle screenshot required. |
| VEH-BTR-001 | BTR-4 proportions/orientation correct | ≥6 | CODED_UNTESTED | Uniform scale + axis correction; white material artifact still open. |
| VEH-M2-MOUNT-001 | M2 aligned on HMMWV mount | ≥2 | CODED_UNTESTED | Bottom-on-mount bounds alignment coded. |
| VEH-M2-PITCH-001 | Mouse up raises M2 when Invert Y OFF | 1 | CODED_UNTESTED | Source contract corrected; `PASS45_M2_GUNNER_PITCH_CONTRACT_READY` added; runtime input proof required. |
| VEH-MATERIAL-OWNER-001 | Base vehicle tint must not overwrite production authored materials | 1 | CODED_UNTESTED | `VehicleBase` bypasses `/Game/Production/`; former repair guard is read-only validation-only, no material mutation/polling. |
| WEAPON-MATERIAL-001 | Required rack weapons use authored material + texture dependencies | ≥11 | CODED_UNTESTED / CONTENT CHECK | Truth-only preflight retained; white/default slots fail; no runtime recolour owner. |
| GAME-WEAPONS-001 | 11 grounded pickups near Museum BASE | ≥9 | CODED_UNTESTED | 12 cm ground clearance retained. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | ≥4 | CODED_UNTESTED | Hard reference topology retained; runtime screenshot required. |
| MAP-EXTENT-001 | Keep compact central Oster battlefield | ≥2 | CODED_UNTESTED / RETAIN | 960×940 m hard extent retained; never restore 2.4 km world. |
| VIS-TREES-001 | No primitive/fantasy visible tree forest | ≥2 | CODED_UNTESTED / CONTENT GAP | primitive proxy retirement retained; real pines known; oak unverified. |
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
14. late landmark `Destroy/RemoveInstance` cleanup used to hide primary-authoring errors.

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
12. [x] Pass35/37/38/42 + canonical Pass45 verifiers forward-ported away from retired owners.
13. [x] Consolidate Museum visual ownership: R13.7 visible exterior; R13.8 hidden interaction/collision + final glass; obsolete R14.1 replacement owner deleted.
14. [x] Fix default M2 gunner pitch direction in source; factual runtime input proof remains required.
15. [x] Stop `AOCVehicleBase` legacy tint from touching production assets; production visual guard converted to read-only validation with no repair/polling.
16. [ ] Close BTR white material artifact and remaining weapon authored material/texture gaps supported by current content.
17. [ ] Trace/remove any remaining unreferenced tower/shack or rejected generic near-Museum visual.
18. [ ] Run full current-head source CI; forward-port any remaining stale verifier instead of restoring old behavior.
19. [ ] Create/refresh corrective PR only after source scope is coherent.
20. [ ] Merge only after current-head source CI green.
21. [ ] Local `START_HERE.cmd -> 1. ЗВИЧАЙНА ГРА` factual UE build/runtime acceptance.

## 8. Next factual runtime gates

### Build
- UBT/UE 5.8 build exits 0.

### World / landmarks
- no large black world/ground regions;
- no late scenery pop/rebuild caused by deleted recovery owners;
- Museum visually matches Museum identity, not Culture House;
- Culture House and Silpo visibly separate;
- `PASS45_LANDMARK_SEPARATION_VALIDATION_READY` with zero generic parcel overlap.

### Spawn / vehicles
- actual initial pawn near Museum BASE;
- civilian vehicle enter does not move vehicle to Museum;
- drive away, exit beside current vehicle transform;
- repeat HMMWV/BTR;
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

**Current overall status: PASS 45 ACTIVE / RUNTIME REJECTED / CORRECTIVE SOURCE WORK CODED_UNTESTED / ACTIVE BRANCH NOT MERGED.**

## Pass45 corrective milestone — 2026-08-25 Museum/vehicle ownership

Status: **CODED_UNTESTED / latest factual runtime remains RUNTIME REJECTED until a new local UE 5.8 run**.

- Branch: `fix/pass45-runtime-rejection-20260825`.
- Museum visible ownership collapsed to R13.7; R13.8 is collision + final breakable glass only.
- obsolete R14.1 Museum window replacement header/source deleted; coordinator no longer invokes it.
- R14.0 late R13.7 suppression/instance removal retired; R14.5 owns the only current Museum tree layout.
- production VehicleBase no longer applies legacy BasicShape tint to `/Game/Production/`; validation guard is read-only.
- M2 gunner default pitch source corrected: invert off => mouse up raises aim.
- driver/gunner vehicle enter-exit transform telemetry added; normal path records `museum_respawn_path=0`.
- stale-retirement verifier forward-ported to reject resurrection of these removed owners/repair paths.
- Work report: `OsterConflict/Docs/WorkReports/PASS45_RUNTIME_RECOVERY_CORRECTIVE_2026-08-25_MUSEUM_VEHICLE.md`.

Remaining factual acceptance: local UE build, Museum screenshot, vehicle drive/exit away from Museum, M2 pitch, HMMWV/M2/BTR materials/proportions, weapon material gaps, invalid Oster generic visuals, thermal/FPS behavior.


## Pass45 preserved local UE build/import rejection — 2026-08-25

Status of this historical attempt: **LOCAL UE BUILD REJECTED**. This is preserved as factual chronology even though a later corrected build reached gameplay.

- UE 5.8/MSVC rejected the tactical-map `FVector2D` road table with **C2131** while it was `constexpr`; source now uses a normal `const` table.
- UE 5.8 Interchange rejected the deprecated `auto_detect_mesh_type` property during HMMWV/M2 GLB intake; current importer explicitly forces StaticMesh through the supported Interchange API.
- These source corrections remain **CODED_UNTESTED** as corrections to that failed attempt; a later run reaching gameplay does not erase the earlier failure record.
- Current overall Pass45 truth remains **RUNTIME REJECTED / CORRECTIVE SOURCE WORK CODED_UNTESTED** until the next factual local UE 5.8 acceptance.

### Museum layer ownership correction after audit

- `OCMuseumLayerPerformanceGuardSubsystem` was found to be a conflicting late mutator: it could hide R13.7 visible shell components, remove source instances, alter collision/navigation and retune render state after the new Pass45 Museum owner had built.
- It is now **validation-only**: one delayed observation, `mutation=0`, with `PASS45_MUSEUM_LAYER_VALIDATION_READY/FAIL`; any failure requires a primary-authoring fix instead of late repair.
- R13.7 empty `WindowGlass`/`GreyDoors` prototype plumbing was removed; R13.7 owns visible exterior/trim/grilles only.
- Runtime acceptance now requires `PASS45_MUSEUM_LAYER_VALIDATION_READY` and rejects `PASS45_MUSEUM_LAYER_VALIDATION_FAIL`; retired Pass30/Pass32 repair markers are no longer acceptance gates.
- Production-model CI was forward-ported to Pass45 uniform/native-bounds HMMWV/BTR geometry rather than resurrecting non-uniform legacy fitting.
