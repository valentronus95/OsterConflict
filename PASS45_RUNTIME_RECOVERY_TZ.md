# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date opened: 2026-08-24  
Latest factual gameplay evidence: 2026-08-26  
Latest runtime verdict: **RUNTIME REJECTED 2026-08-26**  
Target: Unreal Engine 5.8.x / Windows  
Canonical user launcher: `START_HERE.cmd`  
Active branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Active PR: **#94 OPEN / UNMERGED**  
Integrated baseline: `main` @ `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f`

## 0. Authority and non-negotiable truth rules

This file is the **single canonical active TZ for Pass 45**.

Authority order:

1. latest explicit user requirement + latest factual local UE screenshot/log;
2. root `AGENTS.md`;
3. this TZ + current work ledger;
4. current implementation;
5. historical pass reports/verifiers.

Rules:

- a green source verifier never overrides a visibly broken UE runtime;
- `CODED`, `SOURCE PASS`, `EDITOR IMPORT PASS`, `FRESH LOAD PASS` and `RUNTIME VISUAL PASS` are separate states;
- no broken content may be promoted to READY merely because a file exists or a verifier recognizes its path;
- no proxy/default/BasicShape geometry or material may impersonate production readiness;
- obsolete runtime owners that can overwrite current behavior are deleted together with stale verifier expectations, not preserved because an old pass once required them;
- one runtime responsibility has one mutating owner;
- PR #94 stays unmerged until current-head local UE 5.8 runtime acceptance actually passes.

Latest evidence pack:

`RUNTIME_EVIDENCE/2026-08-26_PASS45_REJECTED/`

Screenshot sheet:

`RUNTIME_EVIDENCE/2026-08-26_PASS45_REJECTED/PASS45_RUNTIME_2026-08-26_SCREENSHOTS.svg`

Detailed screenshot map, user observations and external factual references:

`RUNTIME_EVIDENCE/2026-08-26_PASS45_REJECTED/README.md`

Historical evidence remains preserved separately:

- `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`
- `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

## 1. Latest factual runtime state — 2026-08-26

The current branch reaches actual gameplay. Several earlier failures improved, but the rendered/gameplay result is still unacceptable.

### Confirmed improvements that must not regress

- gameplay launches;
- the previous near-black world corruption is no longer the dominant rendered state;
- AK-family/M14/MP5/Lever Action and several other firearm meshes/materials now render as recognizable authored assets;
- HMMWV visual forward direction is now coherent enough that it drives forward normally;
- HUD evidence shows the recovery **60 FPS** cap functioning;
- the previous uncapped roughly 100–156 FPS runaway state was not reported in this run;
- HMMWV/BTR/M2 production asset intake reaches gameplay rather than failing at the old source/build barriers.

These are partial improvements only. **PASS 45 remains RUNTIME REJECTED.**

## 2. P0 — weapon firing pipeline, recoil and factual shot ownership

### Runtime rejection

Latest evidence proves:

- several weapons have missing firing audio;
- holding LMB can leave camera movement/recoil continuing after actual firing has stopped;
- releasing LMB can produce a downward camera drift/kick;
- muzzle flash/tracer/projectile origin appears below or away from the visible barrel on multiple weapons;
- firing cadence, camera recoil, sound and ammunition are not consistently tied to the same factual shot event.

### Required architecture

`trigger input -> selected fire mode -> action/cadence gate -> accepted FireOneShot -> aim reconciliation -> production muzzle -> projectile/trace -> muzzle VFX -> audio -> weapon recoil -> camera recoil -> bounded recovery`

Requirements:

- trigger-held state must never itself be treated as a shot;
- one camera recoil impulse per accepted shot, zero extra impulses while a shot is rejected;
- no residual recoil timer may continue after authoritative firing stops;
- releasing LMB must not generate a second opposite pitch kick;
- camera recovery must converge without systematic downward bias;
- ammo decrement, muzzle VFX, tracer/projectile, shot audio, shell/VFX and recoil must agree on accepted shot count;
- automatic-fire cadence may tolerate bounded scheduler jitter but may not exceed configured weapon RPM materially;
- shot debug mode must be able to expose camera aim ray, final aim point and production-muzzle presentation ray.

### Current corrective source state — 2026-08-26 — CODED_UNTESTED

Source audit found concrete causes:

1. `AOCCharacter::StartLocalFireFeedback()` used a separate held-input timer and applied recoil independently from `TryFireServer()` success.
2. base `AOCWeaponBase::TryFireServer()` used `TraceOrigin` from the player view for both hit authority and muzzle/tracer/audio presentation.
3. an automatic server timer pulse arriving slightly early could fail the strict cadence comparison and stop authoritative autofire while the local feedback timer remained alive.

Current correction:

- hit authority remains camera/view-ray based for crosshair semantics;
- production muzzle presentation is resolved from the visible component tagged `OC_ProductionWeaponVisual`;
- muzzle flash, tracer and shot audio now use the production-muzzle origin rather than the camera origin;
- a bounded cadence tolerance handles small timer-scheduler jitter without intentionally raising configured RPM;
- `LocalFireFeedbackTimerHandle`, `StartLocalFireFeedback`, `StopLocalFireFeedback`, `ApplyLocalShotFeedback`, Character-local recoil offsets and their recovery loop are physically retired;
- the temporary `ShouldNeutralizeLegacyLocalRecoil` shim is removed because no second local recoil owner remains;
- confirmed camera recoil is emitted only from the server-accepted shot multicast and recovered by `AOCWeaponBase`;
- crosshair recoil expansion reads the same confirmed-shot recoil state from the active weapon;
- camera shake is requested through `NotifyConfirmedWeaponShotPresentation()` only from that confirmed-shot path;
- trigger press/release no longer creates an independent local shot/recoil stream.

This closes the duplicate local feedback owner at source level. It does **not** prove recoil feel, release behavior or camera-shake behavior in UE runtime.

Source guard:

`VERIFY_PASS45_WEAPON_MUZZLE_DROP_PHYSICS.py`

Workflow:

`.github/workflows/pass45-weapon-firing-physics.yml`

Acceptance:

- 10-shot semi test: exactly 10 shot events / 10 recoil impulses / 10 muzzle events / 10 audio events;
- full-auto hold test: no moment where shot events stop while camera recoil continues;
- release test: no artificial downward camera kick;
- empty-mag hold: no shot recoil or shot sound after ammunition reaches zero;
- rendered slow-motion/debug capture shows tracer/projectile origin at the visible muzzle on every tested weapon class.

## 3. P0 — anti-armor launcher production visual, projectile origin and sound

Latest screenshot shows the launcher in first person as primitive cylinder/box-like geometry instead of a recognizable production weapon.

Current corrective source state — **CODED_UNTESTED**:

- canonical visual remains `/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern`;
- failure to create/use that production visual now emits `PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL` rather than silently treating BasicShape fallback as acceptable;
- launcher projectile spawn now resolves from the production muzzle rather than `TraceOrigin + Dir * 90`;
- launcher ammo is committed only after projectile spawn succeeds;
- confirmed launcher shot now emits muzzle FX and weapon shot audio;
- confirmed event marker: `PASS45_LAUNCHER_CONFIRMED_SHOT`.

Acceptance:

- launcher is recognizable in pickup and first-person states;
- no visible Cube/Cylinder/BasicShape fallback;
- projectile leaves visible muzzle;
- one shot consumes one round and emits one shot-audio event;
- failed projectile spawn consumes zero ammunition and applies zero recoil.

## 4. P0 — data-driven weapon actions and fire modes

### Current corrective source state — 2026-08-26 — CODED_UNTESTED

The old Semi/Auto-only abstraction has been replaced at source level with separate selector capability and mechanical-action metadata.

Implemented model distinguishes:

- semi-automatic;
- full automatic;
- opt-in three-round burst;
- gas-operated action;
- delayed blowback;
- blowback;
- short recoil;
- bolt action;
- pump action;
- lever action;
- belt-fed action;
- launcher/single-shot behavior;
- weapon-specific selector availability.

Current exact source action declarations cover AK-47, MP5, M1911, M700, Remington 870, M249, M14, MAC-10, TEC-9, Lever Action .45-70 and the anti-armor launcher.

Selector rules now enforced in source:

- no universal automatic-fire rule inherited merely from a generic weapon class;
- exact tuning controls supported selector positions;
- `B` cycles only supported modes and skips unsupported positions;
- `EOCFireMode::Burst3` is opt-in rather than silently granted to all weapons;
- no current production weapon has `bSupportsBurst3=true` without an explicitly accepted exact selector configuration;
- when Burst3 is eventually enabled for an accepted variant, the server owns a finite maximum three-shot sequence;
- an accepted burst is not truncated merely because LMB is released between its pulses;
- a second press cannot stack another burst while the current finite sequence is active;
- sprint/reload/equip/drop/death use the hard fire-stop path and clear pending burst state;
- source marker: `PASS45_BURST3_SEQUENCE_READY authoritative=1 finite_shots=3 release_cancel=0`.

Still pending:

- HUD must clearly display current fire mode/action state;
- no current weapon may be declared Burst3-capable until its exact modeled selector is factually accepted;
- bolt/pump/lever cycle time must become real post-shot weapon action state/presentation, not merely a low-RPM approximation;
- action-specific animation/audio timing remains runtime-unverified.

External factual references for MP5/M14/MAC-10/TEC-9/M700/870/M249 are preserved in the 2026-08-26 evidence README. The game configuration must follow the exact modeled variant, not a broad family label.

Acceptance matrix must include every runtime weapon class and list:

`weapon id | exact visual/fallback | action type | allowed fire modes | default mode | RPM/cycle time | magazine | audio profile | ADS profile | muzzle owner`

Source guard:

`VERIFY_PASS45_WEAPON_ACTION_MATRIX.py`

Runtime acceptance additionally requires deliberate tests for Semi, Auto and any exact Burst3-capable weapon eventually enabled, plus observable bolt/pump/lever cycle behavior for manual-action weapons.

## 5. P0 — ADS / sight alignment

Latest screenshots show inconsistent first-person alignment. ADS cannot be one generic transform for all weapons.

Requirements:

- per-weapon ADS profile;
- camera -> rear sight/optic -> front sight -> intended aim line must agree;
- production mesh orientation and scale must be considered before applying ADS transform;
- no camera inside receiver/stock/optic geometry;
- hip and ADS transitions must not shift ballistic aim unpredictably;
- iron sights and optics require separate alignment data where appropriate;
- FOV interpolation cannot substitute for actual sight alignment.

Acceptance:

- dedicated front/ADS screenshot for every available weapon;
- debug aim line intersects the intended sight axis;
- no visible clipping through weapon geometry.

## 6. P0 — weapon drop physics

### Runtime rejection

Dropped weapons can leave the hands and remain floating instead of falling and settling on the ground.

### Exact source cause found

Previous `AOCWeaponBase::ApplyWorldPickupPresentation()` explicitly set `WeaponMesh->SetSimulatePhysics(false)`, and the actor used a non-physical scene root. A child physics body therefore could not own the complete rendered weapon transform correctly.

### Current corrective source state — CODED_UNTESTED

- `WeaponMesh` is now the actor physics root;
- `WeaponRoot` remains the stable production-visual attach point;
- deliberate `DropToWorldServer()` enables collision, gravity and rigid-body simulation on authority;
- inherited carrier velocity is preserved;
- a modest angular velocity is applied so a dropped weapon is not frozen in an artificial pose;
- static/rack world pickups are not globally forced into simulation merely because they are interactable pickups;
- equip disables physics and clears residual velocities.

Acceptance:

- drop from standing, walking and running;
- weapon falls under gravity;
- collides with ground/geometry;
- does not tunnel through floor;
- settles naturally and sleeps;
- replicated clients observe the settled transform;
- production visual stays attached to the collision/physics body;
- pickup after settling works normally.

## 7. P0 — visible primitive weapon/pickup geometry

Current source still contains `BuildSourceOnlyWeaponVisual()` using Engine Cube/Cylinder/BasicShape geometry. That code is fallback/prototype history, not acceptable final visible content.

Requirements:

- Cube/Cylinder/Sphere/BasicShape may remain only for invisible collision/debug roles;
- once an accepted real visual exists, all source primitive visual parts must remain hidden in pickup, equipped and dropped states;
- runtime validator must fail if a primitive fallback becomes visible for a required-available weapon;
- MAC-10 pickup must have readable real scale rather than appearing as a tiny speck;
- no weapon may be represented by anonymous boxes/cylinders on the rack.

Long-term migration:

- retire visible `BuildSourceOnlyWeaponVisual()` production fallback path entirely after every required gameplay class has a truthful visual policy (`exact production` or explicitly labelled real fallback).

## 8. P0 — weapon audio

Current audio subsystem supports confirmed shot/state/impact events, but some runtime weapons remain silent.

Requirements:

- every accepted weapon visual must resolve a non-empty factual audio profile or be explicit `AUDIO CONTENT GAP`;
- no silent production READY state;
- shot sound originates from production muzzle;
- reload, dry fire, fire-mode switch and mechanical action events are weapon/action appropriate;
- shot sound is emitted only after accepted shot;
- no sound when cadence gate rejects a shot;
- local mechanical audio and world muzzle report must not double-count as two gunshots.

Acceptance:

- per-weapon audio audit with outdoor and indoor shot;
- reload/dry-fire/mode-switch coverage where applicable;
- no silent weapon among accepted required-available classes.

## 9. P0 — grenade model, throw physics and smoke VFX

Latest runtime presentation is prototype-grade and rejected.

Requirements:

- real recognizable grenade model for fragmentation/smoke/flash classes;
- visible in-hand/throw presentation rather than an anonymous primitive;
- gravity, collision, bounce and roll;
- fuse starts at defined gameplay event;
- grenade may not remain floating;
- smoke grows volumetrically over time, has believable density/dispersion and blocks sight appropriately for gameplay;
- smoke cannot be a small static sprite/blob;
- fragmentation/flash/smoke VFX and audio are distinct;
- throw origin must not intersect the player's own capsule/weapon.

Acceptance:

- first-person throw capture;
- bounce/settle capture;
- smoke progression screenshots at early/mid/full state;
- no BasicShape visible as final grenade.

## 10. P0 — Museum / Culture House / Silpo ownership and identity

Latest screenshots still reject landmark composition.

Requirements:

- Oster Local History Museum and Culture House are separate visible buildings at separate canonical sites;
- six-column Culture-House facade at/inside Museum site = hard FAIL;
- exactly one mutating visible shell owner per landmark;
- late validation/detail systems may not relocate, replace or hide the authoritative shell;
- obsolete shell/recovery/replacement owners are deleted, not kept dormant;
- Silpo identity/sign belongs only to the canonical Silpo site and must not remain attached to the wrong landmark;
- Museum/Culture/Silpo each receive separate runtime identity markers and screenshot evidence.

Acceptance:

- Museum screenshot with no six-column Culture-House facade;
- Culture House screenshot at its own site;
- Silpo screenshot at its own site with identity/sign correctly owned;
- source audit proves no second mutating owner can overwrite these after startup.

## 11. P0 — vegetation replacement

Latest trees remain visually rejected: malformed thick trunks, repeated silhouettes, primitive/blob crowns and weak reference fidelity.

Requirements:

- do not cosmetically retint the rejected tree family and call it fixed;
- replace the actual runtime tree meshes/family;
- no Cylinder/Sphere fantasy tree forest;
- use verified real conifer/pine assets where suitable;
- oak remains explicit `CONTENT GAP` until a suitable real asset is verified;
- placement/species character follows supplied Oster references;
- remove obvious repeated identical rotations/scales in the visible test area;
- LOD transitions may not collapse trees into crude silhouettes at ordinary combat distances.

Acceptance:

- close/mid/far screenshots;
- no rejected blob/tree proxy family visible;
- reference trace recorded for each accepted major vegetation family.

## 12. P0 — visual fidelity / no prototype acceptance

The latest scene is brighter and more stable but still looks prototype-grade. Stable 60 FPS is not permission to ship primitive visuals.

Rejected visual traits:

- flat low-detail ground;
- weak material variation/detail;
- visible proxy geometry;
- primitive weapon/grenade objects;
- malformed vegetation;
- landmark composition that does not match reference identity;
- crude/floating turret parts;
- obvious white/default material regions;
- aggressive or visibly bad LOD substitution.

Requirements:

- native render scale stays intact unless a separately approved performance decision changes it;
- material fidelity, geometry fidelity, lighting readability and LOD quality are assessed together;
- no performance verifier may declare visual acceptance solely from FPS/frame time;
- reference-faithful replacement beats another layer of procedural disguise over rejected geometry.

Acceptance Gate K:

- no visible BasicShape production fallback;
- no white/default material surface on accepted production content;
- no rejected tree family;
- no wrong landmark identity;
- no large flat/proxy regions in the core photographed gameplay area;
- direct screenshots are mandatory.

## 13. P0 — HMMWV movement and M2 turret station

### HMMWV

Confirmed improvement: visual forward direction now reads correctly and the vehicle drives forward normally.

Remaining requirements:

- road gameplay top speed **at least 80 km/h** under normal healthy vehicle state;
- acceleration/braking must remain controllable rather than instantly snapping to speed;
- visual wheel/body proportions remain coherent;
- no regression to reversed forward axis.

External reference note: the evidence README preserves the AM General performance reference used to establish that an 80 km/h game target is not physically absurd for the represented HMMWV family.

### M2 Browning station

Latest runtime is rejected: shield, mount and weapon appear detached/clipped/floating, and gunner view poorly exposes the actual Browning.

Required hierarchy:

`vehicle roof mount -> rotating ring/shield/gunner station -> elevation cradle -> M2 weapon`

Requirements:

- ring/shield/gunner station rotates as one assembly in yaw;
- selected project configuration requires full **360° yaw traversal**;
- weapon elevation occurs inside the cradle without tearing shield/mount hierarchy apart;
- Browning is visibly centered/aligned on mount;
- no floating/exploded shield or weapon components;
- gunner camera is attached to the station and keeps the Browning usable/visible;
- normal vertical aim is not inverted;
- firing/releasing does not create persistent downward camera drift;
- one consistent turret pivot owns yaw.

Acceptance:

- exterior 0°/90°/180°/270° turret screenshots;
- interior/gunner camera screenshot;
- full 360° traversal test;
- elevation test;
- firing/release recoil test.

## 14. P0 — BTR-4 material, orientation and remote operator view

Latest runtime remains rejected.

Failures:

- large white/default material region remains on BTR upper/front hull;
- after possession a major portion can become white/default;
- visual/physics forward direction remains suspect/reversed from user observation;
- current gunner/turret camera is illogical for the represented remote weapon station.

Requirements:

- every BTR material slot must reopen and retain non-placeholder dependencies before gameplay;
- possession may not change production material ownership or replace authored materials;
- pre-enter, post-enter and post-movement material state must be identical unless an intentional damage material is active;
- vehicle forward axis, input forward axis, wheel/drive physics and visual front must agree;
- BM-7 Parus gameplay uses an **interior remote-operator optic/monitor presentation**;
- operator remains inside hull rather than pretending to put their head through the external weapon module;
- monitor/optic UI must show aiming reticle, weapon state/ammunition and useful sight picture;
- external turret yaw/elevation follows remote operator input independently of interior camera body placement.

Acceptance:

- BTR front/side/rear screenshots before entry;
- same angles after possession;
- forward driving test;
- operator monitor screenshot;
- no white/default region in any accepted state.

## 15. P0 — world daylight/material stability

Earlier black-world rejection must not return.

Current source contract:

- Directional Light approximately `120000 lux`;
- `r.DefaultFeature.AutoExposure=True`;
- `r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange=True`;
- `AOCWorldSectorOster` owns semantic Ground/Roads/Sidewalks material baseline;
- Pass12 checks semantic MID/`Color` stability through runtime window.

Strict evidence requires:

- `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`;
- `PASS12_WORLD_GEOMETRY_STABLE`;
- `PASS45_WORLD_MATERIAL_STABLE`;
- absence of `PASS12_WORLD_GEOMETRY_STABILITY_FAIL`.

Acceptance:

- no large black world regions;
- no blown-out white scene;
- readable ground/roads/sidewalks;
- direct runtime screenshot.

## 16. P0 — vehicle possession/exit must never teleport to Museum

Retain the current initial-character-only BASE recovery architecture.

Requirements:

- ordinary character -> vehicle -> character possession is never treated as fresh BASE deployment;
- `EnterDriver` preserves current vehicle transform;
- exit places pawn beside vehicle's current transform;
- no generic `RestartPlayer` Museum fallback on normal vehicle exit;
- repeat with civilian vehicle, HMMWV and BTR.

Any return of Museum teleport = hard runtime FAIL.

## 17. P1 — reference-driven houses/fences/tower retirement

Requirements:

- no rejected generic AdvancedVillagePack/OCEnterableHouse-style family may return as Oster-authentic production content without reference support;
- no unreferenced dark steep-roof tower/shack;
- fences must match supplied Oster reference character;
- previously retired generic residential/fence generators remain physically retired;
- reference-driven placement beats synthetic filler.

Gate E runtime evidence remains mandatory.

## 18. P1 — weapon material/texture closure

Required chain:

`weapon class -> exact production visual OR explicit real fallback -> material slots -> authored material -> real texture dependencies -> fresh load -> runtime appearance`

Rules:

- white/default/BasicShape = FAIL;
- zero used texture dependencies = FAIL;
- fallback stays exact-production `CONTENT GAP`, never fake READY;
- mesh-load success alone is not material readiness;
- stale `.uasset` existence is not current revision proof.

Current Stein contract:

- R3 deterministic authored material from committed PNG source;
- independent fresh UE process validates material/texture dependencies;
- rendered rack screenshot still authoritative.

Explicit exact-production gaps unless later content closes them:

- Remington 870 exact payload;
- M249 exact payload;
- M16/M4 exact payload.

## 19. P1 — display, FPS and thermal behavior

Confirmed improvement: current screenshots report approximately 60 FPS.

Requirements:

- normal route uses intended fullscreen/borderless state;
- strict recovery cap remains approximately 60 FPS;
- no render-scale downgrade used to fake performance;
- after visual fidelity is restored, run at least a 10-minute soak containing infantry movement, repeated firing, HMMWV and BTR use;
- monitor for progressive frame-time collapse or abnormal heat behavior;
- `60 FPS` in an empty/proxy scene is not final performance acceptance.

## 20. Tactical map

Requirements remain:

- north-up;
- compact central-Oster playable topology;
- one geo-reference authority;
- visible player marker;
- Museum / Culture House / Silpo / Stadium clearly distinct;
- no return of giant synthetic diagonal/X road topology.

Runtime `M` screenshot remains mandatory.

## 21. Stale-owner retirement

Must remain true:

- `OCWorldProductionVisualsSubsystem` stays physically deleted;
- stale completion verifier/workflow cannot require it back;
- Museum/world/material/spawn responsibilities each have one current mutating owner;
- obsolete conflicting owners are physically deleted together with stale verifier expectations;
- historical pass numbering never grants mutation authority.

## 22. Current source implementation milestone — 2026-08-26 weapon firing/drop/action pass

State: **CODED_UNTESTED / CURRENT-HEAD SOURCE VERIFICATION PENDING / NOT RUNTIME ACCEPTED**.

Implemented on PR #94 after the latest screenshot rejection:

- production muzzle resolver based on the visible `OC_ProductionWeaponVisual` component;
- base weapon muzzle flash/tracer/audio presentation moved away from camera `TraceOrigin`;
- anti-armor projectile/FX/audio moved to production muzzle;
- anti-armor projectile spawn failure no longer consumes ammo;
- fail-visible anti-armor production visual markers;
- deliberate dropped-weapon rigid-body physics with gravity/collision and replicated movement;
- confirmed-shot recoil state moved into `AOCWeaponBase`;
- legacy Character `LocalFireFeedbackTimerHandle` and its independent pitch/yaw recovery state physically retired;
- camera shake and crosshair recoil now derive from confirmed-shot weapon state rather than held input;
- bounded fire-cadence scheduling tolerance;
- data-driven Semi/Burst3/Automatic selector API;
- explicit mechanical action metadata for current weapon variants;
- finite authoritative Burst3 sequence architecture with no current weapon falsely opting in;
- `VERIFY_PASS45_WEAPON_MUZZLE_DROP_PHYSICS.py` updated to reject resurrection of the retired local-feedback owner;
- `VERIFY_PASS45_WEAPON_ACTION_MATRIX.py` updated to require the finite Burst3 sequence contract;
- both guards remain in cumulative `RUN_ALL_VERIFY.py`;
- dedicated `.github/workflows/pass45-weapon-firing-physics.yml` remains active.

This does **not** close the P0 weapon gate. Local UE 5.8 must still prove:

- compile succeeds;
- production visual hierarchy survives the new physics root;
- drops settle correctly;
- muzzle placement is visually correct for every mesh orientation;
- recoil/recovery direction feels correct and has no downward release drift;
- camera shake occurs exactly once per accepted shot;
- finite burst behavior is correct when an exact accepted Burst3-capable variant is eventually enabled;
- bolt/pump/lever post-shot action state and presentation are still missing;
- launcher visual is actually production mesh in first person;
- shot audio assets exist and are audible.

## 23. Corrective execution order

Completed/source-coded items are marked only for source work, not runtime acceptance.

1. [x] Preserve 2026-08-26 screenshots and mark latest runtime **RUNTIME REJECTED**.
2. [x] Promote 2026-08-26 evidence over older runtime verdict wording.
3. [x] Retain black-world daylight/exposure source correction and semantic material stability gate.
4. [x] Retain initial-character-only vehicle BASE recovery architecture.
5. [x] Retain proportional vehicle visual fit and HMMWV forward-axis improvement.
6. [x] Stein R3 authored-material + independent fresh-load source path.
7. [x] Source-audit weapon firing, recoil, muzzle and drop-physics defects from latest screenshots.
8. [x] Code base weapon production-muzzle presentation for FX/audio.
9. [x] Code launcher production-muzzle projectile/FX/audio and no-ammo-on-spawn-failure.
10. [x] Code authority-simulated deliberate weapon drops.
11. [x] Code confirmed-shot recoil migration.
12. [x] Add source verifier/workflow for firing/muzzle/drop contracts.
13. [x] Physically retire legacy Character `LocalFireFeedbackTimerHandle` and duplicate local recoil/recovery owner.
14. [x] Expand fire-mode/action model beyond Semi/Auto, build exact per-weapon mechanical action matrix, and code opt-in finite Burst3 sequencing.
15. [ ] Implement manual bolt/pump/lever post-shot action state, cycle timing and presentation.
16. [ ] Build per-weapon ADS/sight profiles and validation.
17. [ ] Close all silent weapon audio-profile gaps.
18. [ ] Remove visible primitive weapon/pickup/launcher fallbacks from accepted runtime.
19. [ ] Replace grenade models/throw presentation/smoke VFX.
20. [ ] Correct Museum/Culture House/Silpo visible identity and separation.
21. [ ] Replace rejected vegetation family.
22. [ ] Rebuild HMMWV M2 ring/shield/gunner hierarchy with 360° yaw and correct camera.
23. [ ] Calibrate HMMWV gameplay top speed to >=80 km/h without breaking handling.
24. [ ] Close BTR white material state across pre/post possession.
25. [ ] Correct BTR forward axis and remote operator monitor/optic gameplay.
26. [ ] Raise core world/material/LOD visual fidelity above prototype state without lowering native render scale.
27. [ ] Validate fullscreen + 60 FPS + thermal soak after visual fixes.
28. [ ] Validate tactical map screenshot.
29. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots.
30. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 24. Final acceptance gates

Pass 45 cannot become `VERIFIED RUNTIME` until every applicable gate below passes.

### Gate A — source/build/import

- current branch/head matches origin;
- source workflows green;
- Stein R3 authoring + independent fresh load pass;
- production HMMWV/M2/BTR import + fresh load pass;
- UE 5.8 editor target builds exit code 0.

### Gate B — world rendering

- daylight/exposure READY;
- world geometry/material stability READY;
- no black-world corruption;
- no blown-out scene;
- direct screenshot pass.

### Gate C — weapon materials and visible content

- every required-available weapon has real accepted visual/material/texture chain;
- no visible primitive fallback;
- launcher production visual valid;
- unresolved exact production items remain explicit CONTENT GAP.

### Gate D — weapon firing physics

- factual shot count = ammo decrement = recoil count = muzzle event count = audio count;
- muzzle/tracer/projectile at visible barrel;
- no held-input ghost recoil;
- no release downward kick;
- selector exposes only modes supported by the exact weapon;
- any accepted Burst3 weapon produces a deterministic finite three-shot sequence without stacking or accidental truncation;
- bolt/pump/lever actions visibly and temporally cycle before the next legal shot;
- ADS alignment correct;
- dropped weapon physics passes;
- grenade/smoke presentation passes.

### Gate E — landmarks/environment

- Museum/Culture/Silpo separate and correctly identified;
- no rejected generic house/fence/tower family;
- rejected tree family absent;
- visual fidelity Gate K passes.

### Gate F — HMMWV/M2

- HMMWV forward axis correct;
- road top speed >=80 km/h;
- correct proportional body;
- coherent M2 ring/shield/gunner assembly;
- 360° yaw;
- correct elevation and gunner view;
- no inverted aim or release camera drift.

### Gate G — BTR-4

- no white/default material before or after possession;
- forward axis correct;
- proportional visual retained;
- remote interior operator optic/monitor works;
- turret control and camera logic coherent.

### Gate H — possession

- no Museum teleport on vehicle enter/exit for civilian vehicle, HMMWV or BTR.

### Gate I — display/performance/thermal

- intended fullscreen/borderless state;
- ~60 FPS recovery cap verified at runtime;
- native render scale retained;
- 10-minute mixed gameplay soak without severe thermal/frame collapse.

### Gate J — tactical map

- current compact Oster topology screenshot accepted.

### Gate K — visual fidelity

- no visible production BasicShape/proxy content in core test area;
- no major white/default materials;
- acceptable ground/material/vegetation/LOD quality;
- reference-faithful landmark composition;
- screenshots visually accepted by the user.

## 25. Current verdict

**PASS 45 = ACTIVE / RUNTIME REJECTED 2026-08-26.**

PR #94 remains **OPEN / UNMERGED**.

The newest weapon firing/muzzle/drop/action corrections are **CODED_UNTESTED**. They may not be described as fixed in runtime until a current-head local UE 5.8 build and playtest proves them.
