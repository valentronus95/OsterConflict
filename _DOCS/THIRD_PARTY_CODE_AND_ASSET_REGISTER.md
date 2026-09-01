# THIRD-PARTY CODE AND ASSET REGISTER

Date created: 2026-09-01
Last audited: 2026-09-01
Parent policy: `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`
Detailed audit: `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md`

This register is mandatory before external code or content is imported into Oster Conflict.

**Audit state:** no external source code or third-party content was imported by the 2026-09-01 reuse-first architecture audit itself. Two pre-existing PASS45 manual-action CC0 audio donor derivatives acquired on 2026-09-01 are now explicitly registered below; they remain source payloads with `runtime_ready=false` / `ue_import_pending=true`, not UE 5.8 runtime acceptance.

## Status values

- `ENGINE-NATIVE-KEEP` — already appropriate Unreal Engine capability; keep/standardize;
- `ENGINE-NATIVE-PILOT` — Unreal Engine capability, but Beta/Experimental/current migration risk requires isolated proof before production cutover;
- `ENGINE-NATIVE-ADOPT` — approved engine-native capability for bounded use;
- `EPIC-SAMPLE-PILOT` — Epic sample/pattern usable under applicable Unreal/Epic terms, subject to bounded migration proof;
- `OPEN-SOURCE-PILOT` — permissively licensed candidate, not yet imported;
- `REFERENCE-ONLY` — may inform design/implementation, not a runtime dependency;
- `ASSET-SOURCE-APPROVED` — acceptable source class after per-item quality/provenance record;
- `LICENSED-ASSET-SOURCE` — usable under per-item marketplace/license terms, not open source;
- `REJECTED` — do not import/use as production dependency.

| Name | Intended Oster use | Source / license class | Audited decision | Version / compatibility note | Imported by this audit | Key restriction / rationale |
|---|---|---|---|---|---|---|
| Unreal Engine Enhanced Input | Gameplay input | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-KEEP` | UE 5.8 | none | Already used by Oster; do not create a second gameplay input owner. |
| Unreal Engine Chaos rigid-body physics | Drops, grenades, props, ragdolls, debris, physical response | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-KEEP` | UE 5.8 | none | Standardize existing physics usage; remove only duplicate custom integrators. |
| Unreal Engine Chaos Vehicles | Wheeled suspension/powertrain/brake/steering replacement | Engine-native plugin / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 API still carries Experimental caution | none | Required isolated packaged multiplayer/performance proof before replacing `AOCVehicleBase` custom solver. |
| Unreal Engine Chaos Modular Vehicles | Modular/detachable vehicle research | Engine-native Experimental plugin / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 Experimental | none | Not production owner during PASS45 unless separately proven. |
| Unreal Engine Chaos Destruction / Geometry Collections / Physics Fields | Selected breakable props/windows/fences/vehicle pieces | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` | UE 5.8 | none | Selective only; do not make all Oster destructible. Replace BasicShape cube fragments after proof. |
| Unreal Engine Niagara | Explosions, smoke, fire, muzzle, impacts, environmental VFX | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-KEEP` / expand | UE 5.8, already a project dependency | none | Presentation only, never gameplay damage/inventory authority. |
| Unreal Engine Audio Modulation | Global/stateful audio buses and mixes | Engine-native plugin / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` | UE 5.8 | none | Use for Master/SFX/Vehicle/Ambience/UI/Music/Voice and state mixes. |
| Unreal Engine MetaSounds | Engine RPM/load, layered explosion, procedural ambience, selected weapon rendering | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 docs still include Beta caution | none | No blanket audio rewrite; pilot packaged audibility/voice/audio-thread cost first. |
| Unreal Engine Soundscape | Procedural environmental ambience | Engine-native Beta plugin / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 Beta | none | Must beat current ambient-zone polling/timers in packaged performance/voice tests before cutover. |
| Epic Game Animation Sample / Motion Matching / Pose Search | Third-person locomotion presentation | Epic sample + engine systems / applicable Unreal/Epic terms | `EPIC-SAMPLE-PILOT` | UE 5.8 path reviewed | none | Animation presentation only; no second gameplay movement owner. |
| Unreal Engine IK Rig / IK Retargeter | Animation reuse/retargeting, skeleton compatibility | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` | UE 5.8 | none | Use for retargeting and weapon/action compatibility. |
| Unreal Engine Control Rig | Hand/foot/weapon-part correction and authoring | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` | UE 5.8 | none | Authoring/correction, not gameplay authority. |
| Unreal Engine Motion Warping | Vehicle/M2/interaction alignment | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` selective | UE 5.8 | none | May align animation to target; may not become teleport/gameplay movement authority. |
| Unreal Engine Behavior Trees / Blackboard | High-level bot decision flow | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 | none | Keep current AI Perception/Navigation; migrate only after behavior parity tests. |
| Unreal Engine EQS | Cover/firing/revive/resupply/spatial queries | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` selective | UE 5.8 | none | Spatial queries only where beneficial; not every deterministic decision. |
| Unreal Engine Smart Objects | Seats, mounted weapons, doors, resupply, contextual positions | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 | none | Add only after bounded interaction proof. |
| Unreal Engine PCG | Generic foliage/clutter/repeated prop distribution | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-ADOPT` selective | UE 5.8 | none | Must never author/move evidence-bound landmarks. |
| Unreal Engine HLOD | Distant static world optimization | Engine-native / Unreal Engine terms | `ENGINE-NATIVE-PILOT` | UE 5.8 | none | Use only after profiler/map audit proves benefit. |
| Unreal Engine World Partition | Large-world streaming architecture | Engine-native / Unreal Engine terms | `REFERENCE-ONLY` / deferred migration | UE 5.8 | none | Do not convert whole project during active PASS45 without measured necessity. |
| Lyra Starter Game | Multiplayer/session/team/settings/UI architecture patterns | Epic sample / applicable Unreal/Epic terms | `REFERENCE-ONLY` / selective bounded migration | UE 5.8 sample family | none | Do not replace Oster wholesale during PASS45. |
| Common User | Login/auth/session flow | Epic/engine plugin terms | `EPIC-SAMPLE-PILOT` post-PASS45 | UE 5.8 | none | Only if online/session requirements exceed current flow. |
| CommonUI + Enhanced Input integration | UI input architecture | Epic/engine plugin terms | `REFERENCE-ONLY` / deferred | UE 5.8 integration still carries experimental caution | none | Keep current UMG + Enhanced Input during PASS45. |
| CARLA `ue5-dev` | Vehicle setup/tuning/simulation architecture reference | Open source; reviewed CARLA code under MIT, assets/dependencies may differ | `REFERENCE-ONLY` | Reviewed UE5 development line uses UE 5.5 / Chaos; exact donor commit required | none | Do not import CARLA wholesale. File-level license/dependency review before any donor code. |
| ALS Community | Character locomotion reference/backup | MIT | `REFERENCE-ONLY` | Public compatibility reviewed behind UE 5.8 | none | Prefer Epic current animation path first. |
| ALS Refactored | Multiplayer-focused locomotion backup | MIT | `OPEN-SOURCE-PILOT` backup | Latest reviewed release supports UE 5.7; UE 5.8 requires source proof | none | Pilot only if Epic Motion Matching path fails Oster needs. |
| Project Borealis PBCharacterMovement | FPS movement reference | MIT | `REFERENCE-ONLY` / default adoption rejected | Published binaries reviewed for UE 5.5; intentionally HL2-style movement | none | Bunnyhop/surf/Source-style goals conflict with grounded Battlefield-like Oster movement. |
| Legacy Microsoft AirSim | Drone simulation reference | MIT | `REFERENCE-ONLY` | Archived/legacy | none | Not a current dependency. |
| IAMAI Project AirSim | Future FPV/drone physics/controller reference | MIT | `OPEN-SOURCE-PILOT` future only | Reviewed support UE 5.2 / 5.7, not formal UE 5.8 | none | Isolate/recompile for UE 5.8; import only needed pieces, not full autonomy/ROS/GIS stack. |
| OpenTournament | Shooter implementation reference | Public source with project-specific legal/content restrictions | `REJECTED` for import | Current public repository reviewed | none | Public GitHub source is not automatically reusable OSS. Reference ideas only. |
| Poly Haven | Generic models/textures/HDRIs | CC0 | `ASSET-SOURCE-APPROVED` | Per-asset quality record required | none | Good generic source; may not replace photo-bound Oster landmark identity. |
| Kenney assets | Utility/generic models/UI/assets where style fits | CC0/public domain | `ASSET-SOURCE-APPROVED` | Per-asset quality/style review required | none | Many assets are stylized; not automatically suitable for realistic final Oster art. |
| Fab assets | Production models/materials/audio/VFX | Per-item Fab/creator license; often Fab Standard, verify each asset | `LICENSED-ASSET-SOURCE` | Per-asset entitlement/version required | none | Not open source. Standalone redistribution/source-pack exposure prohibited or restricted. |
| Freesound | Individual weapon/mechanical/ambient/impact recordings | Per-file CC0 / CC-BY / other license | `ASSET-SOURCE-APPROVED` only per file | Exact sound page/license pin required | none | CC0 preferred; CC-BY attribution recorded; BY-NC/restrictive/unknown license rejected for unrestricted production. |
| Unknown YouTube/game/movie/ripped audio or assets | Any | Unknown/unauthorized | `REJECTED` | n/a | none | Do not import. |

## Actual-import records

### PASS45-3P-AUDIO-001 — lever-action mechanical donor

```text
ID: PASS45-3P-AUDIO-001
NAME: lever_action_cc0_preview_donor.wav / Lever action cocking.wav
SOURCE/PUBLISHER: Freesound / uploader C-V
SOURCE_URL: https://freesound.org/people/C-V/sounds/523401/
VERSION_TAG_COMMIT_OR_ASSET_VERSION: Freesound sound 523401; pinned public-preview transport SHA256 ae257485c6d55f4a4587f99389882cf74eae6779db807eaa0aa0f968e711f965; deterministic derivative SHA256/LFS OID 417ba38e5e87b53ef3711784f821f1b3fc303ac8d4df19d9eda80fb776881542
LICENSE_TERMS: Creative Commons Zero (CC0) 1.0; source-page CC0 markers are fail-closed validated by PASS45_MANUAL_ACTION_AUDIO_INTAKE.py; canonical license https://creativecommons.org/publicdomain/zero/1.0/
DATE_ACQUIRED: 2026-09-01
STATUS: PILOT
OSTER_OWNER_REPLACED: none; this donor supplies LeverCycle presentation content only and owns no gameplay/action state
FILES_ASSETS_IMPORTED: SOURCE_ASSETS/PASS45/ManualActionAudio/lever_action_cc0_preview_donor.wav (Git LFS); SOURCE_ASSETS/PASS45/ManualActionAudio/MANIFEST.json lever entry
FILES_MODIFIED: PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md; PASS45_MANUAL_ACTION_AUDIO_INTAKE.py; .github/workflows/pass45-manual-action-audio-intake.yml
ATTRIBUTION_REQUIRED: no copyright attribution requirement under CC0; project provenance is retained as an internal audit requirement
REDISTRIBUTION_RESTRICTIONS: no CC0 copyright restriction on copying/modification/distribution; CC0 does not waive unrelated patent/trademark/privacy/publicity rights
PUBLIC_REPO_ALLOWED: yes for this verified CC0 derivative and its provenance record
RUNTIME_DEPENDENCY: not yet; manifest remains runtime_ready=false / ue_import_pending=true and no runtime acceptance is claimed
UE_5_8_BUILD_EVIDENCE: pending local UE 5.8 import/fresh-load/runtime acceptance
MULTIPLAYER_EVIDENCE: pending; cosmetic presentation must not create a second gameplay owner or dedicated-server playback burden
PERFORMANCE_EVIDENCE: pending runtime/audio profiling if adopted
VISUAL_AUDIO_ACCEPTANCE: pending direct UE 5.8 first-person audibility/timing acceptance
CUTOVER_COMMIT: pending runtime acceptance
OLD_OWNER_REMOVAL_COMMIT: n/a; rejected procedural whole-weapon manual-action fallback was retired separately, this donor does not replace gameplay authority
NOTES: acquisition workflow run 33501795799 SUCCESS. Identity scope is lever-action-family donor only; it is not proof of exact Stein/Marlin/Model-1894 identity.
```

### PASS45-3P-AUDIO-002 — bolt-action mechanical donor

```text
ID: PASS45-3P-AUDIO-002
NAME: bolt_action_cc0_preview_donor.wav / Mosin Nagant Bolt.wav
SOURCE/PUBLISHER: Freesound / uploader rammbostein
SOURCE_URL: https://freesound.org/people/rammbostein/sounds/263459/
VERSION_TAG_COMMIT_OR_ASSET_VERSION: Freesound sound 263459; pinned public-preview transport SHA256 d9f4ee7633275f911f3521b5b7b319d634022944aafb9e7f51660a8a342d3040; deterministic derivative SHA256/LFS OID 5e64820d532c11e91af3eedf96ab34a38df7b3dd066b0b1c9d67b3fe3f34c8a7
LICENSE_TERMS: Creative Commons Zero (CC0) 1.0; source-page CC0 markers are fail-closed validated by PASS45_MANUAL_ACTION_AUDIO_INTAKE.py; canonical license https://creativecommons.org/publicdomain/zero/1.0/
DATE_ACQUIRED: 2026-09-01
STATUS: PILOT
OSTER_OWNER_REPLACED: none; this donor supplies BoltCycle presentation content only and owns no gameplay/action state
FILES_ASSETS_IMPORTED: SOURCE_ASSETS/PASS45/ManualActionAudio/bolt_action_cc0_preview_donor.wav (Git LFS); SOURCE_ASSETS/PASS45/ManualActionAudio/MANIFEST.json bolt entry
FILES_MODIFIED: PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md; PASS45_MANUAL_ACTION_AUDIO_INTAKE.py; .github/workflows/pass45-manual-action-audio-intake.yml
ATTRIBUTION_REQUIRED: no copyright attribution requirement under CC0; project provenance is retained as an internal audit requirement
REDISTRIBUTION_RESTRICTIONS: no CC0 copyright restriction on copying/modification/distribution; CC0 does not waive unrelated patent/trademark/privacy/publicity rights
PUBLIC_REPO_ALLOWED: yes for this verified CC0 derivative and its provenance record
RUNTIME_DEPENDENCY: not yet; manifest remains runtime_ready=false / ue_import_pending=true and no runtime acceptance is claimed
UE_5_8_BUILD_EVIDENCE: pending local UE 5.8 import/fresh-load/runtime acceptance
MULTIPLAYER_EVIDENCE: pending; cosmetic presentation must not create a second gameplay owner or dedicated-server playback burden
PERFORMANCE_EVIDENCE: pending runtime/audio profiling if adopted
VISUAL_AUDIO_ACCEPTANCE: pending direct UE 5.8 first-person audibility/timing acceptance
CUTOVER_COMMIT: pending runtime acceptance
OLD_OWNER_REMOVAL_COMMIT: n/a; rejected procedural whole-weapon manual-action fallback was retired separately, this donor does not replace gameplay authority
NOTES: acquisition workflow run 33501795799 SUCCESS. Identity scope is bolt-action-family donor from a Mosin-Nagant source; it is not proof of exact M700 identity.
```

The existing pump-cycle source fallback `/Game/R13/Audio/shotguncock.shotguncock` is project-owned repository content and is not an external import record.

## Mandatory actual-import record

Create one record for every external code/content import before production use:

```text
ID:
NAME:
SOURCE/PUBLISHER:
SOURCE_URL:
VERSION_TAG_COMMIT_OR_ASSET_VERSION:
LICENSE_TERMS:
DATE_ACQUIRED:
STATUS: CANDIDATE | PILOT | APPROVED | REJECTED
OSTER_OWNER_REPLACED:
FILES_ASSETS_IMPORTED:
FILES_MODIFIED:
ATTRIBUTION_REQUIRED:
REDISTRIBUTION_RESTRICTIONS:
PUBLIC_REPO_ALLOWED:
RUNTIME_DEPENDENCY:
UE_5_8_BUILD_EVIDENCE:
MULTIPLAYER_EVIDENCE:
PERFORMANCE_EVIDENCE:
VISUAL_AUDIO_ACCEPTANCE:
CUTOVER_COMMIT:
OLD_OWNER_REMOVAL_COMMIT:
NOTES:
```

Unknown license, unknown source, unpinned code/content, or “it is on GitHub so it must be free” is not an acceptable production provenance claim.
