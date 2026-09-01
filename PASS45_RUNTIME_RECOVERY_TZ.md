# OSTER CONFLICT — PASS 45 RUNTIME RECOVERY TZ

Date opened: 2026-08-24  
Latest factual gameplay evidence: 2026-08-31
Latest runtime verdict: **RUNTIME REJECTED 2026-08-31**
Target: Unreal Engine 5.8.x / Windows  
Canonical user launcher: `START_HERE.cmd`  
Active branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Active PR: **#94 OPEN / UNMERGED**  
Integrated baseline: `main` @ `bca00f4046700f383af9f1742cc24b6a62401b1a`

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

`RUNTIME_EVIDENCE/2026-08-27_PASS45_REJECTED/`

Detailed latest screenshot map / user observations:

`RUNTIME_EVIDENCE/2026-08-27_PASS45_REJECTED/README.md`

The 2026-08-27 direct screenshots outrank older green source tests and older preview/source optimism. They explicitly keep the world visual state, AK/hand ADS presentation, M2 presentation/alignment, BTR presentation/orientation and plaza/landmark/ground quality rejected until current-head runtime proof supersedes them.

Historical evidence remains preserved separately:

- `RUNTIME_EVIDENCE/2026-08-26_PASS45_REJECTED/`
- `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/`
- `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`

Location-specific visual evidence is bound through the repository-controlled subordinate index:

`PASS45_REFERENCE_PACK_BINDINGS.md`

That binding index and its active location specs are normative for Gate E/K, but do not replace this TZ as execution/status owner and do not themselves constitute UE 5.8 runtime acceptance.

## 0A. Latest P0 startup blocker — 2026-08-31 Quick Normal black screen

The latest factual local UE 5.8 evidence is now a startup rejection, newer than the 2026-08-27 rendered visual pack:

- `START_HERE.cmd` -> `1. ЗВИЧАЙНА ГРА` completed the incremental C++ build with `Result: Succeeded`;
- the direct `OsterConflict_Runtime` game process then opened a black window and never produced usable gameplay/UI;
- the log reached `PASS45_RENDER_BUDGET_READY` and `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`, then entered KiteDemo tree material/static-mesh work;
- observed material diagnostics include `Failed to compile Material for platform PCD3D_SM5`, `Node TransformPosition input must be a 3-component vector` and `SpeedTree node not currently supported for Skeletal Meshes`;
- the final blocking path visibly includes `Building static mesh HillTree_02` / `Waiting for static meshes to be ready`;
- source audit proves `AOCWorldSectorOster` synchronously resolved `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01` via `ConstructorHelpers::FObjectFinder` in the native actor constructor/CDO and immediately consumed tree bounds during `BuildVegetation()`.

P0 recovery rule from this evidence:

- no KiteDemo production-tree package may be synchronously resolved from the native constructor/CDO;
- normal runtime must be able to render its first frame without touching those rejected UE 5.8 material/static-mesh compile dependencies;
- the exact tree family remains a factual runtime/content gap while quarantined; source identity alone is not acceptance;
- an opt-in deferred async load may be used only for targeted repair evidence and remains `runtime_acceptance=0` until the material/static-mesh path and direct UE 5.8 visual result pass;
- PR #94 remains OPEN / UNMERGED.

The source recovery for this blocker is tracked by `VERIFY_PASS45_TREE_STARTUP_DEFERRED.py`. A new local Quick Normal launch is required before any startup/runtime status can improve.

## 1. Latest factual runtime state — 2026-08-27

The current branch reaches actual gameplay. Several earlier failures improved, but the rendered/gameplay result is still unacceptable. The 2026-08-27 direct screenshots are the latest factual runtime visual verdict and supersede any older source-only claim of visual readiness.

### Confirmed improvements that must not regress

- gameplay launches;
- the previous near-black world corruption is no longer the dominant rendered state;
- AK-family/M14/MP5/Lever Action and several other firearm meshes/materials now render as recognizable authored assets;
- HMMWV visual forward direction is now coherent enough that it drives forward normally;
- HUD evidence shows the recovery **60 FPS** cap functioning;
- the previous uncapped roughly 100–156 FPS runaway state was not reported in the preceding accepted partial run;
- HMMWV/BTR/M2 production asset intake reaches gameplay rather than failing at the old source/build barriers.

These are partial improvements only. **PASS 45 remains RUNTIME REJECTED.** The latest 2026-08-27 screenshots specifically keep AK/hand ADS, M2, BTR and core-world/landmark visual fidelity open.

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
- all source StaticMesh/BasicShape launcher components are hidden synchronously at `BeginPlay()` **before** the production `LoadObject` attempt;
- failure to load/create the production visual now emits `PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL ... primitive_visible=0 runtime_acceptance=0` rather than returning with the rejected tube/cube still rendered;
- success emits `PASS45_LAUNCHER_PRODUCTION_VISUAL_READY ... primitive_visible=0 production_visual=1`;
- launcher projectile spawn resolves from the production muzzle rather than `TraceOrigin + Dir * 90`;
- launcher ammo is committed only after projectile spawn succeeds;
- confirmed launcher shot emits muzzle FX and weapon shot audio;
- confirmed event marker: `PASS45_LAUNCHER_CONFIRMED_SHOT`.

Acceptance:

- launcher is recognizable in pickup and first-person states;
- no visible Cube/Cylinder/BasicShape fallback;
- projectile leaves visible muzzle;
- one shot consumes one round and emits one shot-audio event;
- failed projectile spawn consumes zero ammunition and applies zero recoil.

## 4. P0 — data-driven weapon actions and fire modes

### Current corrective source state — 2026-09-01 — SOURCE-CODED / RUNTIME REJECTED

The Semi/Auto-only abstraction has been replaced at source level with separate selector capability, mechanical-action metadata and an authoritative manual-cycle gate.

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

Manual-action source gate exists independently of low RPM:

- `FOCWeaponTuning::ManualActionCycleSeconds` is explicit post-shot action timing;
- M700 bolt cycle = `1.10 s` game tuning;
- Remington 870 pump cycle = `0.72 s` game tuning;
- Lever Action .45-70 cycle = `0.85 s` game tuning;
- replicated `bActionCycling` owns authoritative state;
- Bolt/Pump/Lever shots start the cycle and another shot, reload or selector mutation is rejected until completion;
- presentation/HUD may observe `bActionCycling` but may not own a second timing source;
- source marker: `PASS45_MANUAL_ACTION_CYCLE_READY ... authoritative=1`.

Manual-action presentation/audio routing is now fail-closed rather than procedural:

- `UOCFirstPersonWeaponPresentationSubsystem` observes the replicated `bActionCycling` transition and routes local action audio from that factual state;
- exact authored action animation is resolved only through `FOCWeaponAnimationProfile::ManualActionAnimationObjectPath` and played only if the sequence loads and matches the production skeletal mesh/skeleton;
- M700, Remington 870 and Lever Action are explicitly marked as requiring authored manual-action coverage, while their exact action sequence slots remain empty until factual content is committed;
- the old whole-weapon/arms sine fallback, `ManualAction*` profile displacement fields, `ActionCycleStartTime` presentation state and `PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE` path are physically retired;
- missing/incompatible authored action content emits `PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP` or `PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL`, preserves the baseline weapon+arms transform, and remains `runtime_acceptance=0`;
- `PASS45_MANUAL_ACTION_PRESENTATION_READY` is not a valid production marker;
- `EOCWeaponAudioEvent::ManualActionCycle` routes by exact `EOCWeaponActionType` into separate `BoltCycle`, `PumpCycle` and `LeverCycle` sound sets;
- local first-person mechanical audio is emitted on the local replicated-gate transition; remote listeners use `OnRep_ActionCycling` and explicitly skip the local owner to prevent double playback;
- PumpCycle can use tracked `/Game/R13/Audio/shotguncock`; BoltCycle and LeverCycle remain explicit **AUDIO CONTENT GAP**.

Still pending:

- HUD current mode/action state;
- exact Burst3-capable asset approval if such a variant is introduced;
- accepted authored skeletal/moving-part M700 bolt, Remington 870 pump and Lever Action lever sequences;
- accepted factual bolt/lever mechanical sound assets and final per-weapon mechanical mix;
- local UE 5.8 timing/feel/visual/audio verification.

Source guard: `VERIFY_PASS45_WEAPON_ACTION_MATRIX.py`

## 5. P0 — ADS / sight alignment

Latest screenshots show inconsistent first-person alignment. ADS cannot be one generic transform for all weapons.

### Current corrective source state — 2026-08-28 — ARCHITECTURE SOURCE-CODED / RUNTIME REJECTED

The source now has an explicit fail-visible calibration contract instead of allowing generic offsets to masquerade as accepted sights:

- every current runtime weapon id remains registered in `FOCFirstPersonWeaponProfile` resolution;
- each profile carries optional `ADSRearSightSocket`, `ADSFrontSightSocket`, `ADSOpticSocket` and a separate factual `bADSCalibrated` flag;
- current weapons remain `bADSCalibrated=false` because no exact UE 5.8 sight-socket evidence has yet been accepted;
- entering requested ADS calls alignment validation once for the active weapon;
- an uncalibrated profile emits `PASS45_ADS_PROFILE_UNCALIBRATED ... no_fake_ready=1` instead of claiming READY;
- after the 2026-08-27 AK/hand rejection, uncalibrated requested ADS is **presentation fail-closed**: gameplay aiming state may remain authoritative elsewhere, but guessed ADS weapon/arms transforms are not applied by the presentation subsystem;
- fail-closed marker: `PASS45_ADS_PRESENTATION_FAIL_CLOSED ... hip_transform_preserved=1 runtime_visual_acceptance=pending`;
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

The 2026-08-26 runtime showed anonymous box/cylinder weapon pieces and a primitive anti-armor launcher. `BuildSourceOnlyWeaponVisual()` still exists as migration-era collision/debug/prototype history, but it is no longer allowed to become accepted rendered content.

### Current corrective source state — 2026-08-26 — SOURCE-CODED / RUNTIME UNTESTED

- every concrete weapon production helper calls `HideStaticWeaponFallback()` **before** any skeletal/static production `LoadObject` can fail;
- source BasicShape components are hidden in-game, visibility-disabled, shadow-disabled and navigation-disabled before production resolution;
- a missing AK/MP5/M1911/M700/M14/MAC-10/TEC-9/Lever visual now emits `PASS45_WEAPON_PRODUCTION_VISUAL_GAP ... primitive_visible=0` rather than leaving the composite proxy visible;
- missing exact Remington 870 / M249 emits the same fail-closed visual gap with `real_fallback_pending=1`;
- `UOCRealWeaponFallbackSubsystem` independently detects `/Engine/BasicShapes/` components and retires their rendering with `PASS45_PRIMITIVE_WEAPON_VISUAL_RETIRED`;
- any BasicShape component that remains visible emits hard failure `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL ... runtime_acceptance=0`;
- a valid rack with zero visible BasicShape weapons emits `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY ... visible_basicshape_weapons=0 content_readiness_separate=1`;
- real M249/M1911/MAC-10/Remington fallback meshes attach to the unscaled `WeaponRoot` rather than the scaled physics body, preventing fallback distortion;
- fallback replacement preserves the invisible physics-root collision authority needed by pickup/drop instead of disabling the root collision;
- `PASS45_REAL_WEAPON_FALLBACK_READY ... primitive_visible=0 visual_root_unscaled=1 physics_root_preserved=1` records that split;
- anti-armor launcher follows the same fail-closed rule before its production mesh load;
- strict runtime evidence now **requires** `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY` and **forbids** `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL` plus launcher production-visual failure.

Source guard: `VERIFY_PASS45_PRIMITIVE_WEAPON_RETIREMENT.py`  
Workflow: `.github/workflows/pass45-primitive-weapon-retirement.yml`

This does not claim that every exact production weapon exists. Missing exact production content remains `CONTENT GAP`; the important change is that missing content can no longer impersonate a finished weapon by displaying cubes/cylinders.

Requirements:

- BasicShape may remain only for invisible collision/debug roles;
- accepted real visuals must keep source primitive parts hidden in pickup/equipped/dropped states;
- runtime validator fails if a primitive fallback becomes visible for a required-available weapon;
- MAC-10 pickup must have readable real scale;
- no anonymous boxes/cylinders on accepted weapon racks.

## 8. P0 — weapon audio

Current audio subsystem supports confirmed shot/state/impact events, but the 2026-08-26 runtime proved that some weapons can still be silent.

### Current corrective source state — 2026-09-01 — SOURCE-CODED / RUNTIME UNTESTED

A repository weapon-audio fallback closes the **source-level silent-shot path** without pretending that generic sound identity is final authored content:

- an assigned authored `UOCWeaponAudioProfile` still wins whenever it contains the requested event;
- if the requested near-shot event is unassigned/empty, `UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()` lazily creates a transient presentation-only profile;
- the represented AK first prefers the already tracked `/Game/AK-47/.../AK47_Fire_Cue`, `Reload_Cue` and `AK47_Empty_Cue` assets;
- other current weapons may temporarily reuse the tracked `/Game/R13/Audio/gunfire_sfx` shot and tracked reload assets rather than disappear acoustically;
- if no authored distant tail exists, the fallback uses the factual near report at reduced distance volume rather than becoming fully silent;
- tracked `/Game/R13/Audio/shotguncock` is wired only to the pump-action fallback; bolt and lever mechanical sounds remain explicit **AUDIO CONTENT GAP** until accepted factual content exists;
- tracked `snd_bullethit` is available as a temporary impact fallback;
- fallback profile creation never mutates ammo, fire cadence, damage, projectile/trace authority, weapon transforms or action timing;
- source marker: `PASS45_WEAPON_AUDIO_FALLBACK_READY ... authoritative_mutation=0 runtime_acceptance=0`;
- failed repository sound load emits `PASS45_WEAPON_AUDIO_CONTENT_GAP` instead of a fake READY state.

Source guard: `VERIFY_PASS45_WEAPON_AUDIO_FALLBACK.py`  
Workflow: `.github/workflows/pass45-weapon-audio-fallback.yml`

This is **not final audio acceptance**. Exact per-weapon shot character, indoor/outdoor variants, distant tails, suppressor behavior, reload layers, bolt/lever mechanics, mix levels and local UE audibility still require authored content and factual playtest.

Current manual-action routing remains:

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

The 2026-08-26 runtime rejected both the grenade body and the smoke presentation. At that historical checkpoint the grenade rendered an Engine BasicShape sphere, `AOCSmokeCloud` rendered a cluster of Engine BasicShape spheres, and the repository audit had not yet established an accepted authored smoke/Niagara payload. That historical rejection remains valid evidence for the old state, but it is no longer the current source-content description.

### Current corrective source state — 2026-08-30 — SOURCE-INTEGRATED / RUNTIME REJECTED

Visual/smoke primitive retirement and authored-source continuation:

- visible `/Engine/BasicShapes/Sphere.Sphere` ownership is removed from `AOCGrenadeProjectile`;
- grenade collision remains a small invisible `USphereComponent`, separate from visual truth;
- the tracked `/Game/R13/Weapons/grenade.grenade` mesh is loaded as the current shared recognizable grenade body with uniform bounds-based scaling;
- successful visual resolution emits `PASS45_GRENADE_PRODUCTION_VISUAL_READY ... primitive_visible=0 production_visual=1`;
- missing/invalid production grenade content emits `PASS45_GRENADE_PRODUCTION_VISUAL_FAIL ... primitive_visible=0 runtime_acceptance=0`; the old sphere is never shown as fallback;
- the shared body is not falsely promoted to three exact grenade bodies: frag/smoke/flash type identity currently uses distinct tracked authored materials, while `shared_generic_body=1 exact_type_body=0 type_specific_content_gap=1` remains explicit;
- the old `AOCSmokeCloud` `SmokePuff_*` BasicShape cluster is physically retired;
- committed `/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop` Niagara content is now the sole visible smoke presentation owner;
- `AOCSmokeCloud` loads and activates that authored Niagara on gameplay clients; load failure emits `PASS45_SMOKE_VFX_LOAD_FAIL ... primitive_visible=0 runtime_acceptance=0` rather than resurrecting primitive smoke geometry;
- a successful factual client load/activation emits `PASS45_SMOKE_VFX_RUNTIME_READY ... runtime_loaded=1 activated=1 ... exact_visual_sync=0 manual_visual_acceptance=0`;
- smoke gameplay containment is a finite 3D volume whose radius and half-height expand from game-time age without a per-frame actor Tick;
- source intentionally reports `exact_visual_sync=0`: exact Niagara expansion timing, scale, look and performance still require direct UE 5.8 calibration/acceptance;
- a failed smoke gameplay-volume spawn emits `PASS45_SMOKE_GAMEPLAY_VOLUME_FAIL`.

Transactional/safe throw source correction:

- `ServerThrowSelectedGrenade_Implementation()` validates authority/world/alive/not-in-vehicle before any mutation;
- desired grenade origin is checked with a bounded sphere `SweepSingleByChannel` from the view origin toward the intended forward spawn point;
- final candidate location is independently checked with `OverlapBlockingTestByChannel`;
- character and owned/equipped weapons are ignored by clearance queries, so player collision itself does not falsely block a legal throw;
- unsafe start penetration or final overlap rejects the throw with `PASS45_GRENADE_SAFE_SPAWN_REJECTED ... inventory_consumed=0`;
- projectile uses `DontSpawnIfColliding`; failed actor creation emits `PASS45_GRENADE_SPAWN_FAIL inventory_consumed=0`;
- grenade inventory decrements **only after** factual authoritative projectile spawn succeeds;
- successful throw inherits current player velocity in addition to forward/up throw impulse;
- successful commit emits `PASS45_GRENADE_THROW_COMMIT_READY safe_sweep=1 spawn_success=1 inventory_committed_after_spawn=1 inherited_velocity=1 presentation_event=1`;
- `EOCCharacterActionEvent::GrenadeThrow` is an explicit cosmetic action bridge for first-/third-person authored animation;
- successful throw broadcasts that event and emits `PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY ... authored_animation_pending=1 second_gameplay_timer=0`;
- the native presentation component currently emits `PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP`; a Blueprint/event bridge is not accepted as proof of an authored hand/throw/recover sequence;
- the presentation bridge owns no second fuse, inventory clock or projectile authority.

Guard/acceptance harness:

- `VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py` guards primitive retirement, committed grenade/smoke assets, authored type-identity materials, replicated frag Niagara presentation, finite expanding smoke gameplay volume and swept/transactional throw semantics;
- workflow `.github/workflows/pass45-grenade-smoke-primitive-retirement.yml` covers that source guard;
- `VERIFY_PASS45_GRENADE_THROW_ANIMATION_GATE.py` and `VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py` keep authored first-person throw animation fail-closed;
- `VERIFY_PASS45_GRENADE_FLASH_GATE.py` / `VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py` keep a distinct authored flash world-VFX requirement fail-closed;
- cumulative `RUN_ALL_VERIFY.py` includes the grenade/smoke source guard;
- strict runtime evidence requires a factual successful `PASS45_GRENADE_THROW_COMMIT_READY`, successful authored type-identity material, factual authored throw-animation runtime readiness and `PASS45_SMOKE_VFX_RUNTIME_READY`;
- strict runtime evidence forbids safe-spawn/spawn failures in the valid open-space acceptance throw, grenade visual/material failures, authored throw-animation content gap, smoke load/content/spawn failures;
- a source READY marker with `manual_visual_acceptance=0` remains automated evidence only and cannot close direct visual acceptance.

This closes the source-level inventory-loss, primitive-smoke and missing-smoke-donor architecture defects. It does **not** claim final grenade hand animation, exact per-type grenade bodies, flash world VFX, grenade scale/throw feel or smoke scale/look/performance in UE runtime.

Still pending:

- accepted authored first-person hand/pull/throw/recover animation must consume the `GrenadeThrow` presentation event; the event bridge itself is not visual acceptance;
- exact distinct frag/smoke/flash grenade bodies remain **CONTENT GAP**; current shared real body + authored type-identity materials provide source-level distinguishability without pretending exact-body closure;
- distinct authored flash-grenade world VFX remains **CONTENT GAP**;
- smoke Niagara is source-integrated, but direct UE 5.8 smoke growth/scale/look/sight-blocking/performance acceptance remains pending and exact visual/gameplay expansion synchronization is not claimed;
- local UE 5.8 near-wall behavior, bounce/roll/fuse, visual scale and throw feel remain unverified.

Requirements:

- recognizable fragmentation/smoke/flash models or accepted type-specific presentation without false exact-body claims;
- visible first-person throw presentation;
- gravity/collision/bounce/roll;
- defined fuse start;
- no floating grenade;
- growing volumetric/particle smoke with useful sight blocking;
- distinct frag/flash/smoke VFX and audio;
- safe throw origin outside player collision/weapon/world geometry;
- grenade count decrements only after factual projectile spawn succeeds;
- failed/blocked spawn consumes zero inventory;
- missing authored smoke VFX must fail visibly and may never fall back to BasicShape puffs.

Acceptance:

- no visible BasicShape grenade or smoke geometry;
- production grenade mesh loads and is readable in hand/flight/ground states;
- valid open-space throw produces `PASS45_GRENADE_THROW_COMMIT_READY`;
- failed projectile spawn consumes zero grenade inventory;
- near-wall/blocked throw cannot spawn inside the player or solid geometry and consumes zero inventory;
- authored throw animation is visibly coherent in first person and does not create a second gameplay timer;
- bounce/roll settles coherently without floating;
- frag/smoke/flash presentation is distinguishable;
- smoke expands into accepted authored VFX and materially obscures sight without fake sphere geometry.

## 10. P0 — Museum / Culture House / Silpo ownership, identity and bound reference evidence

### Current corrective source state — 2026-08-28 — SOURCE-CODED / REFERENCE-BOUND / RUNTIME REJECTED

Source audit and the fail-visible contract prove intended identity ownership without pretending that source geometry is the same thing as visual acceptance:

- R13.7 is the sole Museum exterior identity owner, rooted through the canonical Museum geo anchor, and its source signature contains no Culture-House six-column civic facade;
- R14.6 is the sole six-column Culture House identity owner and is rooted at the separate canonical `FOCGeoReference::CultureHouse()` site;
- R14.0 owns the Silpo shell at the canonical Silpo anchor;
- R14.3 owns the visible Silpo facade identity/sign at that same Silpo anchor, including the explicit `Сільпо` text stage;
- one startup coordinator cancels historical delayed landmark timers and runs the current Museum, Silpo and Culture stages inside one bounded startup sequence;
- `OCR146LandmarkSeparationSubsystem` remains `mutation=0`: it rejects cross-parcel Museum/Culture/Silpo shell ownership instead of moving, destroying or respawning buildings to manufacture a pass;
- the strict runtime evidence gate additionally requires the factual `R14.3 Silpo facade identity pass built at` marker, so an unbranded R14.0 shell can no longer false-pass as completed Silpo identity;
- the focused `RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd` route requires the same facade/sign stage;
- source guard: `VERIFY_PASS45_LANDMARK_IDENTITY.py`;
- dedicated workflow: `.github/workflows/pass45-landmark-identity.yml`;
- cumulative `RUN_ALL_VERIFY.py` includes the landmark identity guard.

### Mandatory subordinate visual contracts

Detailed reviewed evidence is not duplicated wholesale inside this canonical TZ. `PASS45_REFERENCE_PACK_BINDINGS.md` binds the location-specific normative contracts below into Gate E/K:

- Museum: `_DOCS/REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/REFERENCE_SPEC.md`;
- Silpo: `_DOCS/REFERENCE_PACKS/LOC_SILPO_002_OSTER_SILPO/REFERENCE_SPEC.md`;
- Culture House: `_DOCS/REFERENCE_PACKS/LOC_CULTURE_003_OSTER_CULTURE_HOUSE/REFERENCE_SPEC.md`.

Binding state is not runtime acceptance.

Museum contract:

- user-photo-driven massing/material/site/stadium context remains normative;
- required direct screenshot set: `MUS-CAM-01..07`;
- Museum evidence cannot define Culture House or Silpo geometry.

Silpo contract:

- user pack includes facade/entrance/long wall/interior checkout/opposite-side context and water-tower evidence;
- production baseline is the selected **2020 graphite facade state**; 2017–2019 light facade remains an explicit variant only and may not be mixed into one impossible temporal state;
- stepped parapet, volumetric `Сільпо` sign, lower entrance wing, long advertisement wall, parking/street context and checkout identity are normative;
- `LOC_TOWER_002A_OSTER_WATER_TOWER` requires the verified aged-brick/ring/opening/telecom silhouette, while its exact world transform remains `PROVISIONAL` until multi-view geographic evidence closes it;
- required direct screenshot set: `CAM-SILPO-01_FRONT_WIDE`, `CAM-SILPO-02_FRONT_CLOSE`, `CAM-SILPO-03_ENTRANCE_SIDE`, `CAM-SILPO-04_LONG_WALL`, `CAM-SILPO-05_OPPOSITE_SIDE`, `CAM-SILPO-06_STREET_AXIS`, `CAM-SILPO-07_WATER_TOWER_SIGHTLINE`.

Culture House contract:

- verified identity/address: Oster Culture House, Hranovskoho 3;
- verified public context includes former-synagogue/Soviet Culture House adaptation and the old park beside the building;
- current six-column source facade remains a **PROVISIONAL WORKING HYPOTHESIS**, useful for source ownership separation but not promoted to photo-verified exact geometry;
- exact bearing, dimensions, material zones, roof/window/door rhythm and hidden/rear geometry remain provisional until direct evidence closes them;
- required provisional direct screenshot set: `CUL-CAM-01_FRONT_WIDE`, `CUL-CAM-02_FRONT_CLOSE`, `CUL-CAM-03_OBLIQUE_LEFT`, `CUL-CAM-04_OBLIQUE_RIGHT`, `CUL-CAM-05_SITE_CONTEXT`;
- a later dedicated user Culture House photo pack supersedes conflicting provisional assumptions.

This is **source ownership/reference closure only**. Museum, Silpo and Culture House can still fail Gate K when rendered quality, composition, materials or site context do not match their bound evidence. Local UE 5.8 screenshots remain authoritative.

Requirements:

- Museum and Culture House are separate visible buildings at separate canonical sites;
- six-column Culture-House facade at Museum site = hard FAIL;
- exactly one mutating visible shell owner per landmark;
- late validators/details may not replace/relocate authoritative shells;
- Silpo identity/sign belongs only to canonical Silpo site;
- each landmark needs separate runtime identity and screenshot evidence;
- active location reference packs in `PASS45_REFERENCE_PACK_BINDINGS.md` are mandatory Gate E/K evidence, not optional reading;
- `VERIFIED`, `PROBABLE`, `UNKNOWN/PROVISIONAL` evidence classes may not be silently upgraded by current source constants or green verifiers.

Acceptance:

- Museum screenshot set visibly reads as the Museum/Solonyna-house reference and contains no six-column civic facade;
- Culture House screenshot set visibly reads as the separate civic building at its own site while provisional visual claims remain honestly provisional;
- Silpo screenshot set visibly proves selected-period shell/sign/entrance/street identity plus the required water-tower sightline;
- automated log evidence contains all separation/identity READY markers plus the R14.3 facade identity stage;
- automated structural acceptance cannot substitute for direct visual/photo-fidelity inspection.

## 11. P0 — vegetation replacement

### Current corrective source state — 2026-08-31 — STARTUP-QUARANTINED / RUNTIME REJECTED

The intended player-facing tree identities remain `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01`, but the latest UE 5.8 Quick Normal run rejected their normal START-time material/static-mesh path. The source recovery therefore removes synchronous native-constructor/CDO loading and quarantines the second Stadion Oster START-time path. Normal runtime does not opt into `-Pass45LoadKiteDemoTrees`; an explicit deferred async route remains diagnostic-only and `runtime_acceptance=0` until the exact material/static-mesh path is repaired and visually accepted.

The obsolete `OCTreeContentUpgradeSubsystem` remains physically deleted: there is still no second late remap owner. `VERIFY_PASS45_TREE_STARTUP_DEFERRED.py`, the pre-tick startup guard and foliage guards protect source identity/startup ownership separately from runtime material/LOD acceptance. Oak remains an explicit unverified content gap.

Requirements:

- replace, do not merely recolor, rejected tree family;
- no Cylinder/Sphere fantasy forest;
- verified real conifer/pine assets where suitable;
- oak remains explicit CONTENT GAP until verified;
- placement/species follow Oster references;
- avoid obvious repeated rotations/scales and crude LOD collapse;
- normal first-frame startup must not synchronously load the currently rejected KiteDemo material/static-mesh chain.

## 12. P0 — visual fidelity / no prototype acceptance

Stable 60 FPS is not permission to ship primitive visuals.

Gate K requires no visible production BasicShape/proxy content, no major white/default materials, acceptable ground/material/vegetation/LOD quality, reference-faithful landmark composition and direct screenshots. For Museum, Silpo and Culture House, “reference-faithful” is defined by their active bound contracts in `PASS45_REFERENCE_PACK_BINDINGS.md`.

## 13. P0 — HMMWV movement and M2 turret station

HMMWV forward direction improved, but road top speed must be at least 80 km/h without breaking handling.

M2 required hierarchy:

`vehicle roof mount -> rotating ring/shield/gunner station -> elevation cradle -> M2 weapon`

Requirements include coherent assembly, full 360° yaw for selected project configuration, correct elevation, no floating parts, useful gunner camera, non-inverted vertical aim and no release camera drift.

Authored-pivot source correction already exists and must not regress to the rejected bounds-bottom/longest-axis alignment heuristic. Runtime acceptance of that source correction remains pending after the 2026-08-27 rejection.

## 14. P0 — BTR-4 material, orientation and remote operator view

Requirements:

- no white/default material before or after possession;
- material ownership stable through possession/movement;
- visual/physics/input forward axes agree;
- BM-7 Parus uses interior remote-operator optic/monitor presentation;
- external turret follows remote operator aim independently of interior camera placement.

BTR glTF Y-up import R3 / +X-forward source contract is already source-coded with one explicit compensation path. The 2026-08-27 runtime screenshots still reject final BTR visual/orientation presentation, so source green is not acceptance.

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

The Silpo-bound historic water tower is a specific evidence-owned landmark and is not permission to resurrect a generic tower family. Its silhouette/material contract is normative; exact transform remains provisional until multi-view geographic closure.

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
- obsolete conflicting owners are physically deleted together with stale verifier expectations;
- the rejected manual-action whole-transform fallback/state/profile API also stays physically retired; missing authored M700/870/Lever action content preserves baseline presentation rather than reviving a procedural substitute.

## 22. Current source implementation milestone — 2026-09-01 item 16 fail-closed continuation

State: **SOURCE-CODED / PRE-REUSE-DOCUMENTATION IMPLEMENTATION HEAD `482a84b06c3f0c3dd92919b04e9a846510987eab` / CURRENT BRANCH HEAD MUST BE READ FROM GITHUB / NOT RUNTIME ACCEPTED**.

Historical verified source milestones remain structural evidence only. Current-head source CI must be read for the exact head after each substantive cycle; no source CI result overrides the factual `RUNTIME REJECTED 2026-08-31` verdict or replaces local UE 5.8 compile/gameplay/direct screenshot acceptance.

Implemented/source-prepared:

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
- authored manual-action sequence bridge through `OCWeaponAnimationProfiles`, exact production skeletal compatibility checks and no second gameplay timer;
- rejected whole-weapon/arms manual-action interpolation path, state and profile displacement fields physically retired;
- missing M700/870/Lever authored sequences preserve baseline presentation and emit explicit authored-content-gap/failure markers instead of procedural motion;
- exact manual-action mechanical audio routing through `BoltCycle` / `PumpCycle` / `LeverCycle` with explicit empty-set content-gap behavior;
- local/remote manual-action audio ownership split to avoid intentional double playback;
- tracked pump mechanical cue remains available; bolt/lever mechanical audio remains fail-visible content gap;
- explicit per-weapon ADS socket-reference fields and separate factual `bADSCalibrated` state;
- fail-visible ADS entry diagnostics through `PASS45_ADS_PROFILE_UNCALIBRATED` / `PASS45_ADS_ALIGNMENT_FAIL` / `PASS45_ADS_ALIGNMENT_SAMPLE`;
- uncalibrated requested ADS preserves baseline hip presentation instead of applying guessed offsets, with `PASS45_ADS_PRESENTATION_FAIL_CLOSED`;
- `oc.Weapon.ADS.Debug` calibration rays for camera vs authored sight axis;
- repository fallback prevents an unassigned/empty near-shot profile from silently swallowing a factual shot;
- exact tracked AK cues are preferred for AK; tracked R13 gunfire/reload/impact assets are temporary source fallbacks for current gaps;
- `PASS45_WEAPON_AUDIO_FALLBACK_READY` / `PASS45_WEAPON_AUDIO_CONTENT_GAP` distinguish source fallback from missing content;
- concrete weapon variants and launcher hide source BasicShape geometry before production load failure can render it;
- real weapon fallbacks attach to unscaled `WeaponRoot` while the invisible physics root retains collision authority;
- strict runtime evidence requires `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY` and forbids `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL`;
- tracked R13 grenade mesh replaces the visible Engine sphere fail-closed;
- frag/smoke/flash share that real body for now but use distinct tracked authored identity materials; exact type-specific bodies remain content gap;
- primitive smoke-ball components are physically removed and committed PotaVFX Niagara smoke is wired as the sole visible smoke owner;
- smoke gameplay occlusion is finite and expands by query-time age; exact Niagara synchronization remains explicitly unclaimed;
- grenade throw uses swept clearance + overlap validation and `DontSpawnIfColliding`;
- grenade inventory commit occurs only after factual projectile creation; blocked/failed spawn consumes zero inventory;
- successful grenade throw inherits character velocity and emits `GrenadeThrow` presentation event without a second gameplay timer;
- the committed authored `/Game/R13/Audio/snd_throw1` payload is loaded by the native character presentation owner and played only from the replicated successful-throw event;
- factual audio success emits `PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY ... replicated_event=1 gameplay_authority=0`; missing/unloadable content emits `PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP` and is fatal to strict runtime evidence;
- authored first-person grenade hand/throw/recover animation remains explicit content gap; authored throw audio is source-integrated but cannot substitute for that animation;
- distinct authored flash-grenade world VFX remains explicit content gap;
- Museum source identity is guarded against six-column Culture-House contamination;
- Culture House is guarded as the sole current six-column civic source owner at its separate canonical geo anchor, while the exact six-column visual hypothesis remains provisional until direct photo evidence accepts it;
- R14.0 Silpo shell and R14.3 visible `Сільпо` facade identity are guarded to the same canonical Silpo site;
- strict runtime evidence and the focused landmark launcher require the factual R14.3 Silpo facade/sign stage;
- Museum, Silpo and Culture House have separate repository-controlled bound reference specs through `PASS45_REFERENCE_PACK_BINDINGS.md`;
- source verifiers reject resurrection of old feedback/action shortcuts, manual-action whole-transform fallback, fake ADS calibration, silent-profile acceptance, visible primitive weapon/grenade/smoke fallbacks and landmark identity false-pass paths.

Still not runtime accepted: compile on local UE 5.8, recoil feel/release, authoritative action timing/feel, authored M700/870/Lever sequence compatibility and visuals, bolt/lever mechanical sound content, exact per-weapon sound identity/mix, exact per-weapon sight socket/offset calibration, production hierarchy, drop settling, muzzle alignment, launcher visual, rendered zero-primitive rack proof, grenade visual scale/near-wall/throw behavior, authored grenade throw animation, exact grenade-type bodies, distinct flash world VFX, smoke visual scale/look/performance/exact sync, Museum/Culture/Silpo direct visual identity, Silpo sign readability, water-tower final transform, Culture House exact facade fidelity, broader landmark/photo/world fidelity and current-head startup recovery proof.

## 23. Corrective execution order

Completed/source-coded items are marked only for source/reference work, not runtime acceptance.

1. [x] Preserve 2026-08-27 screenshots/evidence and mark latest runtime **RUNTIME REJECTED**.
2. [x] Promote latest factual evidence over older runtime verdict wording; preserve 2026-08-26 and older packs as history.
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
15. [x] Code replicated-gate authored manual-action animation/audio bridge without a second gameplay timer; subsequently physically retire the rejected whole-weapon/arms procedural fallback so missing authored content preserves baseline presentation.
16. [ ] Replace procedural manual-action fallback cues with accepted authored moving-part/skeletal presentation where production assets support it, and populate real bolt/pump/lever sound content. **The procedural fallback is now physically retired; this item remains open because exact M700/870/Lever action sequences plus bolt/lever audio and UE 5.8 acceptance are still missing.**
17. [x] Build fail-visible per-weapon ADS/sight profile architecture, socket-based alignment diagnostics and source validation without inventing calibration data; uncalibrated ADS presentation now fails closed after the 2026-08-27 AK rejection.
18. [ ] Calibrate exact rear/front/optic references and ADS transforms for every accepted production weapon in local UE 5.8; only then set factual `bADSCalibrated=true` per weapon.
19. [x] Close the source-level silent-shot path with an event-local repository audio fallback and dedicated verifier/workflow; keep runtime audibility and exact sound identity unaccepted.
20. [ ] Replace temporary generic audio fallback with accepted exact per-weapon shot/reload/distant/mechanical profiles and close bolt/lever manual-action audio gaps.
21. [x] Source-retire visible primitive weapon/pickup/launcher fallbacks: hide before production load, preserve invisible collision authority, add hard runtime ready/fail markers and strict evidence gate. **Rendered UE acceptance remains pending.**
22. [x] Source-retire primitive grenade/smoke visuals: use tracked R13 grenade mesh fail-closed; physically remove fake smoke spheres; wire committed PotaVFX authored Niagara as sole visible smoke owner; preserve fail-visible load/runtime markers. **Rendered grenade/smoke visual acceptance remains pending.**
23. [x] Correct grenade source throw semantics: safe swept/overlap-checked origin, `DontSpawnIfColliding`, inventory commit only after successful spawn, inherited movement velocity, explicit `GrenadeThrow` presentation event and strict runtime marker requirement. **Local UE behavior/animation acceptance remains pending.**
24. [ ] Accept first-person grenade hand/throw/recover animation, exact/distinct frag/smoke/flash presentation, distinct flash world VFX and direct UE 5.8 smoke scale/look/performance. **Smoke Niagara source content is now committed/wired; that source integration does not close runtime visual acceptance. Authored throw audio is also committed/wired, but it does not close the hand-animation or visual gaps.**
25. [x] Source-close Museum/Culture House/Silpo identity ownership and strict branded-site evidence. **Rendered identity/fidelity acceptance remains pending.**
26. [x] Bind Museum, Silpo and Culture House as separate Gate E/K reference contracts. Silpo uses reviewed user evidence; Culture House keeps unverified exact geometry explicitly `PROVISIONAL`. **Reference binding is not runtime acceptance.**
27. [ ] Replace rejected vegetation family and complete broader environment acceptance. **The intended HillTree/ScotsPine identities are quarantined from normal START-time synchronous loading after the latest UE 5.8 material/static-mesh rejection; direct visual/material/LOD repair and acceptance remain open.**
28. [ ] Complete/accept HMMWV M2 ring/shield/gunner hierarchy with authored pivot, 360° yaw and correct camera in UE runtime.
29. [ ] Calibrate/accept HMMWV gameplay top speed to >=80 km/h without breaking handling.
30. [ ] Close BTR white material state across pre/post possession in runtime.
31. [ ] Runtime-accept BTR R3 Y-up/+X-forward orientation and remote operator monitor/optic gameplay.
32. [ ] Raise core world/material/LOD visual fidelity above prototype state without lowering native render scale, including ParkPaths/ground/landmark surroundings.
33. [ ] Validate fullscreen + 60 FPS + thermal soak after visual fixes.
34. [ ] Validate tactical map screenshot.
35. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots.
36. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 24. Final acceptance gates

### Gate A — source/build/import
current branch/head; source workflows; Stein R3 fresh load; production HMMWV/M2/BTR import/fresh load; UE 5.8 editor build exit 0.

### Gate B — world rendering
daylight/exposure; stable ground/roads/sidewalks; no black-world or blown-out scene; screenshots.

### Gate C — weapon materials and visible content
real accepted visual/material/texture chain; `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY`; no `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL`; no visible primitive fallback; launcher production visual valid; unresolved exact items remain CONTENT GAP rather than visible cubes/cylinders.

### Gate D — weapon firing / ordnance physics
factual shot count = ammo = recoil = muzzle = audio; production muzzle origin; no ghost recoil; no release downward kick; exact selector modes; deterministic finite Burst3 if enabled; manual-action server gate plus accepted visible action animation and audible action-specific content; exact per-weapon ADS alignment accepted; no temporary generic audio fallback remains on a final accepted weapon; drop physics; no primitive grenade/smoke geometry; factual `PASS45_GRENADE_THROW_COMMIT_READY`; blocked/failed grenade spawn consumes zero inventory; safe swept/overlap-checked throw origin; accepted authored throw animation; accepted type-specific grenade presentation and authored smoke VFX.

### Gate E — landmarks/environment
Museum/Culture/Silpo separated and identified; R14.3 Silpo facade/sign stage present; all active location contracts in `PASS45_REFERENCE_PACK_BINDINGS.md` applied; rejected residential/tree families absent; direct landmark screenshot sets accepted; Gate K passes.

### Gate F — HMMWV/M2
forward axis; >=80 km/h; proportional body; coherent ring/shield/gunner; authored-pivot alignment; 360° yaw; correct elevation/view; no inverted aim/drift.

### Gate G — BTR-4
no white/default material; +X forward/Y-up import contract runtime-accepted; proportional visual; remote interior optic/monitor; coherent turret/camera.

### Gate H — possession
no Museum teleport on civilian vehicle/HMMWV/BTR enter/exit.

### Gate I — display/performance/thermal
intended fullscreen; ~60 FPS; native render scale; 10-minute mixed soak.

### Gate J — tactical map
current compact Oster topology screenshot accepted.

### Gate K — visual fidelity
no production BasicShape/proxy core content; no major white/default materials; acceptable world/vegetation/LOD; reference-faithful landmarks under the active Museum/Silpo/Culture House contracts; required `MUS-CAM-01..07`, `CAM-SILPO-01..07` intent set and `CUL-CAM-01..05` intent set directly accepted in current-head UE 5.8 screenshots. `VERIFIED` and `PROVISIONAL` evidence classes must remain honest; source tests alone cannot close this gate.

## 25. Current verdict

**PASS 45 = ACTIVE / RUNTIME REJECTED 2026-08-31.**

PR #94 remains **OPEN / UNMERGED**.

The weapon firing/muzzle/drop/action/authored-manual-action-bridge/audio-routing/ADS-diagnostic/fail-closed-ADS/repository-audio-fallback/primitive-retirement/grenade-smoke/transactional-throw/authored-throw-audio corrections plus Museum/Culture House/Silpo source identity ownership and bound reference contracts are **SOURCE-CODED / REFERENCE-BOUND / UNTESTED OR REJECTED IN CURRENT LOCAL UE RUNTIME**. The rejected whole-weapon/arms manual-action fallback is now physically retired; exact authored M700/870/Lever action sequences and bolt/lever mechanical audio remain explicit content gaps. None of these items may be described as fixed in runtime until a current-head local UE 5.8 build, playtest and direct screenshot acceptance proves them.

## 26. Reuse-first replacement mandate — bound 2026-09-01

The normative subordinate specification `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md` is now **bound into this canonical TZ**. `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` is the mandatory provenance/license register for every external code/content intake.

This binding changes implementation strategy, not factual runtime truth and not the existing closed/open meaning of checklist items 1-36.

Mandatory replacement doctrine:

`audit current owner -> prove replacement in isolation -> integrate replacement -> runtime/network/performance acceptance -> switch authority -> physically delete obsolete owner -> delete stale tests/config/fallbacks -> regression acceptance`

Non-negotiable rules:

- do not rewrite a weak custom system from zero when a stable UE 5.8 owner or legally reusable candidate already covers the same responsibility better;
- do not stack a new system permanently on top of the old system;
- old duplicate code is **physically deleted after the replacement passes acceptance**;
- rollback uses Git history, not a live hidden legacy runtime fallback;
- one responsibility still has exactly one mutating production owner;
- engine-native stable systems are preferred over third-party dependencies;
- Beta/Experimental systems require isolated packaged proof before production promotion;
- external code/content with unknown license/provenance is not imported;
- Oster-specific gameplay rules remain Oster-owned adapters around the accepted reusable system.

The bound replacement spec explicitly covers and maps:

- Chaos Vehicles for wheeled vehicle physics, with CARLA as reference/donor only after file-level license review;
- Chaos Modular Vehicles as Experimental proof-only until separately accepted;
- Chaos Physics for rigid bodies, drops, grenades, ragdoll and physical response;
- Chaos Destruction / Geometry Collections / Physics Fields for controlled destruction;
- Niagara for explosions, smoke, fire, muzzle/impact and environmental VFX;
- MetaSounds + Audio Modulation for weapons, mechanical actions, vehicles, explosions, footsteps, impacts, environment and mix state;
- Soundscape only as Beta proof candidate for ambience;
- Game Animation Sample / Motion Matching / IK / Control Rig / Motion Warping / Montages for reusable animation architecture where appropriate;
- ALS Community and PBCharacterMovement only as audited candidates, never automatic parallel locomotion owners;
- World Partition / HLOD / PCG for scalable non-identity-critical world content where migration is justified;
- Behavior Trees / AI Perception / EQS / Smart Objects for AI/bot building blocks;
- selective Lyra/Common User/Enhanced Input/CommonUI patterns where they replace weaker duplicate infrastructure;
- licensed/CC0 production model/material sources such as Poly Haven or verified Fab assets instead of C++ BasicShape approximations;
- AirSim/Project AirSim lineage as future drone reference only, not a current mandatory runtime dependency.

PASS45 execution rule: do not reopen already source-closed items merely to refactor them. Apply this mandate immediately when touching still-open items 16, 18, 20, 24 and 27-35 or a directly related runtime defect. PR #94 remains OPEN / UNMERGED until factual current-head UE 5.8 runtime acceptance.

## 27. Final completeness architecture baseline — integrated 2026-09-01

This section integrates the final architecture-gap audit directly into the canonical TZ. `PASS45_RUNTIME_RECOVERY_TZ_REUSE_FINAL_GAP_ADDENDUM_2026-09-01.md` remains supporting audit evidence; the rules below are the canonical execution summary. These rules do **not** reopen closed checklist items and do not inflate PASS45 into an endless engine migration. They apply when a still-open item or directly related defect touches the relevant system.

### 27.1 Asset loading, startup and streaming — ADOPT

The 2026-08-31 black-screen evidence proves that asset residency/startup ownership is a production concern, not an editor convenience.

Required direction:

- use Unreal `Asset Manager`, `Primary Assets`, soft references (`TSoftObjectPtr` / `TSoftClassPtr` or equivalent) and async/streamable loading for heavy or optional production content where appropriate;
- distinguish first-frame mandatory content from content that may stream after menu/gameplay becomes usable;
- do not add new heavy production mesh/material/audio/animation/VFX families to native constructors/CDOs through `ConstructorHelpers::FObjectFinder` merely for convenience;
- synchronous `LoadObject` on startup-critical paths requires a documented reason and measured cost;
- failed async content must fail visibly without resurrecting BasicShape/default proxy content;
- item 27 vegetation and future vehicle/landmark/content migrations must prove startup/loading behavior in addition to visual quality.

### 27.2 Data-driven gameplay/content definitions — SELECTIVE ADOPT

Use `UPrimaryDataAsset` / Data Assets and Gameplay Tags when they reduce duplicated hard-coded profiles or branching without creating a second authority.

Good candidates:

- weapon presentation/audio/content profiles;
- vehicle handling/content profiles;
- grenade/VFX/audio profiles;
- surface/impact families;
- animation/action profile references;
- content bundles managed by Asset Manager.

Gameplay Tags are labels/state descriptors, not a reason to migrate the project to GAS. Existing clear typed C++ state remains valid where it is simpler. Do not replace functioning logic with tag soup merely for fashion.

### 27.3 Cooking, packaging and asset audit — ADOPT

A source/editor pass is not enough. Production acceptance must include packaged/cooked behavior.

Use Unreal cooking/packaging and Asset Audit tooling to verify:

- required assets are actually cooked;
- editor-only/test content is not accidentally required at runtime;
- package size and large dependency chains are visible;
- optional content does not silently hard-reference itself into startup;
- broken/missing cooked assets fail the runtime gate.

Primary Asset Labels/chunks may be introduced only when patch/DLC/install organization produces a concrete benefit. Do not create chunk complexity without a delivery requirement.

### 27.4 Shader/PSO hitch prevention — ADOPT AND MEASURE

Final Windows packaged testing must explicitly cover first-use shader/PSO hitches.

- keep UE PSO precaching enabled where supported and validate coverage on the actual DX11/SM5 production path;
- test cold/cleared driver-cache startup and first encounter with representative weapons, vehicles, Niagara effects, materials and world sectors;
- initial loading/menu flow may wait for genuinely required high-priority PSO work rather than exposing severe first-use hitches;
- a temporary engine default material shown only because a production PSO is not ready cannot satisfy visual acceptance;
- if automatic precaching leaves material runtime hitches, evaluate a bounded bundled PSO cache rather than inventing a custom shader-cache system.

### 27.5 Texture memory and streaming — STANDARDIZE / PILOT ONLY WHEN NEEDED

Conventional UE texture streaming + authored mipmaps is the default production path. Item 33 performance acceptance must inspect texture pool/memory pressure and visual mip behavior.

Streaming Virtual Texturing or Runtime Virtual Texturing may be piloted for large high-resolution surfaces/landscape/material blending only when profiler/memory evidence shows benefit. Do not globally convert textures to virtual textures merely because the feature exists.

### 27.6 Unified Physical Material -> surface response pipeline — ADOPT

Use Unreal Physical Materials / Surface Types as the common physical surface identity for applicable gameplay and presentation.

Target families include concrete, brick, metal, wood, glass, asphalt, soil, grass and water where relevant.

One factual hit/contact may drive, from the same resolved surface identity:

- projectile impact VFX;
- impact/ricochet audio where appropriate;
- decals/bullet marks/scorch presentation;
- debris presentation;
- footsteps;
- tyre/contact audio and surface handling where applicable.

Decals must have bounded lifetime/count or pooling policy. Niagara/audio/decals remain presentation and never become damage authority.

### 27.7 Complete spatial audio budget — ADOPT

The audio plan is incomplete without spatialization and voice budgeting.

Use engine-native Sound Attenuation / Audio Volumes / reverb / occlusion / air absorption where appropriate. Use Sound Concurrency to prevent gunfire, footsteps, ambience and debris from creating an unlimited voice flood.

Required acceptance includes:

- near/far weapon readability;
- indoor/outdoor transition;
- building/room reverb where useful;
- vehicle cabin vs exterior;
- sensible occlusion through world geometry;
- distant sounds yielding priority to nearby critical combat audio;
- no cosmetic playback work on dedicated server.

Audio Modulation remains the state/mix layer; MetaSounds is selective rendering; Soundscape remains Beta pilot-only.

### 27.8 Animation events, physical animation and constraints — SELECTIVE ADOPT

Use Animation Notifies for presentation synchronized to authored animation, especially footsteps, cloth/gear, shell/foley or other cosmetic timing. A Notify must not become a second authoritative weapon/grenade/inventory timer.

Use Physics Assets / Physical Animation Profiles / Physics Constraints where they provide accepted character hit/death blending, ragdoll stabilization, doors/hinges or physical joints better than bespoke transform loops.

Discrete gameplay actions still follow factual server state -> replicated presentation -> authored sequence/Montage/IK/Control Rig as appropriate.

### 27.9 Multiplayer proof must include bad-network conditions — REQUIRED

Normal server-authoritative Unreal replication remains production owner during PASS45.

Any gameplay-affecting replacement must be checked with relevant dedicated/listen-server semantics, at least one real client path, late state synchronization where applicable, and Unreal Network Emulation for latency/jitter/loss.

Required representative failure tests include:

- weapon shot/action/reload state;
- grenade spawn/detonation/inventory commit;
- vehicle possession/driving/exit;
- M2/BTR turret control;
- destruction gameplay state;
- AI/objective state where replicated;
- reconnect/late join for persistent replicated state where supported.

Use harsh network profiles during engineering tests, including packet loss and high latency, because localhost/LAN success is not proof. Networking Insights is preferred for traffic analysis.

`Replication Graph` remains a profiling-triggered Beta pilot only. **Correction for UE 5.8:** Iris is production-ready, not Experimental. Iris migration is nevertheless deferred during PASS45 because the existing replication stack is already authoritative and a full replication migration without a measured scaling/correctness need would add unnecessary runtime-recovery risk. Do not stack alternative replication architectures.

### 27.10 Multiplayer/package automation — ADOPT WHERE IT REDUCES MANUAL REGRESSION

Use Unreal Automation/Functional tests for bounded single-instance checks and evaluate **Gauntlet** for packaged multi-process scenarios such as dedicated server + clients.

Gauntlet is preferred over inventing a custom process-orchestration framework for recurring multiplayer smoke tests. Direct visual/audio UE acceptance remains human evidence and cannot be automated away.

### 27.11 Performance diagnostics and significance — ADOPT ENGINE TOOLING

Use Unreal Insights / Networking Insights / engine CPU-GPU-audio-memory statistics before approving major replacement decisions.

Animation Budget Allocator and Significance Manager are profiling-driven tools, not mandatory everywhere. When adopted they may reduce distant cosmetic/AI/animation/VFX/audio work, but never authoritative damage, projectile, possession, objective, inventory or critical replicated state.

Gate I / item 33 must measure representative gameplay, not an empty map.

### 27.12 Rendering features — HOLD CURRENT SAFE BASELINE

Current factual project configuration uses DX11/SM5 after D3D12/RHI instability and disables expensive modern rendering features for boot/performance safety.

Therefore during active PASS45:

- do not enable Nanite or Virtual Shadow Maps as a hidden dependency;
- do not enable Lumen as a shortcut for bad materials/lighting;
- use conventional LOD/HLOD/instancing/material optimization on the accepted runtime path;
- any future D3D12/SM6 + Nanite/VSM/Lumen work is a separate post-stability visual/performance pilot with rollback and 60 FPS comparison.

### 27.13 Atmosphere, weather and environmental presentation — ENGINE-NATIVE FIRST

If richer sky/weather is introduced, prefer engine-native Directional Light, Sky Atmosphere, Sky Light, Exponential Height Fog and Volumetric Clouds rather than importing a broad weather framework by default.

Weather/time-of-day presentation must be scalable and may not become a second daylight authority that conflicts with the existing exposure/daylight contract. Expensive volumetrics require item 33 performance proof.

### 27.14 Save/settings/localization/accessibility — STANDARDIZE, NOT A PASS45 BLOCKER

Keep runtime-recovery scope bounded, but new production-facing code should avoid obvious future dead ends:

- use `UGameUserSettings` / existing Oster settings authority for persistent graphics/audio/display controls;
- use asynchronous SaveGame APIs if later gameplay persistence is required; do not add synchronous save hitches to combat/runtime paths;
- user-facing text should use `FText`/localization-safe paths rather than baking UI strings into non-localizable runtime logic;
- key remapping, mouse sensitivity, FOV, audio levels and readable/subtitle-capable presentation belong to production UX planning.

These do not block PR #94 unless they intersect an existing PASS45 defect.

### 27.15 Crash/replay/reproducibility tooling — ADOPT FOR DIAGNOSTICS

Use Unreal Crash Reporter/log evidence and Unreal Insights instead of inventing parallel crash/profiling frameworks.

The built-in Replay/DemoNetDriver may be piloted later for reproducible multiplayer bug evidence once baseline replication is stable. It is a diagnostic tool, not a replacement for factual current-head runtime acceptance.

### 27.16 Development data/cache policy

Use UE Derived Data Cache normally; derived shader/asset cache products are generated data and must not be treated as source assets or committed as production truth.

A Shared/Zen DDC may be introduced when multiple development/build machines can actually benefit. Do not add infrastructure for a one-machine workflow merely because Epic supports it.

UE Virtual Assets are **not a current Oster migration target** because their standard production workflow expects Perforce-backed virtualization while Oster currently uses GitHub/Git LFS. Reassess only if the source-control architecture changes deliberately.

### 27.17 Explicit no-migration list during PASS45

Unless a new factual blocker proves otherwise, do not start broad migrations to:

- Gameplay Ability System;
- Iris replication migration without a measured scaling/correctness need;
- Replication Graph without measured scaling need;
- Mass AI without measured actor/AI bottleneck;
- full Lyra architecture;
- World Partition conversion;
- Nanite/VSM/Lumen/DX12 renderer stack;
- EOS/online accounts/voice as a dependency of runtime recovery;
- Virtual Assets/Perforce workflow;
- AirSim/Project AirSim as a current dependency.

These systems may be valuable later. Their existence is not permission to derail runtime recovery.

### 27.18 Final completeness rule

For every future large feature the default question is:

`does stable UE 5.8 or a legally reusable audited system already solve this generic problem better?`

If yes: pilot it, measure it, migrate only the bounded responsibility, switch authority, then physically delete the obsolete duplicate. If no: keep/build the smallest Oster-owned implementation that matches the game requirement.

Every external code/content intake still requires `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md`. Unknown license/provenance remains **DO NOT IMPORT**.

This final architecture baseline changes implementation strategy only. It does not alter the 36-item formal percentage by itself, does not mark runtime-rejected items complete, and does not authorize PR #94 merge.

## 28. Final production-hardening completeness pass — integrated 2026-09-01

This section closes the remaining architecture-policy gaps found after re-auditing current Oster source/configuration against UE 5.8 production guidance. It is normative for future touched code but **does not create new checklist points, reopen closed work, or authorize broad refactors while item 16 remains the first factual open item**.

### 28.1 Server authority, RPC validation and anti-abuse — REQUIRED

Oster remains server-authoritative. A client may request an action; it may never be trusted to provide the final factual result.

Hard rules for all current and future client -> server RPC paths:

- server owns damage, hit acceptance, ammunition, inventory, grenade/trap counts, health/life state, team/role permissions, vehicle seats/possession, turret authority, objective state and destruction gameplay state;
- every server RPC that mutates gameplay must re-check current ownership/permission/state rather than trusting the client-side precondition;
- pointer/object arguments must be valid, relevant and owned/usable by the requesting player where required;
- position/direction/value inputs must reject non-finite (`NaN`/`Inf`) and impossible/out-of-contract ranges before mutation;
- high-frequency aim/input RPCs must be rate-bounded/coalesced and may use unreliable delivery where latest-state semantics are sufficient; do not bind an unlimited reliable-RPC stream directly to repeatable input;
- interaction/revive/pickup/vehicle/repair paths must re-check factual server distance, alive/downed/team/role/line-of-sight/state as applicable at commit time;
- rejected RPC requests change no ammunition/inventory/score/ownership and do not emit accepted presentation markers;
- normal clients may never invoke development/admin/test-only mutation paths in a shipping build;
- audit/configure Unreal RPC DoS detection for packaged multiplayer/dedicated-server testing and record any overrides rather than silently disabling protection.

Current server-authoritative code already performs many of these checks; this rule prevents new or migrated systems from weakening that boundary.

### 28.2 Replication relevancy, dormancy and bandwidth — ADOPT BEFORE A NEW REPLICATION FRAMEWORK

Before introducing Replication Graph or migrating to Iris, first use the normal Unreal replication stack correctly:

- apply relevancy/cull-distance rules to actors that do not need global replication;
- use dormancy for persistent actors that rarely change;
- avoid multicast for cosmetic effects that clients can derive from an already replicated factual event/state;
- replicate durable gameplay state, not every cosmetic intermediate transform/event;
- avoid high-frequency replication of static world props, distant ambience, local VFX/debris and other non-authoritative presentation;
- measure actor/RPC/property traffic with Networking Insights under representative player/bot/vehicle counts and bad-network profiles.

Only measured scaling failure may promote a Replication Graph or Iris migration pilot.

### 28.3 Lag compensation / server-side rewind boundary — DEFER UNTIL MEASURED NEED

Do not import another shooter framework solely to obtain lag compensation.

Current factual hit/damage authority stays on the server. Network Emulation testing must first establish whether high-latency hitscan fairness is unacceptable.

If a later server-side-rewind pilot is justified:

- retain a bounded server history only for the actors/hitboxes actually required;
- accept a client shot timestamp/request, never a client-declared hit/damage result;
- validate/clamp timestamp age and maximum rewind window;
- rewind/query only for factual hit validation, then restore current authoritative state;
- prevent duplicate/replayed shot requests from creating additional hits/ammo events;
- projectile weapons, grenades, vehicles and world physics remain server-simulated unless separately designed and proven.

PASS45 does not require rewind architecture merely because the game is multiplayer.

### 28.4 Collision profiles, object types and trace channels — CENTRALIZE

The project currently uses general collision channels such as `ECC_Visibility` for several different gameplay meanings. Do not keep expanding that ambiguity.

Required direction:

- define and document named project collision profiles/object types/trace channels when semantics differ materially, for example interaction, weapon/projectile query, grenade safe placement, vehicle interaction and AI/visibility where justified;
- do not convert every existing trace in one blind refactor; migrate the paths where shared-channel behavior creates false blocking, missed interaction or unintended coupling;
- one collision profile must clearly define collision vs query-only behavior for production pickups, dropped weapons, grenades, characters, vehicles, doors, destructibles and invisible debug/collision helpers;
- visible mesh identity and collision proxy ownership remain separate concerns;
- Physical Material/Surface Type is surface identity and may drive impact/footstep/tyre presentation, but it is not a substitute for collision-channel semantics;
- newly imported assets must not silently ship with default collision that blocks players/nav/projectiles incorrectly.

### 28.5 Navigation runtime-generation policy — PILOT A CHEAPER OWNER

Current config uses `RecastNavMesh RuntimeGeneration=Dynamic`. Full Dynamic can rebuild geometry-affected tiles at runtime and is not automatically justified for a mostly authored city.

Required pilot:

`current Dynamic -> representative bot/door/vehicle/destruction test -> Dynamic Modifiers Only proof -> CPU/nav correctness comparison`

Promotion rule:

- prefer `Dynamic Modifiers Only` if Oster only needs blockers/cost changes/NavLinks/NavModifiers and it preserves required bot paths;
- retain full `Dynamic` only where gameplay genuinely creates/removes walkable geometry or another measured requirement needs runtime geometry generation;
- doors/gates/mounted positions/destruction should use Nav Modifier / Nav Link / Smart Link style mechanisms where they express the gameplay change without broad geometry rebuilding;
- movable vehicles/props should not constantly dirty navigation unless their gameplay collision truly needs to affect bot routing;
- Navigation Invokers remain a scale-triggered pilot for very large navigable areas, not an automatic migration;
- authoritative AI/navigation decision remains server-side; clients do not become navigation truth owners.

No nav-generation setting is accepted solely from source/config. Representative bot pathing plus performance evidence is required.

### 28.6 UE Data Validation — ADOPT INSTEAD OF MORE PATH-ONLY CHECKERS

Use Unreal Engine's Data Validation framework (`IsDataValid`, `UEditorValidatorBase`, `UEditorValidatorSubsystem`) for asset/content facts that require the Editor/asset graph rather than inventing endless text/path verifiers.

Good production validators include:

- required production mesh/material/texture dependencies exist and load;
- no final required visual uses default/BasicShape/proxy content;
- skeletal assets have expected skeleton/animation compatibility;
- required material slots and real texture dependencies are present;
- LOD/mip/collision requirements fit the relevant production asset class;
- prohibited startup hard references/dependency cycles are absent where the rule can be proven through asset data;
- external/imported content has an approved provenance/register entry where applicable;
- evidence-bound landmark assets cannot silently resolve to unrelated generic content.

CI/editor route:

- add/retain C++ validators where rules are stable and load-aware;
- use `UnrealEditor-Cmd.exe <project>.uproject -run=DataValidation` in the appropriate editor-capable validation route;
- a new UE validator may replace a bespoke Python/source verifier **only after** it proves equivalent or stronger coverage; stale duplicate verifiers are then physically removed according to the reuse-first rule;
- Data Validation is structural/content evidence, not direct visual/audio/runtime acceptance.

### 28.7 Tick, timers and polling budget — EVENT-DRIVEN BY DEFAULT

New Actors/Components/SubSystems must not receive per-frame Tick merely because it is convenient.

Rules:

- disable Tick by default unless a frame-rate-dependent responsibility genuinely requires it;
- prefer input events, RepNotify/delegates, animation updates, physics callbacks or bounded timers for non-frame-critical work;
- far/irrelevant AI, VFX, audio and presentation work should use significance/budgeting/intervals when profiling proves benefit;
- client-only camera/FOV/presentation Tick must not execute equivalent cosmetic work on dedicated server;
- polling that merely reimplements a stable engine subsystem is a migration target, not a pattern to copy;
- item 33 profiling must identify high-frequency Tick/timer owners under representative gameplay, not just aggregate FPS.

### 28.8 Dedicated-server asset and presentation stripping — REQUIRED

The repository already has a dedicated-server target. Preserve that architecture and make reusable systems respect it.

On dedicated server:

- do not create/play cosmetic audio, camera shakes, UI, local first-person arms or purely visual Niagara effects;
- avoid loading client-only heavy meshes/materials/textures/audio/animation when server gameplay does not require them;
- server still loads/cooks the data needed for collision, authoritative physics, gameplay classes, damage, navigation and validated asset identity as required;
- any new MetaSound/Soundscape/Motion Matching/Niagara/PCG/vehicle presentation integration must explicitly prove that dedicated server does not pay unnecessary client presentation cost;
- packaged server + client versions/build identifiers must fail visibly on incompatible builds rather than silently running mismatched gameplay contracts.

### 28.9 Final architecture-freeze rule

After sections 26–28, the broad reuse/architecture baseline is considered **complete enough to execute**.

Do not keep expanding PASS45 with another broad framework survey merely because another UE feature or GitHub project exists. A new architecture item may be added only when at least one of these is factual:

1. a current runtime/content defect is not covered by sections 0–28;
2. profiling proves a measurable CPU/GPU/network/memory/loading bottleneck;
3. a user gameplay requirement introduces a genuinely new subsystem;
4. an engine/version/license change invalidates an existing decision.

Otherwise continue the existing 36-item execution order. This prevents the audit itself from becoming the project.

This section changes engineering policy only. Formal progress remains tied to the existing 36-item checklist, runtime truth remains factual local UE 5.8 evidence, and PR #94 remains OPEN / UNMERGED until the existing acceptance rule passes.
