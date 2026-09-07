# PASS45 REUSE-FIRST SYSTEM REPLACEMENT SPEC

Date: 2026-09-01
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
Detailed audit: `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md`
Third-party register: `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md`
Scope: Oster Conflict / Unreal Engine 5.8.x / Windows / multiplayer
Status: **NORMATIVE SUBORDINATE SPEC BOUND BY THE PARENT TZ**

## 1. Purpose

Oster Conflict must not continue to reimplement mature engine/game systems from zero when Unreal Engine 5.8, an Epic sample intended for Unreal projects, or a legally reusable and technically compatible implementation already covers the same responsibility better.

This specification is **reuse-first, replace-not-stack**. It does not authorize importing another complete game. It authorizes bounded replacement of weak, duplicated or prototype-grade owners while preserving Oster-specific gameplay, map identity, multiplayer rules and runtime acceptance gates.

The detailed technical/licensing analysis behind the decisions below is recorded in `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md`. That audit is evidence; this file is the normative implementation decision layer bound into `PASS45_RUNTIME_RECOVERY_TZ.md`.

## 2. Non-negotiable replacement doctrine

For every migrated responsibility:

`audit current owner -> prove replacement in isolation -> integrate replacement -> runtime/network/performance acceptance -> switch authority -> physically delete obsolete owner -> delete stale tests/config/fallbacks -> regression acceptance`

Rules:

1. **One responsibility = one production mutating owner.** Permanent dual vehicle physics, locomotion, AI decision, destruction, audio-timing or VFX gameplay owners are forbidden.
2. **Old duplicate code is physically deleted after the replacement passes its gate.** It is not retained as a hidden runtime fallback.
3. Rollback safety comes from Git history and a known-good commit, not from shipping two competing systems.
4. Old and new systems may coexist only inside an explicit isolated proof/migration path and may not both mutate the same production actor in the same run.
5. After cutover remove obsolete C++/Blueprint owners, components, timers, config/data fields, compatibility shims, generated prototype content, and stale tests/verifiers/workflows that require the retired owner.
6. Compile/source green is not acceptance. Relevant replacements require local UE 5.8 packaged/gameplay evidence, multiplayer evidence where applicable, visual/audio acceptance and performance regression checks.
7. Engine-native capability is preferred when it removes custom code without reducing control or quality.
8. Experimental/Beta systems are **pilot-only** until separately accepted. They are never silently promoted because they are shipped by Epic.
9. Unknown or ambiguous license/provenance = **DO NOT IMPORT**.
10. Oster-specific rules remain Oster-owned adapters around the accepted reusable system.

## 3. Audited 2026 decision matrix

| Area | Audited decision | Production rule |
|---|---|---|
| Enhanced Input | **KEEP** | Already correct engine-native input owner. Do not add a second gameplay-input system. |
| Chaos rigid-body physics | **STANDARDIZE / KEEP** | Use for ordinary rigid bodies, drops, grenades, ragdolls, debris and physical response; delete only custom duplicate integrators. |
| Chaos Vehicles | **REQUIRED PILOT -> ADOPT IF ACCEPTED** | Candidate to replace custom wheeled suspension/drive/grip/brake/steering solver. UE 5.8 plugin is still marked Experimental, so production cutover requires packaged/network/performance proof first. |
| Chaos Modular Vehicles | **PILOT ONLY** | Experimental research candidate for modular/detachable vehicle structures; not current production owner. |
| CARLA | **REFERENCE ONLY** | MIT code may inform vehicle setup/tuning after file-level review. Do not import CARLA wholesale or add its autonomy/sensor/server stack as runtime dependency. |
| Chaos Destruction / Geometry Collections / Physics Fields | **SELECTIVE ADOPT** | Replace prototype cube-fragment destruction on selected props/windows/fences/parts; do not make all Oster destructible. |
| Niagara | **STANDARDIZE / EXPAND** | Default presentation owner for explosions, smoke, fire, muzzle, impact and environmental VFX. Never gameplay damage authority. |
| Audio Modulation | **ADOPT** | Preferred global/stateful mix buses and dynamic audio-state layer. |
| MetaSounds | **PILOT -> SELECTIVE ADOPT** | Strong candidate for engine/RPM, layered explosions, ambience and some weapon rendering. UE 5.8 docs still carry Beta caution, so no blanket rewrite. |
| Soundscape | **PILOT ONLY** | Beta ambience candidate. Must beat current custom ambient-zone polling in packaged performance/voice tests before cutover. |
| Native CharacterMovement | **KEEP** | Current authoritative gameplay movement remains unless measured defects justify a separate migration. |
| Game Animation Sample / Motion Matching / Pose Search | **PILOT** | Candidate for third-person locomotion presentation, not gameplay movement authority and not weapon/grenade action ownership. |
| IK Rig / IK Retargeter | **ADOPT** | Reuse/retarget animation packs and improve skeleton/weapon compatibility. |
| Control Rig | **SELECTIVE ADOPT** | Authoring/correction layer for hands, feet, weapon parts and animation cleanup; not gameplay authority. |
| Motion Warping | **SELECTIVE ADOPT** | Context alignment for vehicle/M2 enter-exit and similar authored interactions. |
| Animation Montages / authored sequences | **ADOPT AS ACTION PRESENTATION** | Preferred for reloads, M700 bolt, Remington pump, Lever cycle, grenade throw and other discrete actions. |
| ALS Community | **REFERENCE / BACKUP ONLY** | MIT but current public compatibility trails UE 5.8; not preferred over Epic's current animation path. |
| ALS Refactored | **BACKUP PILOT** | MIT and multiplayer-focused; use only if Epic locomotion pilot fails and UE 5.8 build/runtime proof passes. |
| PBCharacterMovement | **REJECT AS DEFAULT OWNER** | MIT but intentionally HL2-style movement and older published binaries; reference only. |
| AI Perception + Navigation | **KEEP** | Already used by Oster and appropriate. |
| Behavior Trees + Blackboard | **PILOT / LIKELY ADOPT FOR HIGH-LEVEL AI** | Candidate to replace growing custom high-level decision loop after behavior parity tests. |
| EQS | **SELECTIVE ADOPT** | Use for cover/position/revive/resupply/spatial scoring, not trivial deterministic checks. |
| Smart Objects | **PILOT** | Candidate for seats, mounted weapons, doors, resupply and contextual interactions. |
| Mass AI | **DEFER** | Investigate only if bot-count profiling proves Actor/AIController architecture is a bottleneck. |
| Lyra | **REFERENCE / SELECTIVE MIGRATION ONLY** | Use patterns or bounded plugins where they solve a measured problem. Do not replace Oster wholesale. |
| Common User | **POST-PASS45 PILOT** | Candidate only if session/auth/login needs exceed current implementation. |
| CommonUI + Enhanced Input integration | **DO NOT MIGRATE DURING PASS45** | UE 5.8 integration path still has experimental caution; keep current UMG/Enhanced Input. |
| PCG | **SELECTIVE ADOPT** | Generic foliage/clutter/repeated props only; never evidence-bound landmark identity. |
| HLOD | **SELECTIVE PILOT** | Use where profiler/map audit proves distant static-content benefit. |
| World Partition conversion | **DEFER** | Do not convert the whole map during active PASS45 without measured necessity. |
| Poly Haven | **APPROVED GENERIC CONTENT SOURCE CLASS** | CC0; per-asset quality/provenance record still required. |
| Kenney | **APPROVED UTILITY/GENERIC SOURCE CLASS** | CC0, but many assets are stylized; use only where visual style fits or for non-final utility content. |
| Fab assets | **LICENSED SOURCE, NOT OPEN SOURCE** | Per-asset license/entitlement required; no standalone redistribution. |
| Freesound | **PER-FILE AUDIO SOURCE ONLY** | CC0 preferred; CC-BY requires attribution; BY-NC/restrictive/unknown licenses are rejected for unrestricted production. |
| Legacy Microsoft AirSim | **REFERENCE ONLY** | Archived/legacy; not current PASS45 dependency. |
| IAMAI Project AirSim | **FUTURE DRONE PILOT / REFERENCE** | MIT, but reviewed engine support trails UE 5.8; isolate and prove before any future adoption. |
| OpenTournament | **DO NOT IMPORT** | Public source is not automatically freely reusable; project legal terms/content restrictions make it reference-only. |

## 4. Vehicle physics migration — exact rule

### 4.1 Current duplication to eliminate

Current `AOCVehicleBase` manually owns generic wheeled-vehicle behavior including line-trace suspension, spring/damping forces, drive force, lateral grip, rolling braking, handbrake force, steering torque, max-speed force limiting and custom wheel animation.

This generic solver must **not** be expanded indefinitely.

### 4.2 Required Chaos Vehicles proof

Before production cutover create one isolated disposable UE 5.8 Chaos Vehicles proof car and verify:

- packaged Windows build;
- server + at least one client;
- forward/reverse/steer/brake/handbrake;
- curb/slope response;
- asphalt/dirt/grass/gravel response where Physical Materials exist;
- stable possession/exit with no Museum teleport;
- no unacceptable replication correction/jitter;
- CPU/frame cost equal or better than current custom solver;
- predictable tuning for an Oster civilian vehicle and HMMWV mass/speed targets.

If this fails, do not delete current production vehicle solver merely to satisfy the architectural preference. Record failure and reassess.

### 4.3 Deletion after accepted cutover

After the pilot and first production vehicle pass, physically remove the equivalent custom solver ownership, including where no longer needed:

- `SimulateVehicleServer()` custom solver path;
- `ApplySuspensionServer()`;
- `ApplyDriveAndGripServer()`;
- custom `SuspensionPointsLocal` ownership;
- duplicate spring/damping/drive/grip/rolling-brake/handbrake/steering-torque tuning fields;
- custom visual wheel spin/steer owner if the accepted Chaos animation path replaces it;
- obsolete verifier expectations/config built only around the removed force solver.

Keep Oster-owned:

- seats and enter/exit;
- driver/gunner/passenger permissions;
- HMMWV M2 gameplay;
- BTR crew/turret/remote optic rules;
- health/damage gameplay state;
- cameras/HUD/prompts;
- team/spawn/deployment rules;
- audio/VFX event hooks;
- target gameplay speed/handling profile values.

BTR is **not automatically forced** onto the same wheeled migration until its movement/physics needs are separately proven.

## 5. Destruction / explosion replacement

Current `AOCDestructibleProp` prototype presentation creates random Engine Cube BasicShape chunks after destruction. That is not acceptable production destruction.

For selected destructible families:

`server damage/state -> replicated intact/destroyed gameplay state -> Geometry Collection/Chaos fractured presentation -> Niagara debris/dust/fire where appropriate -> audio`

After wood/metal/masonry proof sets pass:

- remove BasicShape cube-fragment generation from production paths;
- remove obsolete transient chunk arrays/timers if Geometry Collection owns the presentation;
- replicate only gameplay-relevant destruction state/collision;
- keep cosmetic debris local where it cannot affect authoritative gameplay;
- apply destruction budgets so landmark/core-city architecture is not accidentally converted into full Battlefield-scale destruction without design approval.

## 6. VFX policy

Niagara is the standard presentation owner for:

- weapon muzzle flashes;
- tracers where appropriate;
- bullet impacts by surface;
- frag explosions;
- flash grenade world presentation;
- smoke grenade;
- vehicle damage smoke/fire;
- dirt/concrete/wood/metal/glass debris particles;
- ambient dust/leaves/insects when justified.

No Niagara emitter may become damage/inventory/fuse authority.

Correct event chain:

`authoritative gameplay event -> replicated presentation event -> Niagara + decal + audio + camera/hearing response`

Visible BasicShape/procedural VFX fallbacks are physically removed after accepted Niagara replacements exist.

## 7. Audio architecture and full sound inventory

### 7.1 Ownership split

Keep factual gameplay event authority in Oster (`shot accepted`, `manual action`, `grenade detonated`, `vehicle state`, etc.). MetaSounds/Audio Modulation render and mix those events; they do not decide whether gameplay occurred.

### 7.2 Audio Modulation migration

Adopt buses/mixes for at minimum:

- Master;
- Weapons/SFX;
- Vehicles;
- Ambience;
- UI;
- Music;
- Voice when added;
- indoor/outdoor state;
- vehicle interior/exterior state;
- flash/concussion/temporary hearing state;
- menu/gameplay transitions.

`UOCAudioUserSettings` may remain the preference authority but should drive modulation state instead of requiring every playback call to own separate long-term mix logic.

### 7.3 MetaSounds pilot

Preferred first pilot: **vehicle engine RPM/load** because it naturally consumes continuous gameplay parameters.

Second suitable pilots: layered explosion and ambient wind/bed.

Do not migrate every weapon simultaneously. Promotion requires fresh-load/package audibility, no event double-counting, acceptable voice/concurrency cost and no audio-thread/frame regression.

### 7.4 Soundscape pilot

Compare one central-Oster/park area against current `AOCAmbientAudioZone` polling/random timer implementation using:

- birds;
- wind/leaves;
- dogs;
- distant traffic;
- localized utility hum;
- voice count;
- audio-thread/CPU cost;
- streaming behavior.

Only if the pilot is better should overlapping ambient-zone polling/scheduling code be retired for that responsibility.

### 7.5 Required production sound families

Weapons:

- near outdoor shot;
- indoor shot;
- distant tail;
- suppressor where supported;
- dry fire;
- magazine out/in;
- charging handle/bolt release;
- M700 bolt;
- Remington pump;
- Lever Action cycle;
- selector;
- equip/handling;
- casing/secondary mechanics where useful.

Ballistics/environment:

- impacts by Physical Material;
- ricochet where physically plausible;
- factual projectile crack/whiz;
- debris secondary impacts.

Vehicles:

- start/stop;
- idle;
- RPM/load;
- gear/transmission;
- braking/skid;
- tyres by surface;
- suspension/body impacts;
- cabin/exterior filtering;
- damage/fire;
- turret/M2 routing.

Character:

- footsteps by surface;
- jump/land;
- restrained gear/cloth;
- weapon/grenade handling;
- interactions;
- vehicle enter/exit;
- hit/death presentation if retained.

Environment:

- wind exposure;
- foliage/area response;
- birds/insects;
- distant dogs;
- road activity where appropriate;
- utility/electrical hum;
- room tone;
- weather;
- fire;
- contextual distant combat only when game state requires it.

Generic fallback audio remains migration-only and cannot close final weapon/environment acceptance.

## 8. Character animation / action presentation

### 8.1 Movement authority

Keep `CharacterMovementComponent` as authoritative gameplay movement unless a measured runtime defect proves replacement necessary.

Do not run multiple movement owners.

### 8.2 Locomotion presentation pilot

Pilot Game Animation Sample / Motion Matching / Pose Search for third-person:

- idle;
- walk/run/sprint;
- start/stop/turn;
- crouch if compatible;
- jump/fall/land;
- slope/stair presentation.

This may replace an old locomotion presentation state machine only after visual/network/performance parity. It does not own gameplay velocity or weapon action timing.

### 8.3 Discrete authored actions

Use authored sequences/Montages, with IK Retargeter/Control Rig correction as necessary, for:

- reload families;
- M700 bolt;
- Remington 870 pump;
- Lever Action cycle;
- grenade hand/prepare/throw/recover;
- vehicle enter/exit;
- M2/turret mount/use/exit;
- hit/death transition where applicable.

Do not resurrect the retired procedural whole-weapon manual-action fallback.

### 8.4 IK / rigging / alignment

Adopt IK Rig/Retargeter + Control Rig selectively for:

- cross-skeleton animation reuse;
- hand-to-weapon alignment;
- moving weapon-part animation where mesh/skeleton supports it;
- foot correction;
- cleanup of imported animation.

Motion Warping may own contextual alignment to vehicle/turret/interaction targets, but it may not become a second gameplay movement/teleport authority.

## 9. AI migration

Keep existing `AI Perception` and navigation.

Pilot Behavior Tree + Blackboard for high-level modes now living in the custom decision loop, including:

- attack/defend objective;
- engage target;
- revive;
- regroup/follow squad order;
- resupply;
- use vehicle;
- seek cover/firing position.

Use EQS for spatial selection such as cover, revive approach, firing position, resupply and safe placement around smoke.

Adopt only after parity tests prove the same team/difficulty/squad/gameplay rules. Then delete the superseded high-level decision-loop ownership rather than running BT and the old C++ loop concurrently.

Smart Objects remain a pilot for seats, mounted weapons, doors, resupply and contextual positions. Mass AI is deferred until profiling proves it is needed.

## 10. World / models / materials

### 10.1 Generic content

Prefer verified production assets/materials over C++ BasicShape approximations for non-unique props and surfaces.

Approved source classes after per-item provenance/quality check:

- existing verified Oster assets;
- Poly Haven CC0;
- Kenney CC0 where visual style is acceptable;
- properly licensed Fab assets;
- other explicit CC0/permissive sources entered in the register.

### 10.2 Unique Oster identity

Museum, Silpo, Culture House, water tower, stadium and other evidence-bound landmarks remain reference-driven Oster content. Generic asset packs may supply doors/windows/materials/furniture/props but may not overwrite factual shape/site identity.

### 10.3 PCG/HLOD/World Partition

PCG may own generic repeated clutter/foliage distribution after visual/performance proof. It may not move or generate evidence-bound landmarks.

Pilot HLOD where distant static content is a measured draw-call/performance problem.

Do not convert the entire project to World Partition during active PASS45 unless a dedicated map/profiler audit proves the migration benefit exceeds the risk.

### 10.4 Vegetation item 27

Do not keep repairing a rejected KiteDemo family indefinitely because its paths already exist.

Required order:

`find verified UE 5.8-compatible foliage -> material/LOD/startup proof -> Foliage/PCG/instancing proof -> direct Oster visual acceptance -> remove quarantined rejected-family owner/assumptions`

Species/placement claims remain tied to Oster references.

## 11. Multiplayer/UI/framework policy

Lyra is a reference and bounded donor/pattern source, not a wholesale replacement target during PASS45.

Potential later bounded reuse:

- session/login flow;
- team/game-state patterns;
- settings/device profiles;
- EOS integration if required;
- modular UI ideas.

Keep current Enhanced Input/UMG during PASS45. Common User is post-PASS45 pilot-only unless a current blocking online/session requirement appears. CommonUI input migration is deferred.

## 12. Drone policy

Do not make AirSim/Project AirSim a current PASS45 dependency.

For future FPV scope, IAMAI Project AirSim may be used as an MIT reference/pilot only after UE 5.8 build compatibility and runtime/network/performance are proven. Import only the physics/controller/sensor pieces that actual gameplay needs; do not inherit autonomous-driving/ROS/GIS infrastructure by default.

## 13. Provenance / license gate

Before any external code/content import, `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` must contain:

- source/publisher;
- exact URL/repository;
- exact tag/commit/version;
- license/terms;
- exact files/assets to import;
- local modifications;
- attribution requirements;
- redistribution/public-repository restrictions;
- intended Oster owner replaced;
- runtime dependency state;
- acceptance evidence;
- cutover commit;
- old-owner removal commit.

Public GitHub visibility does not equal permission to reuse. OpenTournament is explicitly reference-only/rejected for import under this audit. Unknown YouTube/game/movie/ripped audio/assets are forbidden.

## 14. PASS45 open-item mapping

This strategy does not reopen already source-closed items merely to refactor them.

Apply it to still-open/directly affected work:

- **16** — authored M700/870/Lever moving-part actions via real sequences/Montages + IK/retarget/Control Rig where useful; no procedural fallback resurrection;
- **18** — ADS remains Oster-specific factual socket/optic calibration; IK/Control Rig may assist alignment;
- **20** — preserve Oster factual audio events; adopt Audio Modulation, pilot MetaSounds selectively, populate exact weapon mechanical/shot/reload/distant content;
- **24** — Niagara remains VFX owner, Chaos physics remains physical base, authored Montage/sequence owns grenade hand/throw/recover;
- **27** — replace rejected vegetation family with verified UE 5.8 content + Foliage/PCG/instancing after startup/material/LOD proof;
- **28–29** — Chaos Vehicles proof/migration for HMMWV driving if accepted; M2 shared turret gameplay remains Oster-owned;
- **30–31** — BTR material/orientation/remote operator remain Oster-specific; no forced Chaos vehicle conversion until separately proven;
- **32** — generic world quality may use PCG/HLOD/CC0/licensed assets without changing evidence-bound landmark ownership;
- **33** — every adopted replacement must survive fullscreen/~60 FPS/native-scale/thermal acceptance;
- **34** — keep Oster tactical-map/georeference architecture unless a measured defect justifies replacement;
- **35** — add replacement-specific full-runtime assertions only after the migration actually lands;
- **36** — unchanged: no PR #94 merge without factual current-head UE 5.8 acceptance.

## 15. Recommended migration order

Do not change every architecture layer at once.

1. finish current item 16 content/runtime boundary;
2. build isolated Chaos Vehicles proof car;
3. replace one prototype destructible cube-chunk set with Geometry Collection proof;
4. introduce Audio Modulation buses without changing factual event ownership;
5. pilot one MetaSound, preferably vehicle engine RPM/load;
6. pilot one Motion Matching third-person character without touching weapon/gameplay movement authority;
7. establish IK/retarget/Control Rig pipeline for weapon/action content;
8. migrate one AI high-level decision slice to BT/EQS after parity tests exist;
9. replace rejected vegetation family with verified UE 5.8 content;
10. only then expand Soundscape, HLOD/PCG, Smart Objects and future drone research.

## 16. Acceptance and deletion gate

Every migration must prove both:

**new owner present + old duplicate absent.**

A migration is incomplete if the new system works but obsolete runtime code can regain authority through a fallback, timer, delayed subsystem, config path, old Blueprint or stale verifier.

Final pattern:

`accepted replacement -> authority switched -> obsolete owner physically deleted -> obsolete tests/config/data deleted -> clean build -> source regression -> local UE 5.8 runtime -> multiplayer/performance/visual/audio acceptance`

Git history is the rollback mechanism.

## 17. Runtime truth

This specification changes implementation strategy only.

It does **not** upgrade any source-only result to runtime acceptance, does not change the formal PASS45 checklist completion percentage by itself, and does not authorize merge.

**PASS45 remains RUNTIME REJECTED 2026-08-31 until superseded by factual current-head local UE 5.8 evidence. PR #94 remains OPEN / UNMERGED.**
