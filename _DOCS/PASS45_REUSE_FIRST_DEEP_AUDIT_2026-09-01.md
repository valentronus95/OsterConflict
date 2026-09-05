# PASS45 REUSE-FIRST DEEP AUDIT — 2026-09-01

Parent: `PASS45_RUNTIME_RECOVERY_TZ.md`
Bound policy: `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`
Target: Oster Conflict / Unreal Engine 5.8.x / Windows / multiplayer FPS
Audit date: 2026-09-01
Runtime truth at audit start: **RUNTIME REJECTED 2026-08-31**

## 1. Purpose

This audit answers one question for every large gameplay/presentation subsystem: **is it cheaper, safer and better to keep repairing Oster's custom implementation, or to migrate that responsibility to a mature Unreal Engine / legally reusable implementation and then physically delete the obsolete duplicate?**

The decision is not based on popularity. Each candidate is checked for:

- current 2026 maintenance state;
- UE 5.8 compatibility or migration cost;
- license / redistribution constraints;
- multiplayer suitability;
- packaged-build risk;
- performance and memory risk;
- overlap with current Oster code;
- whether it removes custom code rather than adding a second permanent owner;
- whether the candidate solves a generic engine problem or an Oster-specific game-design problem.

Decision labels:

- **ADOPT** — use as the preferred replacement owner, behind an isolated proof and acceptance gate where required;
- **STANDARDIZE / KEEP** — Oster is already using the right engine family; remove unnecessary custom duplication but do not rewrite the gameplay contract;
- **PILOT** — technically promising but not accepted as production owner until a focused packaged/network/performance proof passes;
- **REFERENCE ONLY** — useful implementation/design reference; do not add as a runtime dependency;
- **REJECT / DO NOT IMPORT** — license, version, design or architecture makes it a poor fit.

## 2. Current Oster baseline found during source audit

The audit is grounded in the current canonical branch, not an imaginary clean project.

### 2.1 Vehicle physics is substantially custom

`AOCVehicleBase` currently owns a custom physics vehicle built around a simulating `UBoxComponent`. It manually implements:

- four visual wheels;
- line-trace suspension points;
- spring force and damping;
- drive force;
- lateral grip;
- rolling braking;
- handbrake force;
- steering torque;
- speed limiting;
- custom wheel spin/steering visuals;
- server-side input RPC and `SetReplicateMovement` networking.

This is exactly the kind of generic vehicle-physics responsibility that should not continue growing as a bespoke Oster solver.

### 2.2 Input is already on Enhanced Input

`OsterConflict.Build.cs` already depends on `EnhancedInput`, and the project explicitly enables the plugin. Vehicle input also uses `UInputAction`, `UInputMappingContext` and `UEnhancedInputComponent`.

Decision: **KEEP Enhanced Input. Do not replace it.** Any future CommonUI/Lyra work must integrate with the existing Enhanced Input owner rather than creating another gameplay input layer.

### 2.3 Niagara is already present

`OsterConflict.Build.cs` already depends on `Niagara`, and PASS45 already migrated smoke presentation away from BasicShape spheres into authored Niagara content.

Decision: **STANDARDIZE / EXPAND Niagara**, not replace it.

### 2.4 Character animation code is thin state extraction

`UOCCharacterAnimInstance` currently derives speed, direction, airborne/crouched/sprinting/aiming/in-vehicle/reload/downed/dead states from the character. This is a reasonable data bridge, but it is not yet a high-fidelity locomotion solution by itself.

Decision: keep gameplay movement authority separate, then pilot a stronger animation presentation system rather than replacing movement rules and animation at the same time.

### 2.5 AI already uses native AI Perception and Navigation

`AOCAIController` already owns `UAIPerceptionComponent` + sight sense and uses Unreal navigation, but its high-level decisions are a custom C++ think loop running on a timer/tick cadence.

Decision: **KEEP AI Perception / Navigation; PILOT Behavior Tree + Blackboard + EQS for high-level decision ownership.** Do not throw away working perception merely because Behavior Trees exist.

### 2.6 Destruction is prototype-grade custom presentation

`AOCDestructibleProp` currently hides the intact mesh and creates random Engine `Cube` BasicShape components as local physical chunks.

Decision: this presentation owner should be replaced for accepted destructible content. The cube-chunk implementation is prototype-only and must be physically removed after the replacement gate passes.

### 2.7 Audio architecture is event/profile based but presentation is simple

`UOCWorldAudioComponent` chooses a `USoundBase` from a profile and uses `PlaySoundAtLocation`. `AOCAmbientAudioZone` manually polls listener position every 0.20 s, starts/stops a bed and randomly schedules birds/wind/leaves/animals/dogs/traffic/water one-shots.

This is a useful gameplay/event shell, but the final mix, procedural variation, indoor/outdoor state and ambience distribution should move toward Unreal's dedicated audio systems instead of expanding C++ timer logic.

## 3. Vehicles — audited decision

### 3.1 Chaos Vehicles

**Decision: ADOPT THROUGH A REQUIRED PILOT, then migrate wheeled vehicle physics if the pilot passes.**

Why:

- it is Unreal's own vehicle simulation family;
- it directly models wheels, suspension, steering, brakes and powertrain, which currently duplicate large portions of `AOCVehicleBase`;
- UE documentation provides a standard vehicle setup around skeletal mesh, Physics Asset, Wheel Blueprints, torque curve, Animation Blueprint and vehicle Blueprint;
- it keeps vehicle physics inside the engine physics stack rather than a project-specific line-trace spring solver.

Important caution:

UE 5.8 API documentation still marks the `ChaosVehiclesPlugin` as Experimental / use caution when shipping. Therefore this is **not** an unconditional blind replacement. It gets a focused proof first.

Pilot vehicle:

1. one disposable test car using the same approximate dimensions/mass as an Oster civilian car;
2. no production HMMWV/BTR migration yet;
3. packaged UE 5.8 Windows build;
4. server + at least one client;
5. steering, braking, reverse, handbrake, slopes, curb, asphalt/dirt transitions;
6. replication/correction observation;
7. CPU/frame cost comparison with existing `AOCVehicleBase`;
8. no Museum teleport regression on enter/exit.

Promotion rule:

If the pilot is equal or better in handling, networking and performance, Chaos Vehicles becomes the sole wheeled-physics owner for civilian vehicles and HMMWV.

### 3.2 Exact custom vehicle code to retire after accepted migration

Once a production Chaos vehicle proves parity, remove the duplicated physics portions of `AOCVehicleBase`, including equivalent fields/functions for:

- `SuspensionPointsLocal`;
- suspension trace length/radius tuning owned only by the custom solver;
- spring stiffness/damping force calculation;
- `SimulateVehicleServer()` custom solver ownership;
- `ApplySuspensionServer()`;
- `ApplyDriveAndGripServer()`;
- custom lateral grip force;
- custom rolling brake force;
- custom handbrake force integration;
- custom steering torque integration;
- custom visual wheel spin code if the Chaos wheel animation path owns it;
- obsolete tests/verifiers that require the removed custom force solver.

Do **not** delete Oster-specific:

- seat/enter/exit rules;
- driver/gunner/passenger authority;
- cameras;
- vehicle health/damage game state;
- HMMWV M2 gameplay;
- BTR station rules;
- vehicle UI;
- spawn/deployment logic;
- team permissions;
- Oster audio/VFX event hooks.

### 3.3 CARLA

License: MIT for reviewed CARLA code branch.
Current UE5 development line reviewed: UE 5.5 / Chaos migration.

**Decision: REFERENCE ONLY / selective donor after file-level license review. Do not add CARLA as an Oster runtime dependency.**

Reasons:

- CARLA is an autonomous-driving simulator, not a Battlefield-like game;
- its server/sensor/autonomy stack would add enormous irrelevant scope;
- current CARLA recommended hardware is materially heavier than Oster's target;
- useful ideas include torque/suspension/surface tuning, vehicle content setup, recorder/debugging and data-driven vehicle parameters.

If a specific CARLA source file is ever copied/adapted, pin the exact commit/file and preserve MIT notice in the third-party register.

### 3.4 Chaos Modular Vehicles

UE 5.8 explicitly documents this as Experimental.

**Decision: PILOT ONLY.**

Potential value:

- detachable/modular vehicle structures;
- integrated Geometry Collection vehicle parts;
- client-predicted/server-authoritative architecture;
- future modular vehicle damage.

Do not make HMMWV/BTR depend on it during PASS45 unless an isolated UE 5.8 packaged multiplayer proof clearly beats the simpler approach.

## 4. General physics

### Chaos rigid-body physics

**Decision: STANDARDIZE / KEEP.**

Use engine physics components/Chaos as the generic owner for:

- dropped weapons;
- grenades;
- loose props;
- ragdolls;
- debris;
- physical hit response;
- doors/gates where physical hinges are justified.

PASS45 already uses engine physics for several of these. Do not rewrite them merely to rename them “Chaos”. Remove only custom loops that duplicate gravity, settling, bounce or constraint behavior without adding real gameplay value.

## 5. Destruction and explosions

### 5.1 Chaos Destruction / Geometry Collections / Physics Fields

**Decision: SELECTIVE ADOPT.**

Use for:

- accepted breakable props;
- windows/fences/doors where a fractured state improves quality;
- selected vehicle pieces if later gameplay needs them;
- selected non-critical structures.

Do not make the whole city destructible. Full-city destruction would multiply replication, navmesh, save-state and performance complexity for little current PASS45 value.

### 5.2 Old destruction code to remove

After one wood/metal/masonry test set passes:

- remove `AOCDestructibleProp`'s BasicShape cube fragment generation as accepted production presentation;
- remove `TransientChunks`/random Cube creation if Geometry Collection owns the fractured presentation;
- keep the Oster durability/damage-authority layer only if it remains the authoritative gameplay state;
- replicate only the meaningful destroyed/intact state and required gameplay collision changes;
- keep cosmetic debris local when it cannot alter authoritative gameplay.

### 5.3 Niagara for explosion presentation

**Decision: ADOPT / STANDARDIZE.**

Niagara becomes the common presentation owner for:

- frag blast;
- flash world effect;
- smoke;
- fire;
- muzzle flashes;
- impacts by material;
- dirt/dust/concrete/wood/metal particles;
- vehicle damage smoke/fire;
- environmental particles.

Gameplay damage never comes from Niagara. Correct flow:

`server detonation -> gameplay damage/impulse/state -> replicated presentation event -> Niagara + audio + decal + camera/hearing presentation`

## 6. Audio — audited decision

### 6.1 Keep the Oster event authority, replace/upgrade the presentation layer

Oster already has useful event/profile components for weapon/world/vehicle/character audio. These should remain the gameplay-facing API unless a later migration proves a cleaner interface.

Do **not** replace factual shot/action timing with MetaSounds. MetaSounds should render a sound from a factual gameplay event, not decide whether a shot happened.

### 6.2 MetaSounds

UE 5.8 documents MetaSounds as a high-performance DSP graph system, but its Quick Start still labels the feature Beta / use caution when shipping.

**Decision: PILOT, then selective ADOPT. Not an all-at-once rewrite.**

Best first targets:

- vehicle engine RPM/load;
- layered explosion profile;
- procedural wind/ambient bed;
- selected weapon report layering where exact source recordings exist.

Promotion criteria:

- packaged UE 5.8 build;
- no missing audio after fresh load;
- no factual-event double playback;
- voice/concurrency budget acceptable;
- no measurable frame/audio-thread regression;
- dedicated server remains free of cosmetic playback work.

### 6.3 Audio Modulation

**Decision: ADOPT.**

Use for mix state rather than writing more custom volume multipliers everywhere:

- Master;
- Weapon/SFX;
- Vehicle;
- Ambience;
- UI;
- Music;
- Voice when added;
- indoor/outdoor mix changes;
- vehicle cabin/exterior transition;
- flash/concussion/low-health temporary filtering/ducking;
- menu/gameplay state mixes.

`UOCAudioUserSettings` may remain the user-preference authority but should drive modulation buses/mixes instead of every playback site multiplying volumes independently forever.

### 6.4 Soundscape

UE 5.8 documents Soundscape as Beta.

**Decision: PILOT ONLY for ambient world distribution.**

It is promising because it procedurally streams/composes ambience as the player moves, which overlaps directly with `AOCAmbientAudioZone` timer/polling logic.

Pilot in one park/central Oster test area:

- birds;
- wind/leaves;
- distant dogs;
- distant traffic;
- localized utility/electrical hum;
- voice count and CPU/audio-thread profiling.

Only after the pilot passes should `AOCAmbientAudioZone`'s polling/random scheduling be retired for areas owned by Soundscape.

### 6.5 Final sound inventory required

Weapon families:

- near outdoor shot;
- indoor shot;
- distant tail;
- suppressor where applicable;
- dry fire;
- magazine out/in;
- charging handle/bolt release;
- bolt/pump/lever action;
- selector;
- equip/handling;
- casing/secondary mechanics where useful.

Projectile/environment:

- impact by Physical Material;
- ricochet when plausible;
- supersonic crack/whiz from factual projectile path;
- debris secondary impacts.

Vehicles:

- start/stop;
- idle;
- RPM/load;
- gear/transmission;
- brake/skid;
- tire asphalt/dirt/grass/gravel;
- suspension/body hit;
- interior/exterior filtering;
- damage/fire;
- turret/M2 routing.

Character:

- footsteps by Physical Material;
- jump/land;
- restrained gear/cloth;
- interaction sounds;
- vehicle enter/exit;
- grenade handling;
- hit/death if retained by design.

Environment:

- wind exposure;
- foliage type/area;
- birds/insects;
- distant dogs;
- distant road activity;
- electrical/utility hum;
- room tone;
- weather;
- fires;
- contextual distant combat only when game state actually calls for it.

### 6.6 Audio source licensing

- Freesound: **per-file review mandatory**; CC0 preferred. CC-BY requires attribution; BY-NC is not acceptable for unrestricted/commercial-ready production use.
- Fab audio: allowed only under the exact acquired license/entitlement; source assets may not be redistributed standalone.
- unknown YouTube/ripped game/movie audio: **DO NOT IMPORT**.

## 7. Character movement and animation

### 7.1 Native CharacterMovement

Oster already uses Unreal `CharacterMovementComponent` for gameplay movement.

**Decision: KEEP gameplay movement authority unless a measured defect requires replacement.**

Do not replace working movement merely because a flashy locomotion repository exists.

### 7.2 Game Animation Sample + Motion Matching / Pose Search

Epic explicitly allows the Game Animation Sample systems/assets to be migrated into an Unreal project and documents Motion Matching in UE 5.8 as a responsive alternative to state machines/blendspaces.

**Decision: PILOT for third-person locomotion presentation.**

Scope:

- idle;
- walk/run/sprint;
- starts/stops;
- direction changes;
- crouch locomotion if covered/adapted;
- jump/fall/land if compatible;
- slope/stair presentation.

Do not use Motion Matching as a replacement for:

- M700 bolt;
- Remington pump;
- Lever Action cycle;
- reload;
- grenade throw;
- vehicle enter/exit;
- M2 operation.

Those are discrete authored actions and should use Montages/animation sequences/stateful action bridges.

### 7.3 IK Rig / IK Retargeter

**Decision: ADOPT.**

Use to:

- retarget reusable animation packs to Oster production skeletons;
- align hands to weapons;
- reduce per-weapon skeleton incompatibility;
- improve foot placement where appropriate.

### 7.4 Control Rig

**Decision: ADOPT as an authoring/correction tool, not gameplay authority.**

Use for:

- weapon-hand corrections;
- foot/leg procedural adjustment;
- authored moving-part rigs where source mesh permits;
- animation cleanup directly in UE.

### 7.5 Motion Warping

**Decision: SELECTIVE ADOPT.**

Good for:

- vehicle enter/exit alignment;
- mounting M2/turret station;
- contextual mantle/vault if the design keeps it;
- doors/interaction positioning where a target transform matters.

### 7.6 ALS Community

Reviewed community line: MIT source, published compatibility currently trails UE 5.8.

**Decision: REFERENCE ONLY / backup pilot, not preferred primary migration.**

Reason: Epic's own Motion Matching/Game Animation Sample path is closer to current UE 5.8 and avoids importing another broad locomotion owner.

### 7.7 ALS Refactored

MIT, actively developed, strong network-multiplayer focus. Latest released support reviewed is UE 5.7; community reports indicate main can compile on 5.8 with small source adjustments, but there is no formal 5.8 release at audit time.

**Decision: PILOT-ELIGIBLE BACKUP, not current default.**

If Epic Motion Matching pilot fails to meet Oster's multiplayer/weapon-overlay needs, ALS Refactored is the stronger external locomotion candidate to test before inventing a new custom system.

### 7.8 PBCharacterMovement

MIT, but intentionally reproduces HL2/Source-style movement including bunnyhopping, surfing, ramp sliding and related mechanics; published binaries are for UE 5.5.

**Decision: REJECT AS DEFAULT OSTER MOVEMENT OWNER; REFERENCE ONLY.**

Oster targets Battlefield-like grounded movement, so importing a movement component built around HL2 quirks would create design work rather than remove it.

## 8. AI / bots

### 8.1 AI Perception

Already used in Oster.

**Decision: KEEP.**

### 8.2 Behavior Trees + Blackboard

**Decision: PILOT, likely ADOPT for high-level bot state.**

Move high-level modes such as:

- attack/defend objective;
- revive;
- resupply;
- use vehicle;
- move to cover;
- engage target;
- regroup/follow squad order;

out of one growing custom `RunDecisionLoop()` after parity tests prove the Behavior Tree equivalent.

### 8.3 EQS

**Decision: ADOPT for environment-choice queries.**

Good targets:

- cover points;
- firing positions;
- revive approach;
- ammo/health pickup selection;
- safe route/position around smoke;
- vehicle/objective choice scoring where spatial tests dominate.

Do not use EQS for trivial deterministic state checks that are cheaper as C++ conditions.

### 8.4 Smart Objects

**Decision: PILOT / selective ADOPT.**

Good future uses:

- doors/gates;
- mounted weapon stations;
- vehicle seats;
- resupply boxes;
- contextual defensive positions.

### 8.5 Mass AI / Mass Entity

**Decision: NOT NOW.**

Only investigate if target bot counts become high enough that ordinary Actor/AIController architecture is a proven bottleneck. Do not add Mass complexity merely because it exists.

## 9. UI / multiplayer framework

### Lyra

Lyra is an Epic sample/learning project, not a permissive independent OSS game to copy indiscriminately.

**Decision: REFERENCE + selective migration only.**

Potentially reusable patterns:

- session/login flow;
- team/game-state architecture;
- settings/device profiles;
- modular UI patterns;
- interaction/equipment ideas;
- EOS integration if Oster later requires internet matchmaking.

Do not replace the whole Oster game mode/weapon/inventory stack with Lyra during PASS45.

### Common User

Epic documents Common User as a standalone plugin usable in other projects and supporting login/auth/session flow.

**Decision: POST-PASS45 PILOT if online-service/session requirements exceed the current custom flow.**

### CommonUI + Enhanced Input

UE 5.8 documentation still warns that the Enhanced Input integration path is Experimental / not recommended for shipping in that configuration.

**Decision: DO NOT migrate gameplay UI input to CommonUI during PASS45. Keep existing Enhanced Input/UMG.**

## 10. World building / performance

### PCG

**Decision: SELECTIVE ADOPT.**

Good for non-identity-critical repeated content:

- grass/ground clutter;
- generic shrubs;
- litter;
- stones;
- generic roadside props;
- controlled foliage scattering.

Hard rule: PCG may not author, move or replace evidence-bound Museum/Silpo/Culture House/water-tower/stadium identity geometry.

### World Partition / HLOD

**Decision: HLOD SELECTIVE PILOT; World Partition migration DEFER until map audit proves the payoff.**

HLOD can reduce draw calls for distant static content, but converting the whole project to World Partition during an active runtime-recovery pass is high-risk.

If current Oster map is not already World Partition, migration is a separate architecture slice after PASS45 runtime recovery unless profiler evidence makes it necessary sooner.

### Vegetation item 27

The rejected KiteDemo tree family should not be repaired indefinitely merely because it already exists.

Decision order:

1. find verified UE 5.8-compatible production foliage;
2. test material/LOD/startup behavior;
3. use Foliage/PCG/instancing as appropriate;
4. remove quarantined rejected tree-owner assumptions after replacement passes;
5. keep exact Oster placement/species claims honest to references.

## 11. Models, materials and reusable world assets

### Poly Haven

License: CC0.

**Decision: APPROVED SOURCE CLASS for generic materials/HDRIs/models after asset-quality review.**

Good uses:

- concrete;
- brick;
- plaster;
- asphalt;
- soil;
- wood;
- generic metal;
- HDRIs when appropriate;
- generic props if visual style matches.

### Kenney

Asset pages reviewed: CC0/public-domain licensed.

**Decision: APPROVED SOURCE CLASS FOR NON-PRODUCTION/UTILITY OR STYLE-COMPATIBLE GENERIC ASSETS.**

Many Kenney assets are intentionally stylized/low-poly, so they are not automatically suitable for Oster's realistic final look. They can still replace BasicShape/debug/utility content where style is acceptable.

### Fab / Megascans / commercial or free Fab assets

Fab Standard License permits incorporation into a project but forbids standalone redistribution of the asset.

**Decision: LICENSED SOURCE, NOT “OPEN SOURCE”.**

Requirements:

- record entitlement/license;
- do not expose source assets as standalone downloads;
- verify public-repository implications before committing raw source packs;
- record exact package/version in third-party register.

### Unique Oster landmarks

**Decision: CUSTOM / REFERENCE-OWNED.**

Do not replace:

- Museum;
- Silpo;
- Culture House;
- water tower;
- stadium;
- other photo/geography-bound landmarks

with a generic building pack. Generic assets may supply doors/windows/materials/furniture/props only when they do not falsify landmark identity.

## 12. Drones

### Legacy Microsoft AirSim

MIT, but Microsoft states the old project is being archived/no longer developed in favor of Project AirSim.

**Decision: REFERENCE ONLY.**

### IAMAI Project AirSim

MIT, actively continued by former AirSim engineers. Current reviewed support is UE 5.2 and 5.7, not 5.8.

**Decision: FUTURE DRONE PILOT / REFERENCE, NOT CURRENT PASS45 DEPENDENCY.**

If FPV drones enter production scope:

- isolate the drone plugin/sim libraries;
- recompile/adapt for UE 5.8;
- use only physics/controller pieces actually needed by gameplay;
- do not import autonomy/GIS/ROS/sensor infrastructure unless the game explicitly needs it;
- compare network/performance cost against a simpler Oster quadrotor gameplay pawn.

## 13. Explicit reject list

### OpenTournament

Repository is public/source-visible, but its own legal notice says code is subject to its project license and content may not be shared/redistributed without approval.

**Decision: DO NOT COPY OR IMPORT code/content into Oster. Reference ideas only.**

Public GitHub source is not automatically open-source reusable code. This project is the canonical example of why the provenance register is mandatory.

### Unknown GitHub/Gumroad/YouTube asset dumps

**Decision: DO NOT IMPORT** unless license and provenance are explicit and compatible.

## 14. PASS45 mapping

Apply the audited decisions without reopening already source-closed work only to still-open or directly affected tasks:

- **item 16** manual-action animation: authored sequence + IK/Control Rig/retargeting pipeline; no procedural whole-weapon fallback resurrection;
- **item 18** ADS: IK/Control Rig/socket calibration may assist, but factual sight alignment remains Oster-specific;
- **item 20** weapon audio: event authority stays Oster-owned; Audio Modulation adopt; MetaSounds pilot/selective adoption; exact sound library remains content work;
- **item 24** grenades: Niagara stays production VFX owner; discrete Montage/sequence for hand/throw/recover; Chaos physics stays authoritative base;
- **item 27** vegetation: replace rejected family using verified UE 5.8 assets + foliage/PCG/instancing after material/LOD proof;
- **items 28–29** HMMWV/M2: Chaos Vehicles pilot/migration for driving; M2 turret remains Oster-owned shared turret layer;
- **items 30–31** BTR: do not force wheeled Chaos migration until BTR-specific movement/physics requirements are separately proven; retain Oster crew/turret/remote optic gameplay;
- **item 32** world fidelity: PCG only for generic repeated content, HLOD pilot for distant content, CC0/licensed production assets for generic materials/props;
- **item 33** performance: every newly adopted subsystem must survive the existing fullscreen/~60 FPS/thermal acceptance route;
- **item 34** tactical map: no external framework replacement justified; keep Oster geo/reference ownership;
- **item 35** full runtime test: add replacement-specific assertions only after each migration actually lands;
- **item 36** merge: unchanged, no merge without factual current-head UE 5.8 acceptance.

## 15. Migration order

Do not attempt all replacements simultaneously.

Recommended order after/alongside current PASS45 blocking work:

1. finish current item 16 content/runtime boundary;
2. create Chaos Vehicles isolated proof car;
3. replace prototype destruction cube chunks with one Geometry Collection proof;
4. add Audio Modulation buses while preserving current event API;
5. pilot one MetaSound system (vehicle engine is preferred because RPM is naturally parameter-driven);
6. pilot one Motion Matching third-person character without touching weapon gameplay authority;
7. add IK/retarget/Control Rig pipeline for weapons/actions;
8. migrate AI high-level decision slice to BT/EQS only after behavior parity tests exist;
9. replace rejected vegetation family with verified UE 5.8 content;
10. only then consider Soundscape, HLOD/PCG expansion, Smart Objects and future drone work.

This order limits simultaneous architecture churn and makes regressions attributable.

## 16. Non-negotiable deletion gate

For every accepted migration the PR must prove both sides:

**new owner present + old duplicate absent.**

A migration is incomplete if the new system works but obsolete runtime code still exists and can regain authority through a fallback, delayed subsystem, config path or old verifier.

The final implementation pattern is:

`accepted replacement -> production authority switched -> obsolete runtime owner physically deleted -> obsolete tests/config/data deleted -> clean build -> source regression -> local UE 5.8 runtime -> multiplayer/performance acceptance`

Git history is the rollback mechanism.

## 17. Audit verdict

Strong immediate/near-term recommendations:

- Chaos Vehicles: **pilot, then adopt for wheeled vehicle physics if accepted**;
- Chaos Physics: **standardize/keep**;
- Chaos Destruction: **selectively adopt**;
- Niagara: **standardize/expand**;
- Audio Modulation: **adopt**;
- MetaSounds: **pilot/selective adopt** due UE 5.8 Beta warning;
- Motion Matching/Game Animation Sample: **pilot for locomotion presentation**;
- IK Rig/Retargeter + Control Rig + Motion Warping/Montages: **adopt selectively**;
- AI Perception/Navigation: **keep**;
- Behavior Trees/EQS: **pilot/adopt for high-level/spatial bot decisions**;
- PCG/HLOD: **selective pilot/adopt**;
- Poly Haven CC0: **approved generic content source class**;
- Kenney CC0: **approved utility/generic source class when visual style fits**;
- CARLA: **reference only**;
- ALS Community: **reference/backup only**;
- ALS Refactored: **backup pilot if Epic locomotion path fails**;
- PBCharacterMovement: **do not adopt as default Oster movement**;
- Soundscape: **Beta pilot only**;
- Chaos Modular Vehicles: **Experimental pilot only**;
- Project AirSim: **future drone pilot/reference only**;
- OpenTournament: **do not import**.

This audit changes the implementation strategy, not runtime truth. **PASS45 remains RUNTIME REJECTED until current-head local UE 5.8 acceptance proves otherwise. PR #94 remains OPEN / UNMERGED.**
