# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The immediately preceding complete history is preserved byte-for-byte at:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_CURRENT_HEAD_PREFLIGHT_2026-09-02.md`

Preserved Git blob: `798b6b3b35e09cc4a4fce600dcaaf6afce7f9bf6`.

Earlier archives remain preserved at:

- `PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_REMINGTON_MOTION_2026-09-01.md` — blob `99969c3221d26a2d4a9bb372243e9fe68681956a`;
- `PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_2026-09-01.md`.

Git history remains the raw source of truth. This file stays compact so future sessions can resume from the latest factual checkpoint without replaying hundreds of commits.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Target: `main@bca00f4046700f383af9f1742cc24b6a62401b1a` at this checkpoint.
- Latest factual full gameplay runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- Latest substantive item-16 source-hardening head before this ledger update: `114f74acbd2db47a19eea4447ddb7a66b0b7917c`.
- PR #94 must remain unmerged until a **current-head** local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/audio acceptance.
- Official checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source/docs work does not increase it.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items because item 16 still contains content/runtime blockers.

## Binding reuse-first state

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

- Do not stack a second Remington donor unless the primary donor is factually blocked/rejected in isolated UE 5.8 proof.
- Authored discrete weapon actions belong in AnimSequence/Montage timing with IK/Control Rig only as needed; MetaSounds cannot become a second action timer.
- Compile/source green is not runtime acceptance.
- Unknown or incompatible license cannot be promoted.
- The retired procedural whole-weapon/arms manual-action fallback must not return.

## Current item-16 factual state

- M700 exact authored manual-action animation: **CONTENT GAP**.
- Lever Action exact authored manual-action animation: **CONTENT GAP**.
- Remington exact production `.uasset`: absent/unaccepted.
- Primary Remington donor: registered CC-BY-4.0 `8sianDude` GLB, exact SHA-256 / LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`, exact payload size `20,621,580` bytes.
- Third-party register state remains `PILOT / APPROVED_FOR_UE_IMPORT`; `runtime_ready=false`, `ue58_import_pending=true`, `item16_checked=false`.
- Pump audio source fallback remains project-owned `/Game/R13/Audio/shotguncock`.
- Repository-owned CC0 bolt and lever mechanical WAV derivatives remain source payloads pending UE import/runtime wiring.

## 2026-09-02 Remington source truth

Pinned donor evidence proves articulated reload motion and distinct skinned parts, including `PBody_058` and `Pmag_061`. Source transport/relative-motion audits prove non-trivial sibling-relative motion rather than only rigid whole-weapon movement.

Enforced semantic boundary remains:

`ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN`.

Source evidence still does **not** prove that `Pmag_061` is physically the fore-end/pump and does not prove any imported clip is a valid standalone post-shot pump cycle.

## Current-head / exact-donor launcher contract

`OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd` remains fail-closed before UE starts:

- exact canonical branch and current remote canonical HEAD required;
- only tracked pilot-defining files are checked; unrelated local `Changes` are not modified;
- donor size must be exactly `20,621,580` bytes;
- donor SHA-256 must be exactly `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- no checkout, pull, fetch, reset, clean, stash, restore, switch or LFS mutation is performed by the launcher;
- `pump_node_identity=UNPROVEN`, `standalone_pump_clip=UNPROVEN`, `runtime_acceptance=0`, `item16_checked=0` remain mandatory.

## 2026-09-02 factual local UE 5.8 evidence and corrective slices

### A. Legacy `AssetImportTask.async` rejection

The current-head/exact-donor launcher was first run locally against UE `5.8.1-56057345` on canonical head `fbc2d0d48f10bb8e93937d41cc7f700ab150a8f1`. Preflight passed, then UE rejected the base import harness because this engine build no longer exposes the legacy `AssetImportTask.async` editor property:

`AssetImportTask: Failed to find property 'async' for attribute 'async' on 'AssetImportTask'`

That was a pilot-harness compatibility failure, not donor incompatibility.

Corrective commits:

- `b71e734fbb49f161b25d31eab160b132a6c75a16` removes the unsupported `async` property write while preserving no-save/no-production behavior and fail-closed imported-object/skeleton/animation gates;
- `c4d995653c50e4153727c54eae279fde6ae98bfb` guards that UE 5.8 property drift against regression.

### B. UE 5.8.1 animation-track enumeration false negative

The follow-up local UE 5.8 run progressed through actual donor import and animation sampling. The log proves that imported `PBody_058` and `Pmag_061` are addressable by `AnimationLibrary.does_bone_name_exist()` and return changing sampled poses in imported reload/walk sequences. For example, both bones reported `moved=1` in imported `FPS_Pistol_Reload_easy` and `FPS_Pistol_Reload_full`.

However, UE 5.8.1 `AnimationLibrary.get_animation_track_names()` did not expose those exact bone names, so the old pilot recorded `track_present=0` and then rejected with:

`required_weapon_side_tracks_not_preserved=1 bones=PBody_058,Pmag_061`

The same run therefore demonstrated a contradictory harness assumption: the bones were addressable and pose-sampleable, but the diagnostic track-name enumeration omitted them. Relative-motion sampling was skipped only because the old pilot incorrectly used that diagnostic enumeration as an authority gate.

Corrective commits:

- `c662c2bcad9b7eea0b2dc330ffa9335dbc846a2f` makes UE 5.8 bone addressability (`does_bone_name_exist`) authoritative for imported bone presence, keeps `get_animation_track_names()` diagnostic-only, and still requires non-trivial sampled per-bone motion plus sibling-relative motion before PASS;
- `114f74acbd2db47a19eea4447ddb7a66b0b7917c` guards against reintroducing track-enumeration authority while preserving the shared-parent, sampled-motion, relative-motion, no-save and no-runtime-acceptance invariants.

This is still an isolated import/motion pilot. It does not identify the real pump, does not authorize a production asset, and does not close item 16.

## Acceptance state

- Remington donor provenance/source intake: **PASS FOR ISOLATED UE IMPORT**.
- Articulated donor motion: **SOURCE EVIDENCE PASS**.
- Structure/skin separation: **SOURCE EVIDENCE PASS**.
- Source sibling-relative motion: **SOURCE EVIDENCE PASS**.
- Exact donor UE 5.8 import itself: **FACTUALLY REACHED in local UE 5.8.1**.
- Imported `PBody_058` / `Pmag_061` bone addressability: **LOCAL EVIDENCE PASS**.
- Imported per-bone non-trivial motion: **LOCAL EVIDENCE PASS**.
- Imported sibling-parent gate: **PREPARED / previous run progressed beyond it without that failure**.
- Imported sibling-relative motion after the UE 5.8 enumeration correction: **RERUN PENDING**.
- Exact pump/fore-end identity: **UNPROVEN**.
- Standalone pump/manual-action sequence: **UNPROVEN**.
- Production Remington cutover: **NOT AUTHORIZED**.
- Item 16: **UNCHECKED**.
- Full gameplay runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- PR #94: **OPEN / UNMERGED**.
- Local user `Changes`: **NOT TOUCHED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Fast-forward the local canonical checkout to the current canonical head and rerun:

`OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd`

The next run must now prove in UE 5.8 that `PBody_058` / `Pmag_061` remain addressable, preserve the required sibling hierarchy, carry non-trivial per-bone motion, and change relative to one another in at least one imported animation. The diagnostic result of `get_animation_track_names()` is no longer allowed to override direct UE bone addressability plus sampled-motion evidence.

Even a PASS still cannot close item 16 by itself. Direct visual inspection must identify the actual fore-end/pump and determine whether an imported clip is a valid standalone manual-action sequence.

Until that evidence exists: do not populate production `ManualActionAnimationObjectPath`, do not create `/Game/Production/Weapons/Remington870` as accepted content, do not stack a second Remington donor, do not mark item 16 complete, and do not merge PR #94.
