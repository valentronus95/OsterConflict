# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date: 2026-08-24
Latest runtime rejection: 2026-08-25
Status: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-25 / PR #94 SOURCE CI GREEN / P0 BLACK-WORLD SOURCE CONTRACT CODED_UNTESTED / LOCAL UE RUNTIME PENDING**
Current integrated source milestone: `main` @ `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f` (PR #93 merged)
Active source continuation: `fix/pass45-runtime-rejection-material-closure-20260826`, PR #94. Code head `9ba84273d06a6c210c808ceacbe96c45466a3d73` completed the full current GitHub source matrix **39/39 green** before this documentation synchronization. PR remains unmerged pending factual local UE 5.8 acceptance.
Target: UE 5.8.x Windows
User launcher: `START_HERE.cmd`

## 0. Authority

This file is the canonical active TZ for Pass 45.

Authority order remains:

1. latest explicit user requirement + latest factual local UE screenshot/log;
2. root `AGENTS.md`;
3. this TZ + current work ledger;
4. current implementation;
5. historical pass reports/verifiers.

A green source check never overrides a factual broken runtime.

Latest evidence pack:

`RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`

Previous runtime rejection pack:

`RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

### 0.1 Legacy owner deletion contract — 2026-08-25

Pass 45 now treats stale runtime code as an architectural defect, not harmless history.

**Physical retirement beats inert resurrection.** If runtime evidence rejects an old `UWorldSubsystem`, visual owner, mutation layer or compatibility path and that class no longer owns required collision/data, delete the source owner and its stale verifier/workflow instead of leaving a compiled no-op that can later be re-enabled by historical CI.

**No historical verifier may require a runtime-rejected owner.** Source CI must be forward-ported to current behavior. A verifier that requires a rejected READY marker, generic asset family, material mutation, spawn fallback or old owner class is itself stale and must be retired or rewritten.

Current legacy owner deletion already applied in this corrective branch:

- deleted `OCWorldProductionVisualsSubsystem.h/.cpp` after the latest runtime rejected its black/generic world output;
- deleted `VERIFY_PASS45_COMPLETION_AUDIT.py`, which explicitly required that rejected B2 owner and its generic AdvancedVillagePack/ground-material behavior;
- deleted `.github/workflows/pass45-completion-audit.yml` for the same stale contract;
- added `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` + workflow so those rejected owners/contracts cannot silently return;
- `RUN_ALL_VERIFY.py` now runs the retirement verifier instead of the stale B2 completion verifier.

This is a **legacy owner deletion** migration, not proof that the replacement visuals are correct. Runtime remains `CODED_UNTESTED` until the next local UE playtest.

For every remaining Museum/world/material/spawn layer, classify it as one of:

1. current mutating owner — may mutate its responsibility;
2. data/collision-only legacy support — may remain but must not overwrite current visuals/transforms/materials;
3. obsolete conflicting owner — delete physically together with stale verifier expectations.

One runtime responsibility may have only one mutating owner. Pass chronology does not grant ownership.

## 1. Current factual verdict

The local UE 5.8 build blocker discovered on 2026-08-25 was fixed by PR #82 and the project now reaches gameplay. That proves the previous C2131 tactical-map compile blocker is no longer the immediate blocker.

The resulting gameplay is nevertheless **RUNTIME REJECTED**.

The latest screenshots and user observation prove:

- large world/ground areas render black;
- several required weapons remain white/default/untextured while AK-47 renders correctly;
- visible generic fences/houses do not match Oster references;
- the Museum site still presents a six-column Culture-House-like facade instead of the actual Oster Local History Museum identity;
- an unapproved steep-roof dark tower/shack remains visible;
- HMMWV visual proportions are deformed/accordion-like;
- M2 Browning is mounted with visibly wrong transform/alignment;
- normal mounted Browning vertical aim is inverted;
- entering the red civilian vehicle teleported the vehicle/player to the Museum area;
- after driving to the BTR and exiting, the player was teleported back to Museum again;
- BTR-4 proportions/orientation are visibly wrong, including stretched rear/body geometry;
- BTR-4 has a large white/default material artifact;
- normal route unexpectedly opened windowed;
- runtime reached roughly 100–156 FPS while the machine heated strongly;
- high FPS in a visually broken/black scene is not performance acceptance.

**PASS 45 = RUNTIME REJECTED.**

## 2. Confirmed improvement that must be retained

The latest run also proves some previous blockers were removed and must not regress:

- UE build reaches gameplay instead of stopping on tactical-map C2131;
- BTR-4 asset intake reaches runtime;
- HMMWV production mesh reaches runtime;
- M2 production mesh reaches runtime;
- catastrophic 8–12 FPS behavior from the previous rejected run is not reproduced in these screenshots;
- no implicit normal-game 16-bot autofill may return;
- compact central-Oster map bounds remain current;
- no grey BasicShape weapon-material repair may return.

These are partial improvements only. None promote Pass 45 to VERIFIED RUNTIME.

## 3. Root-cause priorities for the current corrective pass

### P0 — black world/material corruption

The world is visually invalid while large areas render near-black.

The Pass 45 B2 production-visual completion layer is runtime-rejected and has now been **physically deleted** from the corrective branch. Its stale completion verifier/workflow were deleted with it so CI cannot resurrect the rejected owner.

Source audit/correction continuation — 2026-08-26, PR #94, **CODED_UNTESTED**:

- `AOCVisualEnvironment` is the current component-owned replicated daylight owner; no second camera/post-process exposure owner was found in the audited runtime path;
- the previous `SunLight->SetIntensity(4.0f)` contract conflicted with disabled auto exposure and was a concrete P0 black-world candidate under UE 5.8 physical-light units;
- daylight is now one paired renderer contract: `120000 lux` + `r.DefaultFeature.AutoExposure=True` + `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True`;
- the old `4 lux` path and `AutoExposure=False` are source-gated out;
- `AOCWorldSectorOster` remains the accepted source owner for semantic `Ground/Roads/Sidewalks` MID tinting;
- audited late systems do not own a second `Ground/Roads/Sidewalks` material rewrite; `OCRuntimeAcceptancePass6Subsystem` only removes obsolete BASE road/sidewalk instances and applies its separate `SetMaterial()` path to weapon components;
- `UOCWorldGeometryStabilitySubsystem` now validates `Ground/Roads/Sidewalks` semantic MID + `Color` parameter at the 12 s baseline and again through the 16 s / 20 s stability window;
- missing semantic component/material/MID/`Color` parameter emits family-specific `PASS12_WORLD_GEOMETRY_STABILITY_FAIL` instead of silently allowing a broken black/default surface;
- success emits `PASS45_WORLD_MATERIAL_BASELINE_READY` and `PASS45_WORLD_MATERIAL_STABLE`;
- strict `START_HERE -> 2. ПОВНИЙ RUNTIME-ТЕСТ` evidence now requires `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`, `PASS12_WORLD_GEOMETRY_STABLE` and `PASS45_WORLD_MATERIAL_STABLE`, and forbids `PASS12_WORLD_GEOMETRY_STABILITY_FAIL`;
- `VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` now source-gates that P0 evidence linkage so the black-world check cannot regress into an optional side test;
- code head `9ba84273d06a6c210c808ceacbe96c45466a3d73` completed **39/39 GitHub source workflows green**, including `World geometry stability pass 12`, `Pass 45 strict runtime acceptance harness`, `Runtime recovery Pass 45` and full `Source verification`.

Requirements:

- do not recreate `OCWorldProductionVisualsSubsystem` under a new name with the same behavior;
- retain the readable semantic baseline until a reference-faithful production visual owner is proven in runtime;
- no silent fallback to black/default material;
- material load failure must remain fail-visible in logs without corrupting the entire scene;
- no second world-material owner may overwrite Ground/Roads/Sidewalks after the accepted current owner;
- physical daylight lux, auto exposure and extended EV100 range are one contract; no one-sided change may be accepted;
- strict main runtime acceptance must consume world-material stability evidence, not leave it in a separate optional launcher;
- do not lower native render scale to disguise the problem.

Acceptance:

- `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY` present in the gameplay log;
- `PASS12_WORLD_GEOMETRY_STABLE` and `PASS45_WORLD_MATERIAL_STABLE` present;
- `PASS12_WORLD_GEOMETRY_STABILITY_FAIL` absent;
- no large black ground/world regions in normal gameplay;
- ground, roads and sidewalks remain readable under the normal renderer;
- runtime screenshot mandatory.

### P0 — vehicle possession/exit teleport

Exact source cause found: the Museum BASE guard historically validated every newly possessed `APawn`, so `character -> vehicle -> character` possession transitions were falsely treated as fresh BASE deployments.

Corrective source behavior is now initial-character-only.

Requirements:

- Museum spawn guard applies only to initial deployment/spawn recovery, never ordinary vehicle possession/unpossession;
- only `AOCCharacter` can be BASE deployment validated;
- each controller is validated at most once for initial BASE recovery;
- `EnterDriver` must preserve the vehicle's current world transform;
- `ExitDriver`/`GetExitTransform()` must place the human pawn adjacent to the vehicle's **current** transform;
- vehicle input recovery must restore input, not call respawn/restart at Museum;
- no generic `RestartPlayer` path may be used for normal vehicle exit;
- runtime markers must prove initial-only recovery and no vehicle revalidation.

Acceptance:

- enter a civilian car away from Museum: no teleport;
- drive to BTR: location preserved;
- exit vehicle: pawn appears beside vehicle, not at Museum;
- repeat with HMMWV/BTR.

### P0 — vehicle production visual transforms

HMMWV and BTR-4 are now present but the latest accepted runtime evidence rejected their previous non-uniform fit.

Source correction now uses uniform scale + native longest-axis correction + grounded bounds for both production meshes.

Requirements:

- never non-uniformly stretch production vehicle meshes to fit a generic proxy box;
- use uniform scale derived from the production mesh's authoritative bounds;
- correct forward axis and yaw so front/side/rear remain physically coherent;
- preserve wheel/body proportions;
- BTR rear may not be stretched;
- white/default BTR material slot is a hard material failure;
- production visual guard must fail instead of calling a distorted mesh READY.

Acceptance:

- HMMWV and BTR proportions recognizable from front/side/rear;
- no accordion stretch;
- no large white body panel/material artifact;
- fresh screenshot from at least two sides of each vehicle.

### P0 — M2 Browning mount and controls

M2 source visual alignment is being migrated from proxy-centre placement to production-bounds mount placement.

Requirements:

- M2 mount transform must be tied to the HMMWV roof/turret mount plane, not a generic proxy centre;
- production M2 uses uniform scale and bottom-on-mount bounds alignment;
- barrel must face vehicle-forward in neutral pose;
- gunner camera/aim origin must match mount;
- normal vertical gun aim must **not be inverted**;
- default pitch input contract: mouse up raises aim, mouse down lowers aim;
- if optional invert setting is enabled, only that setting reverses the contract.

Acceptance:

- Browning visibly centred/aligned on the HMMWV mount;
- gunner input direction correct in runtime.

### P0 — Museum / Culture House identity and owner consolidation

User reference history and public Oster references agree on the core identity conflict:

- Oster Local History Museum is the former Solonyna house at Tatarska 30, a late-19th-century brick/wood residential building;
- the six-column neoclassical civic facade is a separate Culture House/public building;
- therefore a six-column Culture-House shell at the Museum site is always a runtime failure.

The current source contains a historical stack of Museum owners/recovery/detail/validation layers (`R137/R138/R140/R141/R142/R143/R144/R145`, CoreRecovery, VisibilityPass37, ownership/startup guards). Pass 45 must stop treating pass number as permission for every layer to mutate the same landmark.

Requirements:

- audit every Museum-related subsystem for `SpawnActor`, `Destroy`, transform, material, visibility or replacement mutations;
- exactly one current Museum visible shell owner;
- exactly one current Culture House visible shell owner;
- detail-only layers may remain only when they cannot relocate/replace/hide the authoritative shell;
- data/collision-only legacy layers must be explicitly non-mutating for current visual ownership;
- obsolete shell/recovery/replacement layers are physically deleted with stale verifier expectations;
- Culture House shell may never own or overlap Museum anchor/site radius;
- if the correct photo-faithful Museum production asset is unavailable, use a truthful minimal placeholder consistent with the Museum footprint rather than the Culture House facade.

Acceptance:

- Museum and Culture House visibly distinct and spatially separate;
- Museum screenshot must not show the six-column Culture House facade;
- source audit proves there is no second late shell owner capable of undoing the current Museum state.

### P1 — invalid Oster fences/houses/tower

The latest screenshots reject the current generic village visual set as Oster production content.

Requirements:

- existing AdvancedVillagePack house/fence assets are not automatically accepted just because they are real meshes;
- supplied user photos/history are the primary visual authority;
- public Oster references may be used only to fill gaps and must not override user references;
- remove the steep-roof dark tower/shack unless a reference proves it belongs to the selected compact Oster area;
- fence families must match Oster reference character: real local metal/wood/sheet fence types where shown, not generic fantasy/village fencing;
- no arbitrary decorative building family may be introduced outside the accepted topology/reference set.

Source retirement continuation — 2026-08-26, PR #91, **CODED_UNTESTED**:

- normal runtime no longer spawns the explicitly non-reference-specific `AOCEnterableHouse`;
- `BuildResidentialBlocks()` procedural house/shed/private-fence grid owner is physically removed from `AOCWorldSectorOster`;
- the generic `BuildSolomiiKrushelnytskoiStreet()` house/shed/fence generator is physically removed; road topology remains owned by `BuildRoadNetwork()`;
- Museum/Stadium/College reference-driven fence geometry remains;
- `VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` prevents silent resurrection of those generic owners;
- runtime absence of the previously observed dark tower/shack remains Gate E evidence and is not claimed from source alone.

Acceptance:

- no rejected fence family visible near Museum/current test area;
- no unreferenced tower/shack;
- new visual family requires a traceable reference.

### P1 — weapon material/texture closure

Current factual runtime:

- AK-47 appears materially correct;
- multiple other required rack weapons remain white/default.

Current source closure in PR #94:

- Stein weapon imports are revisioned and **texture-first**: committed authored PNGs are imported before their FBX material chains;
- production/BTR intake is revisioned; stale `.uasset` existence is not proof that the current material contract was imported;
- canonical BTR intake prefers a local FBX and otherwise uses the repository-safe authored GLB material path;
- runtime weapon validation follows Gate F: every required class must resolve an **exact production visual OR an explicit real-mesh fallback**;
- an explicit fallback remains exact-production `CONTENT GAP` and must never be relabelled production READY;
- every accepted required-available visual must have non-placeholder authored material slots and actual used texture dependencies;
- absent exact Remington870/M249 payload remains explicit `CONTENT GAP`; their real fallback may remain playable only if its own authored material/texture chain is valid;
- code head `9ba84273d06a6c210c808ceacbe96c45466a3d73` passed **39/39 GitHub source workflows** before documentation synchronization.

Requirements for all required weapon classes:

`weapon class -> exact mesh OR explicit real fallback -> material slot(s) -> material asset(s) -> texture dependencies -> runtime appearance`

- white/default slot = FAIL;
- `DefaultMaterial`, `BasicShapeMaterial`, missing material or missing required texture = FAIL;
- zero/placeholder used texture dependency = FAIL;
- mesh-load success alone is never production readiness;
- no generated grey/white colour repair;
- fallback never impersonates exact production readiness;
- no M16/M4 READY claim without verified real payload.

Acceptance:

- runtime rack screenshot shows authored appearance for every required available weapon;
- strict material report shows no placeholder/default slot and no `textureDependency=GAP` for accepted visuals;
- any unresolved exact item is explicit `CONTENT GAP`, not READY.

### P1 — fullscreen and thermal behavior

Latest normal run opened windowed and machine heated strongly while FPS reached roughly 100–156.

Corrective launcher source now removes forced `-windowed`, requests fullscreen and applies a 60 FPS recovery cap without changing render scale.

Requirements:

- no hard-coded normal-route `-windowed` behavior;
- normal user route opens in intended fullscreen/borderless fullscreen according to saved settings/recovery policy;
- diagnostic compatibility route may remain explicitly windowed only if clearly labelled;
- normal route uses a thermal-safe default frame cap of **60 FPS** during recovery;
- frame cap must not lower render resolution or graphics quality;
- preserve the current DX11/SM5 recovery renderer until a separate renderer upgrade is accepted;
- no automatic uncapped 100–150+ FPS normal playtest while thermal recovery is active.

Acceptance:

- normal route opens with intended display mode;
- normal runtime does not exceed the recovery cap materially;
- no strong progressive thermal/FPS collapse.

## 4. Tactical map

The compact topology work remains source-coded but is not accepted from source evidence alone.

Requirements remain:

- authoritative compact central-Oster map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`;
- north-up;
- POI authority from one geo-reference source;
- no old giant synthetic diagonal/X road topology;
- player marker visible;
- Museum / Culture House / Silpo / Stadium distinct.

Runtime `M` screenshot remains mandatory before verification.

## 5. Vegetation

Requirements remain:

- no primitive Cylinder/Sphere fantasy tree forest;
- verified real pine/conifer assets may be used;
- oak remains `CONTENT GAP` until a suitable real asset is verified;
- supplied Oster references control tree placement and species character;
- no return of generic birch/poplar proxy families solely for an old verifier.

## 6. Content gaps that remain explicit

Unless later factual evidence closes them:

- photo-faithful College production model: `CONTENT GAP`;
- complete reference-faithful park detail set: `CONTENT GAP`;
- verified real oak asset: `CONTENT GAP`;
- M16/M4 production payload: `CONTENT GAP`;
- exact Remington870 production payload: `CONTENT GAP` unless a later factual import closes it;
- exact M249 production payload: `CONTENT GAP` unless a later factual import closes it;
- any required-available weapon whose authored material/texture dependencies fail fresh UE preflight: `CONTENT GAP` / runtime FAIL, never READY.

## 7. Behavior that must not return

Pass 45 explicitly forbids:

1. implicit normal-game bot autofill;
2. historical 2.4 km gameplay map;
3. old edge BASE/test-lane/vehicle seeds;
4. coordinate-based ±920 m BASE classification;
5. grey/BasicShape weapon material repair;
6. source verifier rules that resurrect rejected runtime behavior;
7. repeated 0.20 s × 40 full-world landmark mutation scans;
8. claiming vehicle/weapon material readiness from mesh existence only;
9. declaring generic imported village assets Oster-authentic without reference support;
10. calling high FPS in a black/broken scene performance acceptance;
11. lowering render scale to disguise a performance problem;
12. Museum/Culture House shell overlap;
13. normal vehicle exit via Museum respawn fallback;
14. non-uniform production-vehicle stretching;
15. normal playtest running uncapped while thermal recovery is active;
16. compiled runtime-rejected owner classes retained solely so old CI stays green;
17. historical verifier/workflow requiring a deleted rejected owner;
18. two live mutation layers owning the same world material, landmark shell, spawn correction or production transform;
19. an all-or-nothing 11/11 exact-production weapon gate that treats a truthful exact-payload `CONTENT GAP` as permission to hide an otherwise explicit real fallback;
20. direct `START_HERE -> PLAYFLOW` full-test routing that bypasses the strict post-run material/dependency/evidence gates;
21. duplicate strict production-vehicle import owners in both `START_HERE` and `CURRENT_GAMEPLAY`;
22. restoring `4 lux + AutoExposure=False` or changing physical daylight lux/exposure/EV100 as unrelated independent settings;
23. allowing strict main runtime acceptance to pass without Pass12 world-material stability evidence.

## 8. Corrective execution order — current pass

1. [x] Archive latest runtime screenshots and mark Pass 45 `RUNTIME REJECTED`.
2. [x] Update canonical TZ with latest runtime defects.
3. [x] Physically delete rejected B2 production-visual owner and stale completion verifier/workflow.
4. [x] Replace stale completion CI contract with `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py`.
5. [x] Fix Museum deployment guard source so ordinary vehicle possession/exit is never BASE revalidation.
6. [x] Replace HMMWV/BTR non-uniform production mesh stretching with uniform proportional fitting.
7. [x] Move M2 production visual to bottom-on-mount bounds alignment.
8. [x] Remove normal-route forced windowed mode and apply recovery 60 FPS cap.
9. [x] Consolidate Museum ownership: R13.7 visible exterior; R13.8 hidden collision + final glass; R13.9/R14.0 final doors/facade; R14.5 sole tree owner; physically delete obsolete R14.1 window replacement.
10. [x] Correct default mounted M2 Browning pitch direction in source; runtime input proof still required.
11. [x] Source-retire the traced unreferenced generic house/fence owners (`AOCEnterableHouse` normal spawn, `BuildResidentialBlocks`, generic Krushelnytska house generator); **CODED_UNTESTED**, and runtime Gate E still must prove the dark tower/shack artifact is absent.
12. [ ] Close BTR white/default material slot and remaining weapon authored material/texture dependencies that existing content can support. **PR #94 source implementation + current source CI are green; local UE editor import and rendered runtime remain pending, so Gate F/G are not closed.**
13. [x] Forward-port stale production/Museum/material/launcher verifiers and lock deleted/obsolete owners and exact-only/direct-launch assumptions out of current CI.
14. [x] Update work ledger with Museum ownership, production-material, vehicle-transform, M2 pitch and PR #94 material-closure state.
15. [x] Full PR #91 current-head source CI green, including `Source verification`, Pass45 retirement/material/dependency gates, runtime source contracts, and historical regression suite.
16. [x] Corrective source milestone merged to `main` only after current-head source CI was green: PR #91 -> `c4712144efede68b3d80475bec64ea9c8e400fc4`.
17. [x] PR #93 forward-ported current R14 verifier -> `main` `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f` without claiming runtime acceptance.
18. [x] PR #94 code head `9ba84273d06a6c210c808ceacbe96c45466a3d73` completed **39/39 current GitHub source workflows green** before documentation synchronization; PR remains open/unmerged.
19. [x] Complete P0 black-world source ownership audit and pair physical daylight `120000 lux` with `AutoExposure=True` + extended EV100; add semantic MID runtime stability validation and bind it into strict main evidence. **CODED_UNTESTED**.
20. [ ] Factual local `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` UE 5.8 import + build + gameplay + rendered visual acceptance.

## 9. Acceptance gates

Pass 45 cannot become `VERIFIED RUNTIME` until all applicable factual gates pass.

### Gate A — build/import

- revisioned production vehicle/BTR import succeeds;
- texture-first Stein reimport succeeds;
- UE 5.8 build succeeds with exit code 0.

### Gate B — world materials

- `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY` present;
- `PASS12_WORLD_GEOMETRY_STABLE` present;
- `PASS45_WORLD_MATERIAL_STABLE` present;
- `PASS12_WORLD_GEOMETRY_STABILITY_FAIL` absent;
- no large black ground/world corruption;
- no silent default/failed material replacement;
- direct runtime screenshot confirms readable ground/roads/sidewalks.

### Gate C — performance/thermals

- frontend/gameplay >=30 FPS minimum;
- recovery normal-route frame cap ~60 FPS;
- no severe progressive thermal behavior;
- no render-scale downgrade.

### Gate D — landmarks

- Museum and Culture House visually separate;
- Museum is not the six-column Culture House facade;
- Silpo remains one separately owned site;
- one mutating visible shell owner per landmark.

### Gate E — environment references

- no rejected generic fence/house family near tested Oster area;
- no unreferenced dark tower/shack;
- visible production family has reference support.

### Gate F — weapons

- every required available weapon uses an exact production visual or explicit real fallback with authored non-placeholder material + used texture dependencies;
- fallback remains exact-production `CONTENT GAP`, never READY;
- white/default/BasicShape slots fail;
- `textureDependency=GAP` fails;
- runtime rack screenshot remains mandatory.

### Gate G — vehicles

- HMMWV/BTR proportions/orientation correct;
- no white BTR body artifact;
- M2 mount aligned;
- mounted pitch non-inverted by default;
- no Museum teleport on enter/exit.

### Gate H — display

- normal route uses intended fullscreen/borderless saved display mode;
- compatibility diagnostic route, if windowed, is explicitly labelled.

### Gate I — tactical map

- runtime `M` screenshot matches compact central-Oster topology and distinct POIs.

### Gate J — stale-owner retirement

- `OCWorldProductionVisualsSubsystem` remains physically absent;
- no old completion verifier/workflow can require it back;
- Museum/world/material/spawn responsibilities each have one mutating current owner;
- obsolete conflicting owners are deleted, not merely hidden behind later mutation ordering.

## 10. Current status

- Pass 44: **RUNTIME REJECTED** historical evidence.
- Pass 45 source corrections through PR #82: historical source/build progress only.
- Latest factual 2026-08-25 gameplay: **RUNTIME REJECTED**.
- Current `main`: `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f` after PR #93.
- Active PR #94: `fix/pass45-runtime-rejection-material-closure-20260826`; material/content/strict-acceptance/P0 black-world source closure is **CODED_UNTESTED**.
- PR #94 code head `9ba84273d06a6c210c808ceacbe96c45466a3d73` completed **39/39 current GitHub source workflows green** before documentation synchronization.
- P0 black-world source contract now requires physical daylight `120000 lux`, auto exposure, extended EV100 and semantic `Ground/Roads/Sidewalks` MID stability evidence in the strict main route.
- Green source CI does not prove UE editor import, UBT compile, rendered material appearance, correct exposure on the target machine, vehicle interaction, landmark identity, thermal behavior or tactical-map appearance.
- PR #94 remains open/unmerged pending factual local UE 5.8 acceptance.
- Runtime verification: **NOT ACHIEVED**.

### Corrective source milestone — 2026-08-26 P0 black-world ownership/exposure

Current source state is **CODED_UNTESTED**:

- `AOCVisualEnvironment` is component-owned and replicated; the audited current path did not reveal a second exposure owner that supersedes it after gameplay start.
- `4 lux + AutoExposure=False` is retired from the accepted source contract.
- current paired renderer contract is `120000 lux + AutoExposure=True + extended EV100 range`.
- `AOCWorldSectorOster` remains the accepted semantic Ground/Roads/Sidewalks material owner.
- Pass12 validates semantic MID + `Color` parameter and instance stability at 12/16/20 seconds.
- `VERIFY_WORLD_GEOMETRY_STABILITY_PASS_12.py` rejects restoration of `4 lux`, disabled exposure/EV100, broken semantic MID contracts and a second locally coupled world `SetMaterial` owner.
- initial broad second-owner detector produced a false positive on `OCRuntimeAcceptancePass6Subsystem`; source inspection proved its material write is weapon-only, so the static detector was narrowed without weakening runtime semantic-material validation.
- `VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` makes daylight + Pass12 world material stability part of strict automated acceptance evidence.
- `VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` guarantees that linkage remains mandatory.
- code head `9ba84273d06a6c210c808ceacbe96c45466a3d73`: **39/39 source workflows PASS**.

None of this is rendered runtime acceptance. The next local UE 5.8 run is still required to prove the black-world defect is actually gone and the scene is neither underexposed nor blown out.

### Corrective source milestone — 2026-08-25 Museum/vehicle ownership

Current corrective source state is **CODED_UNTESTED**:

- R13.7 is the single visible Museum exterior owner; prototype trees/static glass/static door slabs/wrong service prototype were removed from primary authoring.
- R13.8 owns hidden collision plus final `AOCMuseumBreakableWindow` glass only; generic prototype doors were removed.
- obsolete `OCR141MuseumWindowReplacementSubsystem` was physically deleted and removed from the startup coordinator.
- R14.0 no longer hides/removes R13.7 content late; R14.5 is the sole Museum tree-layout owner.
- `AOCVehicleBase` bypasses legacy BasicShape tinting for `/Game/Production/`; the production visual guard is validation-only.
- driver/gunner enter-exit paths now emit current-vehicle transform evidence with `museum_respawn_path=0`.
- mounted M2 default pitch source contract is mouse-up raises aim when invert-Y is off.
- detailed report: `OsterConflict/Docs/WorkReports/PASS45_RUNTIME_RECOVERY_CORRECTIVE_2026-08-25_MUSEUM_VEHICLE.md`.

None of these source changes are runtime acceptance. Local UE 5.8 build/playtest remains mandatory.

### 8.1 Corrective owner audit extension — 2026-08-25

All items below are **CODED_UNTESTED** until factual local UE runtime acceptance.

- Museum ownership audit found a stale late mutation path in `OCMuseumLayerPerformanceGuardSubsystem`. The old Pass32 behavior could hide R13.7 visible components and repair/remove world state after authoritative startup, directly violating one-owner rules.
- Current contract is validation-only: `R13.7 = visible exterior`, `R13.8 = hidden interaction collision + final breakable glass`; the layer validator may only observe and emit `PASS45_MUSEUM_LAYER_VALIDATION_READY/FAIL`, with `mutation=0` and `primary_authoring_fix_required=1` on failure.
- R13.7 no longer creates even empty prototype glass/door components; obsolete visible/prototype ownership is removed at source rather than hidden later.
- `RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd` no longer requires retired Pass30 speculative-interior or Pass32 repair READY markers. It requires the current validation-only Museum evidence.
- Production model integration CI validates proportional native-bounds HMMWV/BTR grounding and explicitly rejects reintroduction of per-axis non-uniform fitting.
- Historical local build failure remains preserved separately: **LOCAL UE BUILD REJECTED**, including tactical-map **C2131** and deprecated Interchange `auto_detect_mesh_type`; later source fixes do not erase that factual attempt.
