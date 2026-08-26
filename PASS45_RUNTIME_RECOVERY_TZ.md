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

### Current corrective source state — 2026-08-26 — SOURCE-CODED / RUNTIME UNTESTED

A repository weapon-audio fallback now closes the **source-level silent-shot path** without pretending that generic sound identity is final authored content:

- an assigned authored `UOCWeaponAudioProfile` still wins whenever it contains the requested event;
- if the requested near-shot event is unassigned/empty, `UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()` lazily creates a transient presentation-only profile;
- the represented AK first prefers the already tracked `/Game/AK-47/.../AK47_Fire_Cue`, `Reload_Cue` and `AK47_Empty_Cue` assets;
- other current weapons may temporarily reuse the tracked `/Game/R13/Audio/gunfire_sfx` shot and tracked reload assets rather than disappear acoustically;
- if no authored distant tail exists, the fallback uses the factual near report at reduced distance volume rather than becoming fully silent;
- tracked `/Game/R13/Audio/shotguncock` is wired only to the pump-action fallback; bolt and lever mechanical sounds remain explicit **AUDIO CONTENT GAP** until exact content exists;
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

The 2026-08-26 runtime rejected both the grenade body and the smoke presentation. Source inspection confirmed why: the grenade rendered an Engine BasicShape sphere, while `AOCSmokeCloud` rendered a cluster of Engine BasicShape spheres. A repository tree check found the tracked R13 grenade mesh but no authored smoke/Niagara payload, so the source must fail closed rather than invent fake smoke out of more geometry.

### Current corrective source state — 2026-08-26 — PARTIAL SOURCE-CODED / RUNTIME UNTESTED

- visible `/Engine/BasicShapes/Sphere.Sphere` ownership is removed from `AOCGrenadeProjectile`;
- grenade collision remains a small invisible `USphereComponent`, separate from visual truth;
- the tracked `/Game/R13/Weapons/grenade.grenade` mesh is now loaded as the current recognizable shared grenade body with uniform bounds-based scaling;
- successful visual resolution emits `PASS45_GRENADE_PRODUCTION_VISUAL_READY ... primitive_visible=0 production_visual=1`;
- missing/invalid production grenade content emits `PASS45_GRENADE_PRODUCTION_VISUAL_FAIL ... primitive_visible=0 runtime_acceptance=0`; the old sphere is never shown as fallback;
- the old `AOCSmokeCloud` `SmokePuff_*` BasicShape cluster is physically retired;
- smoke gameplay radius/lifetime remains available for gameplay/AI queries, but no primitive visual is rendered;
- because no accepted authored smoke particle/Niagara payload is currently present, smoke emits explicit `PASS45_SMOKE_VFX_CONTENT_GAP ... authored_vfx=0 primitive_visible=0 runtime_acceptance=0`;
- a failed smoke gameplay-volume spawn emits `PASS45_SMOKE_GAMEPLAY_VOLUME_FAIL`;
- source guard `VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py` rejects resurrection of the sphere grenade or fake smoke-ball cluster;
- workflow `.github/workflows/pass45-grenade-smoke-primitive-retirement.yml` covers the new guard;
- cumulative `RUN_ALL_VERIFY.py` includes the grenade/smoke primitive-retirement verifier.

This is deliberate fail-closed behavior, not final smoke completion. A missing effect is a visible content gap; twelve grey spheres pretending to be smoke are not an improvement merely because the computer can render them enthusiastically.

Still pending:

- authoritative throw path currently decrements grenade inventory before spawn success; inventory commit must move after successful projectile spawn;
- throw origin must be swept/validated outside the pawn/weapon/world collision instead of assuming `camera + forward * 70 cm` is always safe;
- first-person hand/throw presentation remains required;
- distinct accepted frag/smoke/flash grenade models or type-specific visual treatment remain content work; the current tracked R13 mesh is only a shared recognizable body;
- authored growing smoke VFX with useful visual sight blocking remains **CONTENT GAP**;
- local UE 5.8 bounce/roll/fuse/visual scale/throw feel remains unverified.

Requirements:

- recognizable fragmentation/smoke/flash models;
- visible first-person throw presentation;
- gravity/collision/bounce/roll;
- defined fuse start;
- no floating grenade;
- growing volumetric/particle smoke with useful sight blocking;
- distinct frag/flash/smoke VFX and audio;
- safe throw origin outside player collision/weapon;
- grenade count decrements only after factual projectile spawn succeeds;
- missing authored smoke VFX must fail visibly and may never fall back to BasicShape puffs.

Acceptance:

- no visible BasicShape grenade or smoke geometry;
- production grenade mesh loads and is readable in hand/flight/ground states;
- failed projectile spawn consumes zero grenade inventory;
- near-wall throw cannot spawn inside the player or solid geometry;
- bounce/roll settles coherently without floating;
- frag/smoke/flash presentation is distinguishable;
- smoke expands into accepted authored VFX and materially obscures sight without fake sphere geometry.

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

## 22. Current source implementation milestone — 2026-08-26 weapon firing/drop/action/ADS/audio/primitive/grenade-smoke-retirement pass

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
- repository fallback prevents an unassigned/empty near-shot profile from silently swallowing a factual shot;
- exact tracked AK cues are preferred for AK; tracked R13 gunfire/reload/impact assets are temporary source fallbacks for current gaps;
- pump fallback may use tracked `shotguncock`; bolt/lever mechanical audio remains fail-visible content gap;
- `PASS45_WEAPON_AUDIO_FALLBACK_READY` / `PASS45_WEAPON_AUDIO_CONTENT_GAP` distinguish source fallback from missing content;
- concrete weapon variants and launcher hide source BasicShape geometry before production load failure can render it;
- real weapon fallbacks attach to unscaled `WeaponRoot` while the invisible physics root retains collision authority;
- strict runtime evidence requires `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY` and forbids `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL`;
- tracked R13 grenade mesh replaces the visible Engine sphere fail-closed;
- primitive smoke-ball components are physically removed; missing authored smoke VFX is now an explicit content gap instead of fake geometry;
- grenade/smoke primitive-retirement has a dedicated verifier/workflow and is included in cumulative source verification;
- source verifiers reject resurrection of old feedback/action shortcuts, fake ADS calibration, silent-profile acceptance and visible primitive weapon/grenade/smoke fallbacks;
- cumulative `RUN_ALL_VERIFY.py` includes weapon firing, action, ADS, audio-fallback, primitive-retirement and grenade/smoke guards.

Still not runtime accepted: compile on local UE 5.8, recoil feel/release, action timing, procedural cue quality, authored bolt/pump/lever moving-part animation, exact mechanical sound content, exact per-weapon sound identity/mix, exact per-weapon sight socket/offset calibration, production hierarchy, drop settling, muzzle alignment, launcher visual, rendered zero-primitive rack proof, grenade visual scale/throw behavior, safe throw spawn semantics and authored smoke VFX.

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
19. [x] Close the source-level silent-shot path with an event-local repository audio fallback and dedicated verifier/workflow; keep runtime audibility and exact sound identity unaccepted.
20. [ ] Replace temporary generic audio fallback with accepted exact per-weapon shot/reload/distant/mechanical profiles and close bolt/lever manual-action audio gaps.
21. [x] Source-retire visible primitive weapon/pickup/launcher fallbacks: hide before production load, preserve invisible collision authority, add hard runtime ready/fail markers and strict evidence gate. **Rendered UE acceptance remains pending.**
22. [x] Source-retire primitive grenade/smoke visuals: use tracked R13 grenade mesh fail-closed; physically remove fake smoke spheres; add dedicated guard/workflow. **Rendered grenade acceptance and real smoke VFX remain pending.**
23. [ ] Correct grenade throw semantics: safe swept origin, inventory commit only after successful spawn, first-person throw presentation; integrate accepted authored smoke VFX and distinct frag/smoke/flash presentation.
24. [ ] Correct Museum/Culture House/Silpo visible identity and separation.
25. [ ] Replace rejected vegetation family.
26. [ ] Rebuild HMMWV M2 ring/shield/gunner hierarchy with 360° yaw and correct camera.
27. [ ] Calibrate HMMWV gameplay top speed to >=80 km/h without breaking handling.
28. [ ] Close BTR white material state across pre/post possession.
29. [ ] Correct BTR forward axis and remote operator monitor/optic gameplay.
30. [ ] Raise core world/material/LOD visual fidelity above prototype state without lowering native render scale.
31. [ ] Validate fullscreen + 60 FPS + thermal soak after visual fixes.
32. [ ] Validate tactical map screenshot.
33. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots.
34. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 24. Final acceptance gates

### Gate A — source/build/import
current branch/head; source workflows; Stein R3 fresh load; production HMMWV/M2/BTR import/fresh load; UE 5.8 editor build exit 0.

### Gate B — world rendering
daylight/exposure; stable ground/roads/sidewalks; no black-world or blown-out scene; screenshots.

### Gate C — weapon materials and visible content
real accepted visual/material/texture chain; `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY`; no `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL`; no visible primitive fallback; launcher production visual valid; unresolved exact items remain CONTENT GAP rather than visible cubes/cylinders.

### Gate D — weapon firing / ordnance physics
factual shot count = ammo = recoil = muzzle = audio; production muzzle origin; no ghost recoil; no release downward kick; exact selector modes; deterministic finite Burst3 if enabled; manual-action server gate plus accepted visible action animation and audible action-specific content; exact per-weapon ADS alignment accepted; no temporary generic audio fallback remains on a final accepted weapon; drop physics; no primitive grenade/smoke geometry; grenade inventory commits only after successful spawn; safe throw origin; accepted type-specific grenade presentation and authored smoke VFX.

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

The newest weapon firing/muzzle/drop/action/presentation/audio-routing/ADS-diagnostic/repository-audio-fallback/primitive-retirement/grenade-smoke-primitive-retirement corrections are **CODED_UNTESTED**. They may not be described as fixed in runtime until a current-head local UE 5.8 build and playtest proves them.