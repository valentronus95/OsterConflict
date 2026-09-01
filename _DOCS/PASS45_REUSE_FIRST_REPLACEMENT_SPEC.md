# PASS45 REUSE-FIRST SYSTEM REPLACEMENT SPEC

Date: 2026-09-01
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
Scope: Oster Conflict / Unreal Engine 5.8.x / Windows / multiplayer
Status: NORMATIVE SUBORDINATE SPEC ONCE BOUND BY THE PARENT TZ

## 1. Purpose

Oster Conflict must not continue to reimplement mature engine/game systems from zero when Unreal Engine 5.8 or a legally reusable, technically compatible implementation already covers the same responsibility better.

This specification defines a **reuse-first, replace-not-stack** migration policy for still-open PASS45 systems and later stabilization work.

The objective is not to import another complete game into Oster Conflict. The objective is to replace weak, duplicated or prototype-grade owners with proven engine-native or legally reusable components while preserving Oster-specific gameplay, content identity and multiplayer rules.

## 2. Non-negotiable replacement rule

For every migrated responsibility:

`audit current owner -> prove replacement in isolation -> integrate replacement -> runtime/network/performance acceptance -> switch authority -> physically delete obsolete owner -> delete stale tests/config/fallbacks -> regression acceptance`

Rules:

1. **No permanent dual ownership.** Two vehicle physics owners, two locomotion owners, two damage owners, two audio timing owners or two VFX gameplay owners are forbidden.
2. **Old code is deleted after successful replacement acceptance.** It is not kept as an active hidden fallback "just in case".
3. Rollback safety comes from Git history and a known-good commit, not from shipping two competing runtime systems.
4. During the proof phase the old and new systems may coexist only behind an explicit isolated migration switch/test actor. They may not both mutate the same production actor in the same run.
5. Once the replacement passes the defined gate, delete:
   - obsolete C++/Blueprint runtime owner;
   - obsolete components/subsystems/timers;
   - obsolete config and data fields;
   - obsolete content generated only for the old owner;
   - stale source verifiers and workflows that require the retired owner;
   - compatibility shims whose only purpose was to keep the old owner alive.
6. A replacement is not accepted because it compiles. It must pass local UE 5.8 gameplay, multiplayer behavior where relevant, visual/audio acceptance and performance regression checks.
7. Oster-specific rules stay Oster-owned. Reusing engine code does not mean surrendering game design authority.

## 3. Source priority

Use sources in this order unless a documented technical reason requires otherwise:

1. **Stable Unreal Engine 5.8 systems** already shipped with the engine.
2. **Epic sample systems/examples** intended to be migrated/adapted into Unreal Engine projects, subject to the applicable Unreal/Epic license.
3. **Permissively licensed open-source code** such as MIT/BSD/Apache, pinned to a reviewed version/commit.
4. **Licensed production assets** from Fab or other providers only after license/provenance review.
5. Custom Oster implementation only when the required behavior is genuinely project-specific or existing solutions fail the acceptance requirements.

Experimental/Beta engine plugins are never promoted directly to production ownership. They require an isolated proof, packaged build test and explicit runtime/performance acceptance first.

## 4. Mandatory third-party provenance register

Before importing or copying any external code/content, record it in `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` with:

- system/asset name;
- source/publisher;
- exact URL or repository;
- exact version/tag/commit;
- license;
- date acquired;
- exact files/modules/assets imported;
- local modifications;
- attribution requirements;
- redistribution restrictions;
- whether source may be stored in a public repository;
- whether the item is production-approved, reference-only or rejected.

Unknown/ambiguous license = **DO NOT IMPORT**.

## 5. Vehicle physics replacement — production baseline

### 5.1 Target

Use **Chaos Vehicles** as the production baseline for ordinary wheeled vehicle physics.

Responsibilities to migrate to Chaos Vehicles where the current Oster code duplicates them:

- wheel contact and suspension;
- chassis rigid-body simulation;
- engine torque curve;
- forward/reverse gears;
- transmission behavior;
- wheel friction/slip;
- braking and handbrake;
- steering response;
- mass and center of mass;
- road/off-road surface response;
- gravity/collision;
- deterministic/asynchronous physics configuration where appropriate.

### 5.2 Old code deletion

After one civilian vehicle and HMMWV pass the migration gate, physically remove custom Oster implementations that independently calculate the same wheel/suspension/engine/transmission/chassis forces.

Do not retain a second custom force-injection model as an invisible fallback.

### 5.3 Oster-owned vehicle behavior that remains

Keep Oster-specific logic as a thin gameplay layer around the accepted vehicle physics owner:

- enter/exit and seat ownership;
- driver/gunner/passenger seats;
- HMMWV M2 station;
- BTR crew/station rules;
- damage state and game-specific disable/destroy logic;
- HUD;
- cameras;
- interaction prompts;
- weapon/turret integration;
- team/permission logic;
- spawn/deployment rules;
- gameplay speed/handling tuning values.

### 5.4 CARLA policy

CARLA may be used as an **MIT-licensed reference implementation and donor only after file-level review** for vehicle setup ideas, control/tuning patterns and simulation architecture.

Do not import CARLA wholesale. Do not add CARLA server/autonomy/sensor infrastructure as an Oster runtime dependency unless a later dedicated decision proves a concrete need.

### 5.5 Chaos Modular Vehicles policy

UE 5.8 **Chaos Modular Vehicles is Experimental**. It is not the default production owner.

Use it only in an isolated research spike for needs that stable Chaos Vehicles does not cover well, such as modular detachable vehicle structures or integrated Geometry Collection damage.

Promotion requires:

- packaged UE 5.8 build;
- server-authoritative multiplayer test;
- client prediction/rewind test;
- 10-minute HMMWV/BTR soak;
- no worse handling/latency/performance than stable Chaos Vehicles;
- explicit parent-TZ status change.

## 6. Turrets and mounted weapons

Do not tie M2/BTR turret mechanics to a one-off vehicle class.

Create/retain one Oster-owned turret gameplay layer using engine transform/physics primitives:

`vehicle mount -> yaw assembly -> elevation assembly -> weapon -> gunner camera/optic`

Shared responsibilities:

- yaw/elevation limits;
- authored pivot/socket ownership;
- input mapping;
- multiplayer replication;
- gunner camera;
- firing bridge;
- recoil presentation;
- seat ownership.

HMMWV M2 and BTR remote turret use the same architectural contract but may have different data profiles.

Delete vehicle-specific duplicate turret controllers after the shared replacement is accepted.

## 7. General physics replacement

Use **Chaos Physics** as the authoritative base for ordinary physical simulation instead of custom per-object physics loops.

Migrate/standardize where applicable:

- dropped weapons;
- grenades;
- loose props;
- doors/gates with physical motion where appropriate;
- vehicle chassis;
- debris;
- ragdolls;
- physical hit reactions;
- breakable pieces;
- impulse/force response.

Gameplay authority remains server-owned. Cosmetic physics may be client-side only when it cannot affect damage, inventory, collision authority or game state.

Delete custom gravity, bounce, settle or force integrators that duplicate accepted Chaos behavior unless they implement a documented game-specific mechanic unavailable from Chaos.

## 8. Character movement and locomotion

### 8.1 Do not replace blindly

The current FPS movement owner must first be audited against:

- native `UCharacterMovementComponent` / Enhanced Input;
- Epic **Game Animation Sample** and UE 5.8 Motion Matching architecture;
- **ALS Community** as an MIT-licensed locomotion/reference candidate;
- **PBCharacterMovement** as an MIT/open-source FPS movement candidate only if its movement goals match Oster and it cleanly recompiles/validates on UE 5.8.

PBCharacterMovement must not be imported merely because it exists; its published binaries target an earlier UE version and its HL2-style movement may be wrong for Oster's intended Battlefield-like handling.

### 8.2 Preferred architecture

Keep gameplay movement authority simple and deterministic. Use animation systems to present movement rather than allowing animation code to become a second movement owner.

Preferred split:

`CharacterMovement authoritative motion -> movement state/tags -> Motion Matching / animation selection -> IK/warping -> visual pose`

### 8.3 Replace/delete rule

If Game Animation Sample/Motion Matching or another accepted candidate replaces current locomotion presentation, physically delete the superseded locomotion state-machine/procedural presentation owner and stale transitions after parity acceptance.

Do not run ALS + Motion Matching + an Oster locomotion state machine simultaneously as three competing pose owners.

## 9. Animation stack

Prefer engine/sample solutions over handcrafted procedural whole-body/whole-weapon motion when production animation data can own the presentation.

Candidate stack:

- Game Animation Sample patterns/assets where license permits;
- Motion Matching / Pose Search for third-person locomotion;
- IK Rig / IK Retargeter for skeleton migration;
- Control Rig for authored procedural adjustment, not gameplay authority;
- Motion Warping for contextual traversal/alignment;
- Animation Montages for discrete actions;
- physical animation/ragdoll blending for hit/fall/death reactions.

Required action coverage to audit:

- idle/walk/run/sprint;
- crouch;
- jump/fall/land;
- turn/start/stop;
- vault/mantle only if retained in design;
- weapon equip/unequip;
- ADS transitions;
- reloads by weapon family;
- M700 bolt;
- Remington 870 pump;
- Lever Action cycle;
- grenade prepare/throw/recover;
- mounted-weapon enter/exit/use;
- vehicle enter/exit;
- hit reactions;
- death/ragdoll transitions.

First-person weapon/arms animation remains weapon-specific and must not be replaced by a generic third-person locomotion sample.

## 10. Explosions, smoke, fire and visual effects

### 10.1 Production VFX owner

Use **Niagara** as the default production VFX system for:

- muzzle flashes;
- tracers where appropriate;
- bullet impact particles;
- dust/dirt/concrete/metal/wood impacts;
- frag explosions;
- flash grenade world flash/sparks;
- smoke grenades;
- vehicle fire/smoke;
- explosion debris visuals;
- ambient dust/leaves/insects where useful;
- environmental smoke/fire.

Retire visible BasicShape/procedural proxy VFX once equivalent Niagara content is accepted.

### 10.2 Gameplay/VFX separation

Explosion gameplay must follow:

`server authoritative detonation -> damage/impulse/occlusion/state result -> replicated presentation event -> Niagara + decal + audio + camera response`

Niagara must never be the damage authority.

### 10.3 Destruction

Use **Chaos Destruction / Geometry Collections / Physics Fields** for selected destructible content instead of bespoke fractured-prop logic.

Do not make every building destructible. Establish destruction classes and budgets:

- small props: fully physical/destructible where useful;
- doors/fences/windows: targeted break states;
- vehicle detachable/damage parts: controlled and replicated only where gameplay-relevant;
- landmark/core architecture: destructible only if explicitly required by game design and performance budget.

Network only gameplay-relevant destruction state. Pure debris should not flood replication.

## 11. Complete audio replacement plan

### 11.1 Runtime audio technology

Use **MetaSounds** as the preferred data-driven/procedural audio presentation layer where it materially reduces duplicated SoundCue/C++ logic.

Use **Audio Modulation** for global/stateful mix control such as:

- master/SFX/music/voice buses;
- indoor/outdoor mix changes;
- suppression/low-health/flash effects;
- vehicle interior/exterior transitions;
- temporary ducking;
- distance/intensity-driven mix parameters.

**Soundscape** may be evaluated for environmental ambience, but UE 5.8 documents it as Beta. It is not production authority until a packaged performance/streaming test passes.

### 11.2 Weapon audio library requirements

Every accepted weapon profile must cover as applicable:

- near outdoor shot;
- indoor shot;
- distant tail;
- mechanical tail;
- suppressed shot if supported;
- dry fire;
- magazine out/in;
- charging handle/bolt release;
- bolt cycle;
- pump cycle;
- lever cycle;
- selector switch;
- equip/handling;
- shell/casing impact where audible.

Generic temporary fallback may exist only during migration and must be removed from final accepted weapon profiles.

### 11.3 Ballistic/environment interaction audio

Add data-driven surface families using Physical Materials:

- dirt/soil;
- grass;
- asphalt;
- concrete;
- brick;
- wood;
- metal;
- glass;
- water where applicable.

For each relevant family support:

- projectile impact;
- ricochet where physically plausible;
- debris layer;
- footsteps;
- vehicle tyre/track contact where applicable.

Add bullet crack/whiz/supersonic pass presentation only from factual projectile/trajectory events. It must not fake extra shots.

### 11.4 Explosion audio

Explosion profiles should layer:

- initial transient;
- body/low-frequency report;
- distance tail;
- debris/secondary impacts;
- environment/indoor reflection variant where supported;
- optional short temporary hearing effect at unsafe proximity.

Damage remains gameplay-owned; hearing effects are presentation/state consequences, not a second damage system.

### 11.5 Vehicle audio

Each production vehicle should support data-driven layers for:

- engine start/stop;
- idle;
- RPM/load;
- transmission/gear change;
- tyre/road or tyre/dirt response;
- suspension/body impacts;
- braking/skid;
- horn if retained;
- interior/exterior filtering;
- mounted-weapon sound routing;
- damage/fire state.

### 11.6 Character/equipment audio

Audit/add:

- footsteps by surface;
- crouch/landing/jump impacts;
- clothing/gear movement at restrained levels;
- weapon handling;
- grenade handling;
- door/gate interactions;
- vehicle enter/exit;
- hit/death reactions if design uses them;
- radio/voice only under a separate voice design policy.

### 11.7 Environmental ambience

Create biome/location ambience profiles rather than looping one generic background file everywhere.

Candidate Oster layers:

- wind by exposed/sheltered area;
- deciduous/conifer foliage response;
- birds by time/location;
- insects where seasonally appropriate;
- distant dogs;
- distant civilian/road noise where design allows;
- electrical hum/transformer/utility locations;
- interior room tone;
- park/stadium/town-center variations;
- weather layers;
- fire/burning area layers;
- distant combat layers only when actually intended by game state.

Ambient systems must respect attenuation, occlusion/reverb strategy and performance voice limits.

## 12. Models, materials and environment content

### 12.1 Reuse before procedural placeholder

For non-unique world props, prefer production-ready licensed assets rather than constructing approximate cubes/cylinders in C++.

Acceptable source classes include:

- already tracked Oster production assets with verified provenance;
- Epic/Fab content with verified project-use rights;
- **Poly Haven CC0** models/textures/HDRIs;
- other CC0/permissive assets after provenance review.

### 12.2 Unique Oster landmarks remain reference-owned

Museum, Silpo, Culture House, water tower, stadium and other photo-bound Oster landmarks must remain reference-driven. Generic asset packs may provide reusable doors, windows, bricks, props, foliage or materials, but may not overwrite the factual landmark shape/site identity.

### 12.3 Materials

Prefer reusable material functions/master materials for:

- brick;
- plaster;
- concrete;
- asphalt;
- soil;
- grass;
- metal;
- painted metal;
- glass;
- wood;
- wetness/dirt variations.

Do not create one bespoke runtime material system per landmark when a shared material layer can cover the same physical surface without losing reference identity.

## 13. World building and optimization

Audit migration toward engine-native world systems:

- **World Partition** for spatial streaming if the current map structure benefits from it;
- **HLOD** for distant non-interactive world representation and draw-call reduction;
- **PCG** for repeatable non-identity-critical vegetation/ground clutter/prop distribution;
- instancing/HISM for repeated props where appropriate;
- Nanite where content/material/performance compatibility is proven;
- Data Layers for controllable world variants if needed.

PCG must not generate or relocate evidence-bound landmarks.

Vegetation replacement item 27 should prefer a verified UE 5.8-compatible foliage family and PCG/foliage placement pipeline rather than repairing a rejected KiteDemo family forever if replacement is cheaper and more reliable.

## 14. AI and bots

Do not build an entirely custom decision framework unless required.

Preferred stable UE building blocks:

- Navigation System;
- Behavior Trees;
- AI Perception;
- EQS for cover/line-of-sight/position selection;
- Smart Objects for reservable world interactions.

StateTree may be evaluated for specific state-heavy behavior if it reduces complexity.

MassAI is Experimental in UE 5.8 and is not the production baseline for combat bots without a separate scale/performance proof.

Oster-specific AI remains responsible for tactical doctrine, teams, objectives, weapon use, vehicle use and Battlefield-like squad behavior.

## 15. Multiplayer/session architecture

Prefer native Unreal replication/network physics and selectively reuse **Lyra** patterns/plugins where they clearly replace weaker custom infrastructure.

Candidates to audit rather than blindly copy:

- team/player state patterns;
- spawn/respawn flow;
- sessions/login via Common User where needed;
- Gameplay Tags/data-driven state;
- modular UI/input patterns;
- Gameplay Ability System only where it clearly simplifies damage/status/action orchestration.

Do not replace a stable Oster gameplay system with the entire Lyra architecture merely for consistency. Migration must show a concrete reduction in code/bugs/duplication.

## 16. Input and UI

Use **Enhanced Input** as the default input mapping system for contextual player/vehicle/turret controls unless an existing accepted owner already uses it correctly.

Audit **CommonUI**/Lyra patterns for menus, settings, controller/keyboard prompts and screen-layer management before maintaining custom duplicate UI navigation infrastructure.

## 17. Damage, hit reaction and ragdoll

Audit current custom code against:

- Chaos rigid-body physics;
- Physical Animation;
- Physics Assets;
- Gameplay Tags/GAS only where useful for state orchestration;
- Niagara/MetaSound presentation events.

Target split:

`authoritative hit/damage -> character/vehicle state -> replicated presentation event -> hit reaction / physical animation / VFX / audio`

Presentation must not calculate a second independent damage result.

## 18. Doors, gates and world interaction

Standardize interactions around one data-driven interface/component rather than per-object scripts.

Use:

- Enhanced Input interaction action;
- Gameplay Tags/data assets for interaction type;
- native component/physics behavior;
- Smart Objects only where reservation/concurrent-use semantics help AI and players.

Delete duplicated one-off door/gate interaction owners after migration.

## 19. Drones — future candidate, not current PASS45 blocker

For future FPV/drone work, review **AirSim / Project AirSim lineage** as an MIT-licensed reference/candidate for flight/sensor concepts.

Do not make the archived/legacy simulator a mandatory runtime dependency. Extract only proven useful concepts/modules after UE 5.8 compatibility review.

Drone gameplay, camera, damage, radio/latency rules and player controls remain Oster-specific.

## 20. Asset/code intake policy

Every new external dependency must answer before merge:

1. What exact Oster owner does this replace?
2. Is the current owner actually defective/duplicated enough to justify migration?
3. Is the candidate stable on UE 5.8?
4. Is it Stable/Beta/Experimental?
5. What license applies?
6. Can the source/content legally live in this repository?
7. What is the exact pinned version/commit?
8. What runtime/network/performance gate proves the replacement?
9. Which old files/classes/assets/tests will be deleted after cutover?
10. How is rollback performed from Git without keeping a live dual owner?

No answers = no dependency.

## 21. Migration acceptance template

For each replacement create a small migration record containing:

- `OLD_OWNER`;
- `NEW_OWNER`;
- `WHY_REPLACE`;
- `FILES_TO_DELETE_AFTER_ACCEPTANCE`;
- `FILES_TO_KEEP_AS_OSTER_ADAPTER`;
- `LICENSE/PROVENANCE`;
- `UE58_BUILD_GATE`;
- `RUNTIME_GATE`;
- `MULTIPLAYER_GATE`;
- `VISUAL/AUDIO_GATE`;
- `PERFORMANCE_GATE`;
- `CUTOVER_COMMIT`;
- `OLD_OWNER_REMOVAL_COMMIT`;
- `FINAL_STATUS`.

A migration cannot be called complete while `OLD_OWNER` still mutates the same production responsibility.

## 22. PASS45 mapping

This replacement spec does **not** reopen already source-closed checklist items 1-26 merely to refactor them.

It changes the implementation strategy for still-open work and any directly related defect discovered during acceptance:

- item 16: prefer authored animation/sample/retargeting pipeline over another procedural manual-action fallback; old procedural owner stays deleted;
- item 18: use factual sockets/IK/retargeting/calibration tools rather than generic ADS offsets;
- item 20: migrate final sound presentation toward MetaSound/data-driven exact profiles; generic fallback removed after acceptance;
- item 24: Niagara + authored animation pipeline for grenade VFX/throw presentation;
- item 27: replace rejected vegetation content/pipeline instead of endlessly patching the rejected tree family if a verified replacement is cheaper and safer;
- items 28-31: vehicle/turret/BTR acceptance must audit/migrate duplicate custom vehicle physics toward Chaos Vehicles while preserving Oster-specific seat/turret/camera/gameplay logic;
- item 32: World Partition/HLOD/PCG/material reuse may replace prototype world-generation owners where evidence and performance justify it;
- item 33: replacement is invalid if it loses the target display/FPS/thermal budget;
- item 35: full runtime acceptance must exercise the post-cutover owners, not stale legacy fallback paths;
- item 36: PR #94 remains unmerged until factual current-head runtime acceptance.

## 23. Immediate execution priority

Do not pause the current first genuinely open PASS45 blocker merely to perform a project-wide rewrite.

When work touches a still-open domain, apply this reuse-first rule immediately. Larger migrations are performed in bounded slices, each with an isolated proof and physical deletion of the superseded owner after acceptance.

The first high-value replacement audits are:

1. vehicle physics owner inventory -> Chaos Vehicles migration plan for civilian car/HMMWV, then BTR-specific suitability decision;
2. full audio-owner inventory -> MetaSound/Audio Modulation replacement map;
3. character locomotion/animation owner inventory -> Game Animation Sample/Motion Matching comparison;
4. explosion/VFX owner inventory -> Niagara + Chaos Destruction replacement map;
5. world/vegetation owner inventory -> verified foliage + PCG/HLOD/World Partition suitability map;
6. AI/bot owner inventory -> native Behavior Tree/EQS/Perception/Smart Object reuse map.

## 24. Definition of done

This reuse-first initiative is successful when Oster Conflict has **fewer runtime owners and less custom infrastructure**, not merely more plugins.

Success means:

- duplicated custom systems are physically gone;
- selected engine/open-source replacements are pinned and provenance-recorded;
- Oster-specific adapters are small and explicit;
- no active dual ownership remains;
- packaged UE 5.8 runtime passes;
- multiplayer state remains coherent;
- direct visual/audio acceptance passes;
- performance remains inside the defined budget;
- rollback is available through Git history rather than hidden legacy runtime code.
