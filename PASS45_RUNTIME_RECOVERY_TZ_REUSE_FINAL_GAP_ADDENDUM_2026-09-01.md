# PASS45 RUNTIME RECOVERY — REUSE-FIRST FINAL GAP ADDENDUM

Date: 2026-09-01
Parent: `PASS45_RUNTIME_RECOVERY_TZ.md`
Related normative policy: `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`
Related deep audit: `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md`
Scope: Oster Conflict / Unreal Engine 5.8.x / Windows / multiplayer FPS
Status: **MANDATORY ADDENDUM FROM LATEST EXPLICIT USER REQUIREMENT**

## 0. Authority and purpose

This addendum records reuse/engine-native areas that were not explicit enough in the first reuse-first audit. It does not reopen already source-closed PASS45 checklist work merely to refactor it and does not change factual runtime truth.

The existing replacement doctrine still applies:

`audit current owner -> prove replacement in isolation -> integrate replacement -> runtime/network/performance acceptance -> switch authority -> physically delete obsolete duplicate -> delete stale tests/config/fallbacks -> regression acceptance`

Latest explicit user requirement requires these gaps to be considered by ongoing PASS45 execution. If an item below conflicts with an older reuse assumption, this newer audited rule wins for implementation strategy. Runtime screenshots/logs still outrank source optimism.

## 1. Asset loading / startup / streaming — REQUIRED

### Current problem

The current project has already demonstrated that synchronous production-asset work during native construction/startup can block or reject first-frame runtime. The 2026-08-31 vegetation failure is direct evidence: production tree packages/material/static-mesh work were reached before usable gameplay.

### Decision

**ADOPT Unreal Engine Asset Manager / soft-reference / asynchronous-loading patterns for heavy or optional production content.**

Use as appropriate:

- `UAssetManager` for managed asset groups and auditing;
- `TSoftObjectPtr` / `TSoftClassPtr` or equivalent soft paths for content that must not hard-load with the class/CDO;
- async/streamable loading for non-critical production meshes, materials, audio, animation and VFX where first-frame ownership does not require synchronous residency;
- explicit preload groups only for genuinely mandatory first-frame content;
- unload/release of optional groups where safe;
- Asset Manager audit/chunk/memory information for packaged builds.

### Hard rules

- no new heavy production mesh/material/audio/animation family may be added to a native constructor/CDO with `ConstructorHelpers::FObjectFinder` merely for convenience;
- synchronous `LoadObject` in gameplay-critical startup paths requires an explicit reason and measured cost;
- first usable menu/gameplay frame must not wait on optional world-detail content;
- asset-loading failure remains fail-visible and must not resurrect BasicShape/default proxy content;
- item 27 vegetation replacement must prove startup loading behavior as well as visual/material/LOD quality;
- future HMMWV/BTR/weapon/landmark content migration must distinguish `must be resident now` from `may stream after startup`.

This is an implementation rule, not a claim that every existing hard reference must immediately be rewritten during item 16.

## 2. Multiplayer replication / dedicated server — explicit production policy

### Existing production owner

Oster already uses Unreal's normal server-authoritative replication/RPC model. Keep it during active PASS45 unless profiling proves a scaling problem.

### Dedicated server rule

Every reused subsystem that can affect gameplay must be tested with dedicated/listen server semantics as relevant. Cosmetic-only systems must do no unnecessary playback/render/animation work on dedicated server.

Required future acceptance coverage includes:

- authoritative weapon/grenade/damage events;
- vehicle possession and driving;
- M2/BTR turret authority;
- AI state;
- destroyed/intact gameplay state;
- replicated animation/action presentation triggers where needed;
- late join/reconnect state correctness where the feature persists.

### Replication Graph

UE 5.8 Replication Graph is a scalability system for large actor/connection counts, but Epic still marks the plugin Beta.

**Decision: PROFILE-TRIGGERED PILOT ONLY.**

Do not add it during PASS45 merely because Oster is multiplayer. Pilot only if network/server profiling proves normal replication becomes a bottleneck at the actual target player/bot/actor count.

### Iris

**UE 5.8 correction:** Epic's 5.8 release notes mark Iris as production-ready. The earlier Experimental wording is obsolete.

**Decision: DEFER MIGRATION DURING PASS45 UNLESS A MEASURED REPLICATION CORRECTNESS/SCALING NEED JUSTIFIES A BOUNDED PILOT.**

Do not migrate the project to Iris simply because it is now production-ready. Existing replication remains the production owner while runtime recovery is active. Iris may be evaluated after baseline multiplayer is stable, or earlier only if profiling/factual network defects demonstrate a concrete benefit large enough to justify the migration risk.

Iris and Replication Graph are alternative replication architectures for a tested NetDriver path, not systems to stack together by default. A future pilot must have explicit rollback, protocol/build compatibility checks, bad-network tests and measured traffic/correction evidence.

## 3. AI high-level ownership — add StateTree to the decision

The first audit considered Behavior Tree + Blackboard and EQS but did not explicitly compare StateTree.

UE 5.8 StateTree is a hierarchical state-machine system combining tree selection with explicit states/transitions and is a natural candidate for Oster's current custom high-level bot modes.

**Decision: PILOT StateTree against Behavior Tree for one bounded high-level bot slice before choosing the long-term owner.**

Candidate pilot slice:

`objective -> engage -> seek cover -> revive/regroup -> resume objective`

Keep:

- AI Perception;
- Navigation;
- squad/team/game-mode authority;
- EQS for spatial scoring where useful.

After comparison choose **one** high-level decision owner. Do not ship `RunDecisionLoop` + Behavior Tree + StateTree all mutating the same bot state.

## 4. Spatial audio / indoor-outdoor acoustics — engine-native stable layer

The original audit covered Audio Modulation, MetaSounds and Soundscape but did not explicitly require stable engine Audio Volumes.

**Decision: ADOPT Audio Volumes / attenuation / reverb / ambient-zone processing for physical-space acoustics where appropriate.**

Use for:

- indoor/outdoor transitions;
- room/building reverb;
- exterior sound attenuation/low-pass while indoors;
- interior sound attenuation/low-pass while outside;
- Museum/Silpo/houses/large rooms where acoustic boundaries are meaningful;
- tunnel/enclosed/vehicle-cabin cases where a dedicated volume or mix route is appropriate.

Audio Modulation remains the mix/state layer. Audio Volumes describe physical acoustic space. MetaSounds render selected sound designs. Soundscape remains Beta pilot-only for procedural ambience.

Do not expand custom `AOCAmbientAudioZone` polling to solve stable engine reverb/inside-outside problems that Audio Volumes already cover.

## 5. Performance prioritization for bots/characters/VFX/audio

### Animation Budget Allocator

UE 5.8 provides Animation Budget Allocator to constrain skeletal-animation game-thread cost by reducing update frequency/quality for less significant meshes.

**Decision: PILOT -> ADOPT IF BOT/PLAYER ANIMATION PROFILING JUSTIFIES IT.**

Use only after profiling with representative bot/player counts. Local first-person arms and nearby critical actors must not be visibly degraded by budget throttling.

### Significance Manager

UE 5.8 Significance Manager is intended for project-specific prioritization when many players/AI/objects converge.

**Decision: SELECTIVE ADOPT as a common prioritization framework when profiling shows benefit.**

Possible controlled uses:

- reduce AI think frequency for far/unimportant bots;
- suppress or reduce low-value distant VFX;
- reduce cosmetic audio emitters/one-shots;
- reduce animation/detail work for distant actors;
- prioritize nearby combat/vehicles/players.

It may reduce cosmetic/processing detail, but it may never turn off authoritative damage, possession, inventory, objective, projectile or critical replicated state.

## 6. Rendering architecture — do not blindly enable expensive UE5 features

### Current factual project constraint

Current `DefaultEngine.ini` pins the Windows project to DX11/SM5 after prior D3D12/RHI instability and currently disables dynamic GI/reflections, Virtual Shadow Maps, ray tracing and mesh distance fields as part of a boot-safe profile.

### Nanite / Virtual Shadow Maps

UE 5.8 Nanite and Virtual Shadow Maps require a DX12/SM6-class path on Windows; the current DX11/SM5 runtime path does not support Nanite production rendering.

**Decision: DEFER. Do not enable Nanite/VSM during active PASS45 runtime recovery.**

A future D3D12/SM6 stability project may pilot them after current-head runtime is accepted. Until then use conventional authored LOD/HLOD/instancing/material optimization.

### Lumen

Lumen can materially improve lighting but has a non-trivial frame budget and the project currently disables dynamic GI/reflections for stability/performance.

**Decision: DEFER TO A SEPARATE POST-STABILITY VISUAL/PERFORMANCE PILOT.**

Do not use Lumen activation as a shortcut for fixing bad materials, daylight, landmark composition or missing authored content. Any future pilot must compare image quality against the game's 60 FPS and low/mid-PC targets.

## 7. Gameplay Ability System (GAS) — explicit no-migration rule for PASS45

GAS is a mature Unreal framework with multiplayer replication/prediction, attributes, effects, gameplay tags, abilities and cues.

It could theoretically model sprint/revive/grenade/status/damage/action systems, but Oster already has substantial authoritative gameplay code. Migrating now would create broad dual ownership across health, weapons, actions and state.

**Decision: DO NOT MIGRATE OSTER TO GAS DURING PASS45.**

Future adoption is allowed only if later gameplay complexity (classes, buffs/debuffs, suppression/status systems, many reusable abilities) creates a measured maintenance benefit large enough to justify a dedicated migration project.

Do not add AbilitySystemComponent beside current health/weapon/action owners just to 'prepare for later'.

## 8. Online services / EOS / voice chat — post-stability boundary

UE 5.8 provides Online Subsystem / newer Online Services, EOS integrations and a Voice Chat Interface.

**Decision: POST-PASS45 PILOT ONLY unless internet matchmaking/voice becomes an explicit immediate requirement.**

First stabilize:

- local multiplayer;
- dedicated/listen server gameplay;
- session lifecycle already required by Oster;
- replicated gameplay correctness.

Then evaluate bounded integration for:

- internet sessions/matchmaking;
- account/login needs;
- server discovery;
- friends/invites if required;
- team/squad voice chat.

Do not make EOS/login/voice a dependency of current runtime recovery.

## 9. Ballistics / damage / impacts — keep Oster authority, reuse engine primitives

No external general-purpose shooter framework is approved to replace Oster weapon/damage authority during PASS45.

**Decision: KEEP Oster-specific ballistics/damage rules; STANDARDIZE engine-native supporting data/presentation.**

Use Physical Materials/data profiles for shared surface identity where possible:

- concrete;
- brick;
- metal;
- wood;
- glass;
- asphalt;
- soil/grass;
- water where relevant.

Drive from factual hit/projectile events:

- impact VFX;
- decals;
- impact audio;
- ricochet presentation when physically/design appropriate;
- penetration/energy-loss rules only where explicitly designed and tested;
- tyre/vehicle surface sound and grip mappings where relevant.

Niagara/audio/decal presentation never becomes hit or damage authority.

## 10. Camera / first-person presentation — keep existing gameplay owner

Do not import another full camera framework during PASS45.

Keep the current Oster camera/ADS/recoil/free-look/vehicle-camera ownership and use stable engine primitives such as Spring Arms, Camera Components and Camera Shakes where appropriate.

Any future camera framework migration must first prove it fixes a measured defect and does not create a second recoil/ADS/vehicle-camera owner.

## 11. Settings / scalability / device profiles

Current user settings remain Oster-owned, but final low/mid-PC support must use Unreal scalability/device-profile mechanisms rather than scattered one-off console settings forever.

**Decision: ADOPT/standardize scalability groups and Device Profiles after runtime baseline is stable.**

Provide bounded quality levels for at least:

- view distance;
- shadows;
- effects/VFX;
- foliage;
- texture quality;
- post processing;
- resolution scale/AA strategy where accepted;
- animation/crowd budgets where exposed indirectly.

The default profile must preserve the 60 FPS target and acceptable readability on realistic target PCs/laptops. Higher settings may improve visuals but may not become the only playable configuration.

## 12. Diagnostics / reproducibility tooling

Do not invent project-specific profiling systems when Unreal tooling already exists.

Use engine/network/audio/CPU/GPU profiling and Unreal Insights where applicable before approving architecture replacements.

The built-in Replay system may be evaluated as a debugging/reproduction tool for replicated gameplay issues after baseline replication is stable. It is not a gameplay dependency and does not replace factual direct UE runtime acceptance.

## 13. Updated execution priority

These additions do not interrupt the current first-open PASS45 item solely to refactor unrelated systems.

Apply immediately when the relevant open work is touched:

1. **item 16** continues as current action-animation/audio content boundary;
2. **item 27 / startup** must use the new asset-loading/soft-reference rule when replacing vegetation or other heavy startup content;
3. **items 28–31 vehicles** must include dedicated/listen-server replication proof for any vehicle-system pilot;
4. **item 32 world fidelity** uses Audio Volumes for physical acoustic spaces, conventional LOD/HLOD/instancing under current DX11 path, and Asset Manager/soft references where streaming is justified;
5. **item 33 performance** adds animation/significance/scalability profiling where representative actor counts justify it;
6. future AI work must compare StateTree vs Behavior Tree and choose one high-level owner;
7. Replication Graph/Iris/GAS/EOS/Nanite/VSM/Lumen remain explicitly gated/deferred rather than being silently introduced during runtime recovery.

## 14. Final guard

A new engine feature is not automatically an improvement. Adoption requires a measured problem, a bounded proof, and a cheaper/better result than the current owner.

Do not create architecture churn for fashion. Oster's production target remains a stable, readable, performant multiplayer FPS that can run well on ordinary target PCs/laptops.

This addendum does not authorize PR #94 merge and does not alter the latest factual runtime verdict. Current-head UE 5.8 direct runtime evidence remains mandatory.