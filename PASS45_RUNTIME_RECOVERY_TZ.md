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

Source guard: `VERIFY_PASS45_WEAPON_MUZZLE_DROP_PHYSICS.py`  
Workflow: `.github/workflows/pass45-weapon-firing-physics.yml`

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

The old Semi/Auto-only abstraction has been replaced at source level with separate selector capability, mechanical-action metadata and an authoritative manual-cycle gate.

Implemented model distinguishes Semi, full Auto, opt-in Burst3, gas-operated, delayed blowback, blowback, short recoil, bolt, pump, lever, belt-fed and single-shot launcher behavior.

Current exact source action declarations cover AK-47, MP5, M1911, M700, Remington 870, M249, M14, MAC-10, TEC-9, Lever Action .45-70 and the anti-armor launcher.

Selector rules now enforced in source:

- exact tuning controls supported selector positions;
- `B` cycles only supported modes and skips unsupported positions;
- Burst3 is opt-in and no current production weapon falsely claims it;
- an accepted Burst3 sequence is finite, server-owned and not truncated by ordinary trigger release;
- a second trigger press cannot stack another burst while the current sequence is active;
- hard stop paths clear pending burst state;
- source marker: `PASS45_BURST3_SEQUENCE_READY authoritative=1 finite_shots=3 release_cancel=0`.

Manual-action source gate now exists independently of low RPM:

- `FOCWeaponTuning::ManualActionCycleSeconds` is explicit post-shot action timing;
- M700 bolt cycle = `1.10 s` game tuning;
- Remington 870 pump cycle = `0.72 s` game tuning;
- Lever Action .45-70 cycle = `0.85 s` game tuning;
- replicated `bActionCycling` owns authoritative state;
- Bolt/Pump/Lever shots start the cycle and another shot, reload or selector mutation is rejected until completion;
- presentation/HUD may observe `bActionCycling` but may not own a second timing source;
- source marker: `PASS45_MANUAL_ACTION_CYCLE_READY ... authoritative=1`.

Manual-action presentation/audio routing is now also source-coded without creating a second gameplay clock:

- `UOCFirstPersonWeaponPresentationSubsystem` detects the replicated `bActionCycling` transition and shapes a local procedural action cue using the authoritative cycle duration;
- M700, Remington 870 and Lever Action profiles declare separate bolt/pump/lever weapon+arms cue transforms while remaining explicitly **UNCALIBRATED** for final UE visual approval;
- source marker: `PASS45_MANUAL_ACTION_PRESENTATION_READY ... replicated_gate=1 second_gameplay_timer=0`;
- `EOCWeaponAudioEvent::ManualActionCycle` routes by exact `EOCWeaponActionType` into separate `BoltCycle`, `PumpCycle` and `LeverCycle` sound sets;
- local first-person mechanical audio is emitted on the local replicated-gate transition; remote listeners use `OnRep_ActionCycling` and explicitly skip the local owner to prevent double playback;
- empty/manual-action sound arrays remain an explicit **AUDIO CONTENT GAP** and are not promoted to READY.

Still pending:

- HUD current mode/action state;
- exact Burst3-capable asset approval if such a variant is introduced;
- authored skeletal bolt/pump/lever animation or exact moving-part presentation for production meshes;
- actual accepted bolt/pump/lever mechanical sound assets in the audio profiles;
- local UE 5.8 timing/feel/visual/audio verification.

Source guard: `VERIFY_PASS45_WEAPON_ACTION_MATRIX.py`

## 5. P0 — ADS / sight alignment

Latest screenshots show inconsistent first-person alignment. ADS cannot be one generic transform for all weapons.

### Current corrective source state — 2026-08-26 — ARCHITECTURE CODED_UNTESTED

The source now has an explicit fail-visible calibration contract instead of allowing generic offsets to masquerade as accepted sights:

- every current runtime weapon id remains registered in `FOCFirstPersonWeaponProfile` resolution;
- each profile now carries optional `ADSRearSightSocket`, `ADSFrontSightSocket`, `ADSOpticSocket` and a separate factual `bADSCalibrated` flag;
- current weapons remain `bADSCalibrated=false` because no exact UE 5.8 sight-socket evidence has yet been accepted;
- entering ADS calls alignment validation once for the active weapon;
- an uncalibrated profile emits `PASS45_ADS_PROFILE_UNCALIBRATED ... no_fake_ready=1` instead of claiming READY;
- a profile marked calibrated but missing its required production visual/socket emits `PASS45_ADS_ALIGNMENT_FAIL`;
- a valid calibrated profile can sample camera-vs-sight angular error and camera-to-sight-line offset via `PASS45_ADS_ALIGNMENT_SAMPLE`;
- debug cvar `oc.Weapon.ADS.Debug 1` draws the camera aim ray and authored sight axis for calibration evidence;
- ADS diagnostics observe presentation only and own no gameplay timer or ballistic authority.

Source guard: `VERIFY_PASS45_WEAPON_ADS_ALIGNMENT.py`

Still pending:

- inspect every exact production mesh in local UE 5.8;
- author/confirm real rear+front or optic sight socket references for each accepted weapon;
- calibrate per-weapon ADS weapon/arms offsets from those factual references;
- visually accept camera -> sight -> intended aim line for each weapon;
- no profile may change `bADSCalibrated=true` without matching evidence/verifier update.

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

Dropped weapons can leave the hands and remain floating instead of falling and settling on the ground.

Current corrective source state — **CODED_UNTESTED**:

- `WeaponMesh` is now the actor physics root;
- `WeaponRoot` remains the stable production-visual attach point;
- deliberate `DropToWorldServer()` enables collision, gravity and rigid-body simulation on authority;
- inherited carrier velocity is preserved;
- static/rack pickups are not globally forced into simulation;
- equip disables physics and clears residual velocities.

Acceptance: standing/walking/running drop, gravity, collision, no floor tunneling, natural settle/sleep, replicated settled transform, production visual attached, pickup works after settling.

## 7. P0 — visible primitive weapon/pickup geometry

Current source still contains `BuildSourceOnlyWeaponVisual()` using Engine Cube/Cylinder/BasicShape geometry. That code is fallback/prototype history, not acceptable final visible content.

Requirements:

- BasicShape may remain only for invisible collision/debug roles;
- accepted real visuals must keep source primitive parts hidden in pickup/equipped/dropped states;
- runtime validator fails if a primitive fallback becomes visible for a required-available weapon;
- MAC-10 pickup must have readable real scale;
- no anonymous boxes/cylinders on accepted weapon racks.

## 8. P0 — weapon audio

Current audio subsystem supports confirmed shot/state/impact events, but some runtime weapons remain silent.

Current source state for manual actions:

- manual-action mechanical audio is a distinct event from the gunshot report;
- exact action type selects `BoltCycle`, `PumpCycle` or `LeverCycle` rather than one generic mechanical sound;
- the local-owner and remote-listener routes are separated so one action does not intentionally double-count;
- missing profile/empty action set remains observable as a content gap; source routing alone is not audible-content acceptance.

Requirements:

- every accepted weapon visual resolves non-empty factual audio or explicit `AUDIO CONTENT GAP`;
- no silent production READY state;
- shot audio originates from production muzzle and only after accepted shot;
- reload/dry fire/mode switch/mechanical action events are appropriate;
- rejected cadence pulses create no shot sound;
- local mechanical audio and world muzzle report do not double-count as two gunshots.

## 9. P0 — grenade model, throw physics and smoke VFX

Requirements:

- recognizable fragmentation/smoke/flash models;
- visible first-person throw presentation;
- gravity/collision/bounce/roll;
- defined fuse start;
- no floating grenade;
- growing volumetric smoke with useful sight blocking;
- distinct frag/flash/smoke VFX and audio;
- safe throw origin outside player collision/weapon.

## 10. P0 — Museum / Culture House / Silpo ownership and identity

Requirements:

- Museum and Culture House are separate visible buildings at separate canonical sites;
- six-column Culture-House facade at Museum site = hard FAIL;
- exactly one mutating visible shell owner per landmark;
- late validators/details may not replace/relocate authoritative shells;
- Silpo identity/sign belongs only to canonical Silpo site;
- each landmark needs separate runtime identity and screenshot evidence.

## 11. P0 — vegetation replacement

Requirements:

- replace, do not merely recolor, rejected tree family;
- no Cylinder/Sphere fantasy forest;
- verified real conifer/pine assets where suitable;
- oak remains explicit CONTENT GAP until verified;
- placement/species follow Oster references;
- avoid obvious repeated rotations/scales and crude LOD collapse.

## 12. P0 — visual fidelity / no prototype acceptance

Stable 60 FPS is not permission to ship primitive visuals.

Gate K requires no visible production BasicShape/proxy content, no major white/default materials, acceptable ground/material/vegetation/LOD quality, reference-faithful landmark composition and direct screenshots.

## 13. P0 — HMMWV movement and M2 turret station

HMMWV forward direction improved, but road top speed must be at least 80 km/h without breaking handling.

M2 required hierarchy:

`vehicle roof mount -> rotating ring/shield/gunner station -> elevation cradle -> M2 weapon`

Requirements include coherent assembly, full 360° yaw for selected project configuration, correct elevation, no floating parts, useful gunner camera, non-inverted vertical aim and no release camera drift.

## 14. P0 — BTR-4 material, orientation and remote operator view

Requirements:

- no white/default material before or after possession;
- material ownership stable through possession/movement;
- visual/physics/input forward axes agree;
- BM-7 Parus uses interior remote-operator optic/monitor presentation;
- external turret follows remote operator aim independently of interior camera placement.

## 15. P0 — world daylight/material stability

Current source contract:

- Directional Light ~`120000 lux`;
- auto exposure enabled with extended luminance range;
- semantic Ground/Roads/Sidewalks material baseline;
- strict runtime evidence requires daylight, geometry-stable and world-material-stable markers and rejects stability FAIL marker.

## 16. P0 — vehicle possession/exit must never teleport to Museum

Ordinary character -> vehicle -> character possession is never fresh BASE deployment. Repeat exit tests with civilian vehicle, HMMWV and BTR. Any Museum teleport is hard FAIL.

## 17. P1 — reference-driven houses/fences/tower retirement

No rejected generic residential/fence/tower family may return as Oster-authentic production content without reference support. Retired generic generators remain physically retired.

## 18. P1 — weapon material/texture closure

Required chain:

`weapon class -> exact production visual OR explicit real fallback -> material slots -> authored material -> real texture dependencies -> fresh load -> runtime appearance`

White/default/BasicShape or zero texture dependencies = FAIL. Fallback stays exact-production CONTENT GAP, never fake READY. Current Stein path remains R3 authored material + independent fresh UE load.

Explicit exact-production gaps unless later content closes them: Remington 870, M249, M16/M4.

## 19. P1 — display, FPS and thermal behavior

Current screenshots report ~60 FPS. Final acceptance still needs intended fullscreen/borderless, native render scale, and at least 10-minute mixed infantry/weapon/HMMWV/BTR thermal soak after visual fidelity is restored.

## 20. Tactical map

North-up, compact central Oster topology, one geo-reference authority, player marker, distinct Museum/Culture/Silpo/Stadium, no giant synthetic diagonal/X topology. Runtime `M` screenshot mandatory.

## 21. Stale-owner retirement

- `OCWorldProductionVisualsSubsystem` stays physically deleted;
- stale completion verifier/workflow cannot require it back;
- Museum/world/material/spawn responsibilities each have one current mutating owner;
- obsolete conflicting owners are physically deleted together with stale verifier expectations.

## 22. Current source implementation milestone — 2026-08-26 weapon firing/drop/action/ADS pass

State: **CODED_UNTESTED / CURRENT-HEAD SOURCE VERIFICATION PENDING / NOT RUNTIME ACCEPTED**.

Implemented:

- production muzzle resolver and view-ray/presentation separation;
- launcher production-muzzle projectile/FX/audio and no-ammo-on-spawn-failure;
- deliberate dropped-weapon rigid-body physics;
- confirmed-shot recoil state in `AOCWeaponBase`;
- legacy Character `LocalFireFeedbackTimerHandle` and duplicate recoil/recovery owner physically retired;
- confirmed-shot camera shake and crosshair recoil ownership;
- bounded cadence tolerance;
- data-driven Semi/Burst3/Automatic selector;
- exact mechanical action metadata;
- finite authoritative Burst3 architecture with no current false opt-in;
- M700/Remington870/LeverAction explicit post-shot cycle timings;
- replicated `bActionCycling` authoritative action gate;
- procedural first-person bolt/pump/lever cues driven only by that replicated gate and authoritative duration;
- `PASS45_MANUAL_ACTION_PRESENTATION_READY` proves the presentation path adds no second gameplay timer;
- exact manual-action mechanical audio routing through `BoltCycle` / `PumpCycle` / `LeverCycle` with explicit empty-set content-gap behavior;
- local/remote manual-action audio ownership split to avoid intentional double playback;
- explicit per-weapon ADS socket-reference fields and separate factual `bADSCalibrated` state;
- fail-visible ADS entry diagnostics through `PASS45_ADS_PROFILE_UNCALIBRATED` / `PASS45_ADS_ALIGNMENT_FAIL` / `PASS45_ADS_ALIGNMENT_SAMPLE`;
- `oc.Weapon.ADS.Debug` calibration rays for camera vs authored sight axis;
- source verifiers reject resurrection of old feedback/action shortcuts and fake ADS calibration;
- cumulative `RUN_ALL_VERIFY.py` includes weapon firing, action and ADS guards.

Still not runtime accepted: compile on local UE 5.8, recoil feel/release, action timing, procedural cue quality, authored bolt/pump/lever moving-part animation, real mechanical sound content, exact per-weapon sight socket/offset calibration, production hierarchy, drop settling, muzzle alignment, launcher visual and general audio availability.

## 23. Corrective execution order

Completed/source-coded items are marked only for source work, not runtime acceptance.

1. [x] Preserve 2026-08-26 screenshots and mark latest runtime **RUNTIME REJECTED**.
2. [x] Promote 2026-08-26 evidence over older runtime verdict wording.
3. [x] Retain black-world daylight/exposure source correction and semantic material stability gate.
4. [x] Retain initial-character-only vehicle BASE recovery architecture.
5. [x] Retain proportional vehicle visual fit and HMMWV forward-axis improvement.
6. [x] Stein R3 authored-material + independent fresh-load source path.
7. [x] Source-audit weapon firing, recoil, muzzle and drop-physics defects.
8. [x] Code base weapon production-muzzle presentation for FX/audio.
9. [x] Code launcher production-muzzle projectile/FX/audio and no-ammo-on-spawn-failure.
10. [x] Code authority-simulated deliberate weapon drops.
11. [x] Code confirmed-shot recoil migration.
12. [x] Add source verifier/workflow for firing/muzzle/drop contracts.
13. [x] Physically retire legacy Character `LocalFireFeedbackTimerHandle` and duplicate local recoil/recovery owner.
14. [x] Expand fire-mode/action model beyond Semi/Auto, build exact per-weapon mechanical action matrix, and code opt-in finite Burst3 sequencing.
15. [x] Code replicated-gate first-person bolt/pump/lever procedural presentation and exact action-type mechanical audio routing without a second gameplay timer.
16. [ ] Replace procedural manual-action cues with accepted authored moving-part/skeletal presentation where production assets support it, and populate real bolt/pump/lever sound content.
17. [x] Build fail-visible per-weapon ADS/sight profile architecture, socket-based alignment diagnostics and source validation without inventing calibration data.
18. [ ] Calibrate exact rear/front/optic references and ADS transforms for every accepted production weapon in local UE 5.8; only then set factual `bADSCalibrated=true` per weapon.
19. [ ] Close all remaining silent weapon audio-profile gaps.
20. [ ] Remove visible primitive weapon/pickup/launcher fallbacks from accepted runtime.
21. [ ] Replace grenade models/throw presentation/smoke VFX.
22. [ ] Correct Museum/Culture House/Silpo visible identity and separation.
23. [ ] Replace rejected vegetation family.
24. [ ] Rebuild HMMWV M2 ring/shield/gunner hierarchy with 360° yaw and correct camera.
25. [ ] Calibrate HMMWV gameplay top speed to >=80 km/h without breaking handling.
26. [ ] Close BTR white material state across pre/post possession.
27. [ ] Correct BTR forward axis and remote operator monitor/optic gameplay.
28. [ ] Raise core world/material/LOD visual fidelity above prototype state without lowering native render scale.
29. [ ] Validate fullscreen + 60 FPS + thermal soak after visual fixes.
30. [ ] Validate tactical map screenshot.
31. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots.
32. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 24. Final acceptance gates

### Gate A — source/build/import
current branch/head; source workflows; Stein R3 fresh load; production HMMWV/M2/BTR import/fresh load; UE 5.8 editor build exit 0.

### Gate B — world rendering
daylight/exposure; stable ground/roads/sidewalks; no black-world or blown-out scene; screenshots.

### Gate C — weapon materials and visible content
real accepted visual/material/texture chain; no visible primitive fallback; launcher visual valid; unresolved exact items remain CONTENT GAP.

### Gate D — weapon firing physics
factual shot count = ammo = recoil = muzzle = audio; production muzzle origin; no ghost recoil; no release downward kick; exact selector modes; deterministic finite Burst3 if enabled; manual-action server gate plus accepted visible action animation and audible action-specific content; exact per-weapon ADS alignment accepted; drop physics; grenade/smoke presentation.

### Gate E — landmarks/environment
Museum/Culture/Silpo separated and identified; rejected residential/tree families absent; Gate K passes.

### Gate F — HMMWV/M2
forward axis; >=80 km/h; proportional body; coherent ring/shield/gunner; 360° yaw; correct elevation/view; no inverted aim/drift.

### Gate G — BTR-4
no white/default material; forward axis; proportional visual; remote interior optic/monitor; coherent turret/camera.

### Gate H — possession
no Museum teleport on civilian vehicle/HMMWV/BTR enter/exit.

### Gate I — display/performance/thermal
intended fullscreen; ~60 FPS; native render scale; 10-minute mixed soak.

### Gate J — tactical map
current compact Oster topology screenshot accepted.

### Gate K — visual fidelity
no production BasicShape/proxy core content; no major white/default materials; acceptable world/vegetation/LOD; reference-faithful landmarks; screenshots accepted.

## 25. Current verdict

**PASS 45 = ACTIVE / RUNTIME REJECTED 2026-08-26.**

PR #94 remains **OPEN / UNMERGED**.

The newest weapon firing/muzzle/drop/action/presentation/audio-routing/ADS-diagnostic corrections are **CODED_UNTESTED**. They may not be described as fixed in runtime until a current-head local UE 5.8 build and playtest proves them.
