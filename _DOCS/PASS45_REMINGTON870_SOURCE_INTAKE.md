# PASS45 Remington 870 source-intake contract

Date: 2026-09-01
Parent: `PASS45_RUNTIME_RECOVERY_TZ.md` item 16
State: `SOURCE CANDIDATE AUDITED / BINARY NOT ACQUIRED / UE 5.8 RUNTIME UNACCEPTED`

## Why this contract exists

The canonical branch declares `/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870`, but the corresponding tracked production package is absent. Item 16 also requires real authored first-person manual-action presentation, so a static shotgun mesh is not a valid closeout merely because its filename says Remington 870.

This contract makes the intake fail closed. A future Remington production package may not be treated as accepted unless its exact source, license, redistribution state, binary hashes, rig/articulation evidence and intended animation coverage are recorded before runtime promotion.

## Reuse-first audit result

### Preferred currently-audited candidate — NOT ACQUIRED

- Name: `Remington 870 Shotgun - Animated`
- Publisher/uploader: `user77`
- Source URL: `https://sketchfab.com/3d-models/remington-870-shotgun-animated-1b6c11ef58904fab992c6cdffaada309`
- Source model id: `1b6c11ef58904fab992c6cdffaada309`
- License advertised by source: Creative Commons Attribution / CC BY
- Source page credits weapon model to Andrei Milin and arms/animations to DJMaesen.
- Source metadata advertises the model as downloadable, rigged and animated.
- Intake state: `NOT ACQUIRED`.
- Binary SHA-256: `PENDING`.
- Public-repository redistribution review: `PENDING EXACT DOWNLOADED PACKAGE REVIEW`.
- UE 5.8 skeleton/import/fresh-load evidence: `PENDING`.
- First-person ironsight/fire/reload/dry-fire compatibility: `PENDING`.

The candidate is therefore suitable for acquisition/testing, not for a READY claim.

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
- a production `.uasset` without the matching manifest -> CI fail;
- manifest presence does not imply runtime acceptance;
- `runtime_ready` and `item16_checked` remain false until current-head local UE 5.8 import/build/gameplay/direct first-person acceptance proves the final result;
- final animation slots remain governed by the canonical animation-profile/runtime gates. This intake contract does not create a second gameplay/action authority.

## Current factual verdict

`accepted_remington870_source=0 tracked_production_package=0 ue58_runtime_acceptance=0 item16_checked=0`
