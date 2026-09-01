# PASS45 Remington 870 source-intake contract

Date: 2026-09-01
Parent: `PASS45_RUNTIME_RECOVERY_TZ.md` item 16
State: `OSTER SOURCE ACQUIRED / UE 5.8 IMPORT PENDING / RUNTIME UNACCEPTED`

## Why this contract exists

The canonical branch declares `/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870`, but the corresponding tracked production package is still absent. Item 16 also requires real authored first-person manual-action presentation, so a static shotgun mesh is not a valid closeout merely because its filename says Remington 870.

This contract makes the intake fail closed. A future Remington production package may not be treated as accepted unless its exact source, license, redistribution state, binary identity, rig/articulation evidence and intended animation coverage are recorded before runtime promotion.

## Reuse-first audit result

### Preferred directly-auditable candidate — OSTER SOURCE ACQUIRED / APPROVED FOR UE IMPORT / NOT RUNTIME ACCEPTED

- Candidate repository: `Parking-Master/FPS`.
- Exact source commit: `ed07ea542111c2149c5dab735e752824d0b0541c`.
- Exact repository path: `models/weapons/shotgun.glb`.
- Git blob SHA-1: `f822d184d96ede43d79a6f691d69cbe7cf60e686`.
- Exact source size: `20621580` bytes.
- Upstream model credited by that repository: `Remington 870` by `8sianDude`.
- Upstream Sketchfab model id: `eea11de7e9d24b6683962b8388c319eb`.
- Upstream source URL: `https://sketchfab.com/3d-models/remington-870-eea11de7e9d24b6683962b8388c319eb`.
- License: Creative Commons Attribution 4.0 / `CC-BY-4.0`.
- Required attribution: `Remington 870 by 8sianDude, licensed under CC BY 4.0`.
- The exact repository source maps `shotgun` to `models/weapons/shotgun.glb`, creates a `THREE.AnimationMixer` for the loaded scene and stores `gun.animations`.
- The same exact source consumes animation index `2` for fire, index `3` for ordinary/easy reload and index `4` for full/empty reload; shotgun is not excluded from those fire/reload paths.
- Dedicated remote audit owner: `PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT.py`.
- Dedicated remote workflow: `.github/workflows/pass45-remington870-remote-candidate-audit.yml`.
- The remote audit proved GLB 2.0 structure, at least five animation clips, non-empty fire/easy-reload/full-reload channels and at least one glTF skin before acquisition.
- Acquisition owner: `PASS45_REMINGTON870_SOURCE_ACQUIRE.py`.
- Acquisition workflow: `.github/workflows/pass45-remington870-source-acquire.yml`.
- Repository-owned source file: `SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb` through Git LFS.
- Repository manifest: `SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json`.
- Acquisition commit: `177285c68fd693ff1570f3025fae5890128eae17`.
- Acquired source SHA-256 / Git LFS OID: `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`.
- Acquired source size: `20621580` bytes.
- Donor probe recorded in the manifest: 5 animations, 6 meshes, 109 nodes and 4 skins.
- Proven donor action channels recorded in the manifest: fire index 2 -> 71 channels, easy reload index 3 -> 71 channels, full reload index 4 -> 72 channels.
- This proves a pinned, articulated/rigged, animation-capable Remington 870 source donor is now owned by Oster for UE import. It does **not** prove that the donor hierarchy imports into UE 5.8 as a compatible production skeletal/moving-part setup or that its donor clips are acceptable first-person pump presentation.
- Oster intake state: `SOURCE_ACQUIRED_APPROVED_FOR_UE_IMPORT`.
- Tracked production `.uasset`: `ABSENT`.
- UE 5.8 import/fresh-load/skeleton compatibility: `PENDING`.
- Clip mapping/retargeting and moving-pump integration: `PENDING`.
- Materials and first-person fit: `PENDING`.
- Direct first-person pump/action visual acceptance: `PENDING`.
- Runtime state remains `runtime_ready=false`, `ue58_import_pending=true`, `item16_checked=false`.

A green remote/source-intake audit is structural/provenance evidence only. It does not create `SM_Remington870.uasset`, populate Oster animation slots, or close item 16.

### Secondary animated candidate — NOT ACQUIRED

- Name: `Remington 870 Shotgun - Animated`.
- Publisher/uploader: `user77`.
- Source URL: `https://sketchfab.com/3d-models/remington-870-shotgun-animated-1b6c11ef58904fab992c6cdffaada309`.
- Source model id: `1b6c11ef58904fab992c6cdffaada309`.
- License advertised by source: Creative Commons Attribution / CC BY.
- Source page credits weapon model to Andrei Milin and arms/animations to DJMaesen.
- Source metadata advertises the model as downloadable, rigged and animated.
- Intake state: `NOT ACQUIRED`.
- Binary SHA-256: `PENDING`.
- Public-repository redistribution review: `PENDING EXACT DOWNLOADED PACKAGE REVIEW`.
- UE 5.8 skeleton/import/fresh-load evidence: `PENDING`.
- First-person ironsight/fire/reload/dry-fire compatibility: `PENDING`.

The candidate remains a fallback acquisition path only. It must not be fetched or stacked beside the accepted primary donor unless a factual blocker invalidates the primary donor.

### Evaluated static/base candidates — NOT ACCEPTED FOR ITEM 16 CLOSEOUT

1. Andrei Milin `Remington 870 Shotgun`, source model id `6db0ad4764d14eee8f063eea3600071b`, advertised CC Attribution. It is a valid traceable geometry source candidate, but the reviewed source metadata does not establish the exact authored first-person action coverage required by item 16.
2. Public GitHub mirror `openfw-game/defy` at commit `d33cef14f47b054845f9f447249dfd412a51163b` tracks `assets/weapons/remington_870_shotgun.glb` plus textures and credits the Andrei Milin source. Repository presence proves transport/provenance availability, not production skeletal/animation acceptance.
3. Public text glTF copy audited in `perk3greed/bo2` at commit `3a4fb99a3dfb19a8dfdbb73a0ecafb6089723797` contains a CC-BY source attribution and separate `scene.bin`; the reviewed shotgun glTF did not provide accepted item-16 authored animation evidence.

None of those static/base copies may be silently substituted for a skeletal/manual-action-ready production package.

## Mandatory manifest before production-package introduction

If `OsterConflict/Content/Production/Weapons/Remington870/SM_Remington870.uasset` is introduced, the same accepted tree must also contain:

`SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json`

Minimum manifest contract:

```json
{
  "schema": 1,
  "weapon": "Remington870",
  "status": "APPROVED_FOR_UE_IMPORT",
  "source_name": "exact source title",
  "source_url": "https://...",
  "source_model_id": "exact upstream id/version",
  "license_id": "exact reviewed license",
  "license_url": "https://...",
  "attribution": "required attribution text or NONE",
  "public_repo_allowed": true,
  "source_sha256": "64 lowercase hex chars",
  "rigged_or_articulated": true,
  "animation_capable": true,
  "intended_fp_clips": ["ironsight", "fire", "reload", "dryfire"],
  "derivative_notes": "what Oster changed",
  "runtime_ready": false,
  "ue58_import_pending": true,
  "item16_checked": false
}
```

Rules:

- unknown/restrictive/redistribution-incompatible license -> reject;
- missing exact source/version/hash -> reject;
- static-only geometry presented as completed skeletal/manual-action content -> reject;
- source page or URL without acquired bytes -> not imported;
- remote Git blob pin is useful acquisition evidence but is not an Oster-owned `source_sha256` record;
- an acquired third-party source must have a corresponding actual-import record in `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` before production use;
- a production `.uasset` without the matching manifest -> CI fail;
- manifest presence does not imply runtime acceptance;
- `runtime_ready` and `item16_checked` remain false until current-head local UE 5.8 import/build/gameplay/direct first-person acceptance proves the final result;
- final animation slots remain governed by the canonical animation-profile/runtime gates. This intake contract does not create a second gameplay/action authority;
- do not fetch or integrate a second Remington donor merely to accumulate alternatives. The accepted primary donor must first be proved or factually rejected in isolation, per reuse-first replace-not-stack policy.

## Current factual verdict

`remote_animated_candidate_pinned=1 oster_source_bytes_acquired=1 accepted_remington870_source_for_ue_import=1 tracked_production_package=0 ue58_import_pending=1 ue58_runtime_acceptance=0 item16_checked=0`
