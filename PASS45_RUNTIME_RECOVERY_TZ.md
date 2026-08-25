# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date: 2026-08-24
Latest runtime rejection: 2026-08-25
Status: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-25 / CORRECTIVE WORK IN PROGRESS**
Active corrective branch: `fix/pass45-runtime-rejection-20260825`
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

The Pass 45 B2 production-visual completion layer is no longer accepted merely because source verification passed. The latest runtime rejects its visible output.

Requirements:

- audit `OCWorldProductionVisualsSubsystem` first because the black-world/generic-house/fence regression appeared after the B2 visual-owner work;
- do not keep an experimental visual owner active solely to satisfy the old completion verifier;
- if the current B2 layer cannot guarantee correct material output, disable it from normal runtime and restore the last readable baseline while a reference-faithful production layer is rebuilt;
- no silent fallback to black/default material;
- material load failure must remain fail-visible in logs without corrupting the entire scene;
- do not lower native render scale to disguise the problem.

Acceptance:

- no large black ground/world regions in normal gameplay;
- ground, roads and sidewalks remain readable under the normal renderer;
- runtime screenshot mandatory.

### P0 — vehicle possession/exit teleport

Current behavior is unacceptable:

- entering a civilian vehicle may move player/vehicle to Museum;
- exiting after driving to another location may return the player to Museum.

Requirements:

- Museum spawn guard applies only to initial deployment/spawn recovery, never ordinary vehicle possession/unpossession;
- `EnterDriver` must preserve the vehicle's current world transform;
- `ExitDriver`/`GetExitTransform()` must place the human pawn adjacent to the vehicle's **current** transform;
- vehicle input recovery must restore input, not call respawn/restart at Museum;
- no generic `RestartPlayer` path may be used for normal vehicle exit;
- add runtime markers containing vehicle location, requested exit transform and resulting pawn location.

Acceptance:

- enter a civilian car away from Museum: no teleport;
- drive to BTR: location preserved;
- exit vehicle: pawn appears beside vehicle, not at Museum;
- repeat with HMMWV/BTR.

### P0 — vehicle production visual transforms

HMMWV and BTR-4 are now present but not visually acceptable.

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

Requirements:

- M2 mount transform must be tied to the HMMWV roof/turret socket, not a generic proxy offset;
- barrel must face vehicle-forward in neutral pose;
- gunner camera/aim origin must match mount;
- normal vertical gun aim must **not be inverted**;
- default pitch input contract: mouse up raises aim, mouse down lowers aim;
- if an optional invert setting is ever added later, default remains OFF.

Acceptance:

- Browning visually centred/aligned on the HMMWV mount;
- gunner input direction correct in runtime.

### P0 — Museum / Culture House identity

User reference history and public Oster references agree on the core identity conflict:

- Oster Local History Museum is the former Solonyna house at Tatarska 30, a late-19th-century brick/wood residential building;
- the six-column neoclassical civic facade is a separate Culture House/public building;
- therefore a six-column Culture-House shell at the Museum site is always a runtime failure.

Requirements:

- exactly one current Museum visible shell owner;
- exactly one current Culture House visible shell owner;
- Culture House shell may never own or overlap Museum anchor/site radius;
- historical Museum/Culture replacement layers that can rebuild the wrong shell must become inert or detail-only;
- if the correct photo-faithful Museum production asset is unavailable, use a truthful minimal placeholder consistent with the Museum footprint rather than the Culture House facade.

Acceptance:

- Museum and Culture House visibly distinct and spatially separate;
- Museum screenshot must not show the six-column Culture House facade.

### P1 — invalid Oster fences/houses/tower

The latest screenshots reject the current generic village visual set as Oster production content.

Requirements:

- existing AdvancedVillagePack house/fence assets are not automatically accepted just because they are real meshes;
- supplied user photos/history are the primary visual authority;
- public Oster references may be used only to fill gaps and must not override user references;
- remove the steep-roof dark tower/shack unless a reference proves it belongs to the selected compact Oster area;
- fence families must match Oster reference character: real local metal/wood/sheet fence types where shown, not generic fantasy/village fencing;
- no arbitrary decorative building family may be introduced outside the accepted topology/reference set.

Acceptance:

- no rejected fence family visible near Museum/current test area;
- no unreferenced tower/shack;
- new visual family requires a traceable reference.

### P1 — weapon material/texture closure

Current factual runtime:

- AK-47 appears materially correct;
- multiple other required rack weapons remain white/default.

Requirements for all required weapon classes:

`weapon class -> exact mesh -> material slot(s) -> material asset(s) -> texture dependencies -> runtime appearance`

- white/default slot = FAIL;
- `DefaultMaterial`, `BasicShapeMaterial`, missing material or missing required texture = FAIL;
- mesh-load success alone is never production readiness;
- no generated grey/white colour repair;
- no M16/M4 READY claim without verified real payload.

Acceptance:

- runtime rack screenshot shows authored appearance for every required available weapon;
- any unresolved item is explicit `CONTENT GAP`, not READY.

### P1 — fullscreen and thermal behavior

Latest normal run opened windowed and machine heated strongly while FPS reached roughly 100–156.

Requirements:

- remove hard-coded normal-route `-windowed` behavior;
- normal user route opens in intended fullscreen/borderless fullscreen according to saved settings;
- diagnostic compatibility route may remain explicitly windowed only if clearly labelled;
- normal route must use a thermal-safe default frame cap of **60 FPS** during recovery;
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
- any required weapon whose authored material/texture dependencies fail fresh UE preflight: `CONTENT GAP`.

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
15. normal playtest running uncapped while thermal recovery is active.

## 8. Corrective execution order — current pass

1. [x] Archive latest runtime screenshots and mark Pass 45 `RUNTIME REJECTED`.
2. [x] Update canonical TZ with latest runtime defects.
3. [ ] Disable/rework rejected B2 production-visual layer causing black/generic world output.
4. [ ] Fix vehicle enter/exit transform path; remove Museum fallback from ordinary vehicle exit.
5. [ ] Fix HMMWV/BTR production visual scaling/orientation/material truth.
6. [ ] Correct M2 Browning mount transform and disable default pitch inversion.
7. [ ] Enforce Museum/Culture House single truthful site ownership.
8. [ ] Remove unreferenced tower/shack and rejected generic fence/house visuals.
9. [ ] Remove normal-route forced windowed mode and apply recovery 60 FPS cap.
10. [ ] Close all weapon authored material/texture dependencies that existing content can support.
11. [ ] Forward-port/retire stale verifiers that require rejected behavior.
12. [ ] Full current-head source CI green.
13. [ ] Merge corrective branch to `main`.
14. [ ] Factual local UE build + runtime acceptance.

## 9. Acceptance gates

Pass 45 cannot become `VERIFIED RUNTIME` until all applicable factual gates pass.

### Gate A — build

- UE 5.8 build succeeds with exit code 0.

### Gate B — world materials

- no large black world/ground corruption;
- no silent default/failed material replacement.

### Gate C — performance/thermals

- frontend/gameplay >=30 FPS minimum;
- recovery normal-route frame cap ~60 FPS;
- no severe progressive thermal behavior;
- no render-scale downgrade.

### Gate D — landmarks

- Museum and Culture House visually separate;
- Museum is not the six-column Culture House facade;
- Silpo remains one separately owned site.

### Gate E — environment references

- no rejected generic fence/house family near tested Oster area;
- no unreferenced dark tower/shack;
- visible production family has reference support.

### Gate F — weapons

- required available weapons use authored materials/textures;
- white/default slots fail.

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

## 10. Current status

- Pass 44: **RUNTIME REJECTED** historical evidence.
- Pass 45 source corrections through PR #82: historical source/build progress only.
- Latest factual 2026-08-25 gameplay: **RUNTIME REJECTED**.
- Current branch: `fix/pass45-runtime-rejection-20260825`.
- Runtime verification: **NOT ACHIEVED**.
