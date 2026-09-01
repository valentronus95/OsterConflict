# THIRD-PARTY CODE AND ASSET REGISTER

Date created: 2026-09-01
Parent policy: `_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`

This register is required before external code or content is imported into Oster Conflict.

Status values:

- `ENGINE-NATIVE` — ships as part of Unreal Engine / plugin under Unreal Engine terms;
- `EPIC-SAMPLE` — Epic sample/example content or code; usable only under the applicable Unreal/Epic terms;
- `OPEN-SOURCE-CANDIDATE` — permissive source reviewed, not yet imported;
- `ASSET-CANDIDATE` — external content candidate, not yet imported;
- `REFERENCE-ONLY` — may inform design/implementation but is not a runtime dependency;
- `REJECTED` — do not use.

| Name | Intended Oster use | Source class | License / terms | Current decision | Version/pin | Imported files | Notes |
|---|---|---|---|---|---|---|---|
| Unreal Engine 5.8 Chaos Vehicles | Wheeled vehicle physics: chassis, wheels, suspension, engine, transmission, friction, braking | Engine-native | Unreal Engine EULA | ENGINE-NATIVE / preferred production baseline | UE 5.8.x | none added by this register | Replace duplicate custom vehicle-force owners only after isolated parity/runtime/network acceptance. |
| Unreal Engine 5.8 Chaos Modular Vehicles | Research for modular/detachable vehicle structure and network physics | Engine-native Experimental plugin | Unreal Engine EULA | REFERENCE/PROOF ONLY | UE 5.8.x | none | UE 5.8 marks this Experimental. Never production-promote without packaged multiplayer/performance proof. |
| Unreal Engine 5.8 Chaos Physics / Destruction | Rigid bodies, ragdoll, debris, Geometry Collections, destruction, Physics Fields | Engine-native | Unreal Engine EULA | ENGINE-NATIVE / preferred | UE 5.8.x | none | Gameplay state remains server-authoritative; cosmetic debris should not create replication flood. |
| Unreal Engine 5.8 Niagara | Explosions, smoke, muzzle flash, impacts, fire and ambient VFX | Engine-native | Unreal Engine EULA | ENGINE-NATIVE / preferred VFX owner | UE 5.8.x | none | Niagara is presentation, never damage authority. |
| Unreal Engine 5.8 MetaSounds | Data-driven weapon, vehicle, explosion and environmental audio | Engine-native | Unreal Engine EULA | ENGINE-NATIVE / preferred audio presentation | UE 5.8.x | none | Final migration needs exact sound assets and runtime mix acceptance. |
| Unreal Engine 5.8 Audio Modulation | Audio buses, dynamic mix, indoor/outdoor, vehicle interior, flash/health effects | Engine-native plugin | Unreal Engine EULA | ENGINE-NATIVE / preferred mix layer | UE 5.8.x | none | Plugin disabled by default; enable only with documented project config. |
| Unreal Engine 5.8 Soundscape | Procedural environmental ambience | Engine-native Beta plugin | Unreal Engine EULA | PROOF ONLY | UE 5.8.x | none | UE 5.8 marks Soundscape Beta; packaged streaming/performance proof required before production ownership. |
| Epic Game Animation Sample / Motion Matching | Third-person locomotion architecture, Pose Search, animation selection, retargeting patterns | Epic sample / engine systems | Unreal Engine EULA / applicable Epic sample terms | EPIC-SAMPLE / migration candidate | UE 5.8-compatible sample | none | Do not allow animation sample to become a second gameplay movement owner. First-person weapon arms remain separate. |
| Lyra Sample Game / Common User patterns | Multiplayer/session/team/spawn/UI architecture reference and selective plugin reuse | Epic sample | Unreal Engine EULA / Epic terms | EPIC-SAMPLE / selective reuse only | UE 5.8-compatible sample | none | Do not import the whole game architecture unless a bounded migration proves concrete benefit. |
| ALS Community | Character locomotion/reference candidate | Open source | MIT | OPEN-SOURCE-CANDIDATE | pin required before import | none | Compare against Motion Matching/current movement; never run as a parallel second locomotion owner after cutover. |
| Project Borealis PBCharacterMovement | FPS movement reference/candidate | Open source | MIT | OPEN-SOURCE-CANDIDATE / low priority | upstream binaries currently noted for UE 5.5; UE 5.8 pin/build proof required | none | HL2-style movement may be wrong for Oster. Do not import unless design and UE 5.8 compatibility both pass. |
| CARLA ue5-dev | Vehicle simulation architecture/tuning reference and possible narrow donor after file-level review | Open source project | CARLA-specific code MIT; CARLA assets CC-BY; dependencies have separate terms | REFERENCE-ONLY by default | exact commit required before any donor import | none | Do not import CARLA wholesale. Review every donor file/dependency separately. |
| Poly Haven | Generic non-identity-critical 3D models, textures and HDRIs | External asset library | CC0 | ASSET-CANDIDATE / preferred permissive content source | per-asset record required | none | May support generic props/materials; cannot replace photo-bound Oster landmark identity. |
| Fab assets | Production models/materials/audio/VFX where appropriate | External marketplace | Per-asset Fab license; commonly Fab Standard or CC-BY, verify each item | ASSET-CANDIDATE | per-asset record required | none | Do not assume all Fab content has identical terms. Standalone redistribution is restricted under Fab Standard License. |
| AirSim / Project AirSim lineage | Future drone flight/sensor architecture reference | Open source lineage | MIT for Microsoft AirSim; exact fork/project license must be rechecked before import | REFERENCE-ONLY / future | exact repo+commit required | none | Legacy Microsoft AirSim is archived/legacy. Do not make it a current PASS45 dependency. |

## Intake record template

Copy this block for every actual external import:

```text
ID:
NAME:
SOURCE/PUBLISHER:
SOURCE_URL:
VERSION_TAG_COMMIT:
LICENSE:
DATE_ACQUIRED:
STATUS: CANDIDATE | APPROVED | REJECTED
OSTER_OWNER_REPLACED:
FILES_IMPORTED:
FILES_MODIFIED:
ATTRIBUTION_REQUIRED:
REDISTRIBUTION_RESTRICTIONS:
PUBLIC_REPO_ALLOWED:
RUNTIME_DEPENDENCY:
ACCEPTANCE_EVIDENCE:
CUTOVER_COMMIT:
OLD_OWNER_REMOVAL_COMMIT:
NOTES:
```

Unknown license, unknown source, or unpinned code/content is not production-ready and must not be imported merely because it is free to download.
