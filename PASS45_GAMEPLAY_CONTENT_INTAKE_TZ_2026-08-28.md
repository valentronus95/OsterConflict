# OSTER CONFLICT — PASS45 GAMEPLAY CONTENT INTAKE TZ

Date: 2026-08-29  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Execution branch: `content/free-gameplay-assets-intake-20260828-r2`  
Status: **ACTIVE / MUST COMPLETE BEFORE CONTENT MERGE**

## 0. Execution semantics

This TZ is **not documentation-only**. Any executor continuing PASS45 content work must treat this file, `PASS45_CONTENT_INTEGRATION_LEDGER.csv` and the gate scripts as an execution contract.

The mandatory state machine is:

`DISCOVERED -> ACQUIRED -> INVENTORIED -> SELECTED -> MIGRATED/IMPORTED -> INTEGRATED -> RUNTIME_ACCEPTED OR EXCLUDED_WITH_REASON`

Rules:

- `DOWNLOADED`, `ACQUIRED`, `INVENTORIED` and `FILE_EXISTS` are never completion states;
- after acquisition the executor must continue into migration/import, runtime ownership wiring, production-path selection, optimization and UE 5.8 validation;
- every selected third-party pack/model/animation/audio/VFX item must have a row in `PASS45_CONTENT_INTEGRATION_LEDGER.csv`;
- every ledger row must identify role, target integration, runtime owner, required evidence and state;
- the executor must update ledger states as work progresses;
- a pack may be excluded only with a factual reason; silent abandonment is forbidden;
- no item may be marked `INTEGRATED` merely because it exists under `/Game`;
- no item may be marked `RUNTIME_ACCEPTED` without current UE 5.8 evidence;
- if a sample project exists only as a separate `.uproject`, the executor must inventory it and selectively migrate the needed assets with dependencies; blind folder copying is forbidden;
- if direct automated migration cannot be performed safely, the executor must implement the necessary Unreal-side migration/import step or leave the row explicitly `PENDING`, never pretend it was applied.

## 1. Goal

Make gameplay-content intake an explicit PASS45 gate. The objective is not to dump every free Fab pack into shipping content. The objective is to acquire, inventory, select, migrate, integrate, optimize and runtime-validate the assets needed for a complete first-person gameplay presentation.

This file is subordinate to `PASS45_RUNTIME_RECOVERY_TZ.md`. It does not override runtime rejection evidence. A downloaded file is never equivalent to a production-ready asset.

## 2. Mandatory execution command

Run from the repository tree:

`OsterConflict\Scripts\RUN_PASS45_GAMEPLAY_CONTENT_GATE.cmd`

The command must:

1. run `PREPARE_GAMEPLAY_CONTENT_58.cmd`;
2. download the pinned external donor sources listed in `GAMEPLAY_CONTENT_INTAKE_2026-08-28.md`;
3. generate/update the local SHA-256 receipt;
4. inventory local Fab content already added to Oster Conflict;
5. inventory external Unreal sample projects if present locally;
6. run `VERIFY_PASS45_GAMEPLAY_CONTENT_INTAKE.py`;
7. run `VERIFY_PASS45_GAMEPLAY_CONTENT_INTEGRATION.py`;
8. return `PENDING`/non-zero when required assets are only downloaded/inventoried but not integrated;
9. fail closed when a required category, provenance record, integration state or runtime requirement is missing.

`RUN_PASS45_GAMEPLAY_CONTENT_GATE.cmd` is a gate/orchestrator. It does not magically replace Unreal asset migration. Its job is to acquire, inventory and **refuse completion** until the integration ledger proves that implementation work actually happened.

## 3. Required acquired content categories

### A. Already-added Fab / UE content that must be inventoried

At minimum, the intake currently includes and must preserve factual paths for:

- `Content/AK-47/`;
- `Content/Fire_EXP_Vol01_Free/` including Niagara fire/explosion systems;
- `Content/PotaVFX_Smoke/`;
- `Content/Fab/Megascans/3D/Military_Trenches_*Sandbag*`;
- `Content/KiteDemo/Environments/` foliage/ground/rocks candidate content;
- `Content/Mega_Street_Props_Pack/`;
- `Content/Street_Props_Pack_V1/` when present;
- `Content/Megaplant_Library/`;
- `Content/PN_FoliageCollection/`;
- `Content/SampleAnimationPack/`;
- `Content/VehicleVarietyPack/`;
- `Content/AdvancedVillagePack/` and other already-added packs only as candidates subject to reference fidelity and anti-bloat rules;
- other Fab packs already committed on this intake branch.

Their presence only satisfies **ACQUIRED**, not **USED**, **OPTIMIZED** or **RUNTIME ACCEPTED**.

### B. Automated external donor intake

`FETCH_GAMEPLAY_CONTENT_58.cmd` must acquire the pinned/recorded sources for:

- first-person arms / AK animation donor;
- FPS SFX/weapon donor kit;
- shotgun report candidates;
- concrete footsteps;
- generic vehicle RPM loops;
- forest ambience;
- fire ambience;
- license/provenance records.

The exact sources and licensing state are controlled by:

`OsterConflict/SourceAssets/ThirdParty/GAMEPLAY_CONTENT_INTAKE_2026-08-28.md`

and the generated:

`OsterConflict/SourceAssets/ThirdParty/Gameplay/LOCAL_CONTENT_RECEIPT.csv`

### C. Separate Unreal projects created by Fab

The following are **external sample projects**, not blind copy targets:

- `SuperSimpleFPSPack`;
- `GameAnimationSample`;
- `InteractionSystem2` / equivalent Interaction System project.

If these projects exist locally, the intake must inventory their `.uasset/.umap` trees and dependencies. Migration into Oster Conflict must be selective and dependency-aware.

If one is absent locally, emit an explicit `LOCAL_SOURCE_MISSING` state. Do not mark the feature complete by assumption.

## 4. Required application map

The canonical machine-readable mapping is `PASS45_CONTENT_INTEGRATION_LEDGER.csv`. It defines what each downloaded model/pack is for and prevents content from becoming an inert library.

Required application intent:

- existing `SKM_Arms` -> first fix canonical FP arms visibility/attachment/AnimBP; external arms are fallback/donor only;
- `FPSArms3D` -> first-person arms/AK animation donor when factual retargeting is useful;
- `SampleAnimationPack` / `GameAnimationSample` -> selected/retargeted animation content, never a replacement gameplay controller;
- `AK-47` -> exact useful weapon mesh/animation donor only after grip/ADS/material/runtime validation;
- `SuperSimpleFPSPack` -> HUD/widget/reference donor only; Oster HUD remains authoritative;
- `InteractionSystem` -> selective interaction components only; Oster retains one authoritative interaction owner;
- `Fire_EXP_Vol01_Free` -> authored Niagara fire/explosion/debris presentation;
- `PotaVFX_Smoke` -> smoke grenade/environment smoke presentation;
- Quixel military sandbags -> selective fortification/world props;
- street-prop packs -> selective Oster-compatible street furniture/infrastructure;
- foliage/ground packs -> selective Oster-compatible vegetation/ground assets with performance validation;
- `VehicleVarietyPack` -> support/civilian candidate only unless exact identity is proven; it must never impersonate HMMWV/BTR production identity;
- downloaded shotgun/footstep/vehicle/ambient/fire audio -> mapped only through the correct project-owned audio systems and never falsely labelled as exact recordings when they are generic donors.

## 5. Required integration work after acquisition

### CI-FP-ARMS

- canonical existing Oster arms `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms` are checked first;
- fix camera attachment, owner visibility, transform, materials, skeleton and AnimBP;
- external arms remain a donor until runtime evidence proves a replacement is better;
- weapon-to-hand socket alignment and left-hand IK must be calibrated.

### CI-ANIM

Establish/retarget a complete FPS animation layer for:

- idle;
- walk/sway;
- sprint and sprint recovery;
- ADS enter/idle/exit;
- fire/recoil;
- tactical and empty reload;
- equip/unequip;
- grenade handling;
- M700 bolt;
- Remington pump;
- lever action;
- M249 charge/feed/reload where supported by the actual mesh rig.

No animation pack may replace authoritative gameplay timing.

### CI-HUD

Use `SuperSimpleFPSPack` only as a donor/reference where useful. The Oster HUD remains project-owned. Migrate only useful widgets/materials/icons/dependencies; do not transplant an entire competing game framework.

### CI-INTERACTION

Inventory the Interaction System sample. Reuse only components that do not duplicate existing Oster ownership. Door/pickup/vehicle interaction must have one authoritative runtime owner.

### CI-AUDIO

Build production mappings for:

- weapon reports;
- reload/bolt/pump/lever/charge mechanical audio;
- dry fire and selector actions;
- footsteps by physical surface;
- impact/ricochet surface audio;
- character foley;
- ambient Oster layers;
- vehicle RPM/interior/exterior layers.

Generic donor recordings may not be falsely labelled as an exact firearm or exact vehicle recording.

### CI-VFX

Integrate/select authored Niagara assets for:

- muzzle flash/smoke;
- bullet impacts by surface;
- fire;
- explosions;
- smoke grenade;
- debris/sparks;
- vehicle damage smoke/exhaust where needed.

Primitive sphere/cube smoke or default placeholder effects are forbidden as accepted production presentation.

### CI-WORLD

Select only environment assets that fit photographed Oster references. Generic houses/roads/vegetation may be used as supporting detail only when they do not replace reference-specific landmarks or contradict the location evidence.

## 6. Anti-bloat rules

- Do not ship demo maps merely because a pack includes them.
- Do not keep duplicate 8K textures when a lower production resolution is sufficient.
- Do not keep multiple packs that provide the same role unless each has an explicit use.
- Do not add a second character controller, weapon authority, interaction authority, recoil owner, physics framework or HUD framework.
- Do not migrate unused dependencies.
- No asset may enter production without license/provenance status.
- Any plugin required by selected assets must be explicitly enabled, documented and justified.

## 7. Gates

### Gate CI-0 — provenance

PASS requires source/license/provenance record for every third-party asset selected for production.

### Gate CI-1 — acquisition

PASS requires successful downloader receipt plus factual presence of the required Fab content categories already added to the branch.

### Gate CI-2 — external-project inventory

PASS requires an inventory for any locally present `SuperSimpleFPSPack`, `GameAnimationSample` and Interaction System project. Missing local projects remain explicit pending items.

### Gate CI-3 — migration/integration

PASS requires dependency-aware migration/import and project-owned integration. Merely existing in `Content/` is not enough. Every relevant ledger row must be `INTEGRATED`, `RUNTIME_ACCEPTED`, or `EXCLUDED_WITH_REASON`; candidate-only alternatives may remain unselected.

### Gate CI-4 — runtime presentation

PASS requires local UE 5.8 evidence for:

- visible first-person arms;
- correct weapon grip and ADS;
- working core FPS animations;
- working weapon/mechanical audio;
- footsteps/impacts;
- selected ambient audio;
- smoke/fire/explosion VFX;
- no missing-material/default-placeholder regression.

### Gate CI-5 — performance/size

PASS requires removal or exclusion of unused demo maps/assets, sensible texture resolution, no obvious duplicate pack payload and no material regression in the target runtime profile.

## 8. Completion contract

This TZ is complete only when all of the following are true:

- acquisition receipt exists and verifies;
- required Fab categories are inventoried;
- external projects are inventoried or explicitly marked missing locally;
- `PASS45_CONTENT_INTEGRATION_LEDGER.csv` has no unexplained pending production role;
- selected assets have production paths and owners;
- first-person hands and weapon presentation work in UE 5.8;
- selected animation/audio/VFX content is actually referenced and used by project-owned runtime systems;
- redundant/demo-only assets are excluded from shipping;
- PASS45 runtime evidence accepts the result.

`DOWNLOADED` alone is never a completion state. The executor must continue until content is either **actually used and runtime-accepted** or **explicitly excluded with reason**.
