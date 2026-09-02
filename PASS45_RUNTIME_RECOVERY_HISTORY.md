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
- Latest factual isolated UE 5.8 Remington import-pilot verdict: **REJECTED 2026-09-02 on `fbc2d0d48f10bb8e93937d41cc7f700ab150a8f1` due to removed `AssetImportTask.async` editor property**.
- Latest substantive item-16 source-hardening head before this ledger update: `c4d995653c50e4153727c54eae279fde6ae98bfb`.
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

Pinned donor evidence proves articulated reload motion and distinct skinned parts, including `PBody_058` and `Pmag_061`. The later source transport/relative-motion audits also prove that the donor carries non-trivial sibling-relative motion rather than only rigid whole-weapon movement.

Enforced semantic boundary remains:

`ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN`.

In particular, source evidence still does **not** prove that `Pmag_061` is physically the fore-end/pump and does not prove any clip is a valid standalone post-shot pump cycle.

## Current-head / exact-donor launcher hardening

The isolated launcher `OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd` remains fail-closed before UE starts:

- requires the exact canonical branch and current remote canonical HEAD;
- checks only tracked pilot-defining files and does not modify other local `Changes`;
- requires donor size exactly `20,621,580` bytes;
- requires exact SHA-256 `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- performs no checkout, pull, fetch, reset, clean, stash, restore, switch or LFS mutation;
- preserves `pump_node_identity=UNPROVEN`, `standalone_pump_clip=UNPROVEN`, `runtime_acceptance=0`, `item16_checked=0`.

## 2026-09-02 imported sibling-relative motion gate

A remaining source-proof hole was closed without creating a second import owner.

Before this slice the isolated UE imported-motion pilot required `PBody_058` and `Pmag_061` to have named tracks and non-trivial imported motion, but it did not prove that the two parts move **relative to one another** after UE import. A rigid whole-weapon movement could therefore satisfy the old per-bone motion condition.

Implemented commits:

- `11828182d964c75365dffe56951913d07df4518b` — `PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py` now fail-closes unless imported skeletal hierarchy preserves a common parent for `PBody_058` / `Pmag_061` and at least one imported AnimSequence shows non-trivial sibling-relative motion using same-time sampled poses;
- `bac96f3cb096a1c8c7ee5550b734cacb5e2c2d9b` — `VERIFY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.py` guards the shared-parent and imported relative-motion contract against regression.

The pilot still reuses `PASS45_REMINGTON870_UE58_IMPORT_PILOT.py`; it does not create another `AssetImportTask`, save packages, create production Remington content, switch authority, or claim runtime acceptance.

Exact-head GitHub CI for `bac96f3cb096a1c8c7ee5550b734cacb5e2c2d9b` completed successfully. In particular:

- `Pass 45 Remington 870 UE58 imported motion pilot contract` run `33609750258`: **SUCCESS**;
- `Source verification` run `33609749476`: **SUCCESS**;
- `Runtime recovery Pass 45` run `33609750251`: **SUCCESS**;
- `Runtime diagnostics Pass 18` run `33609750178`: **SUCCESS**;
- Remington source intake, donor-motion audit, UE58 import-pilot contract, remote-candidate audit and manual-action audio provenance: **SUCCESS**.

These are source/static checks only. They do not execute the user's local UE 5.8 editor and do not change runtime truth.

## 2026-09-02 factual local UE 5.8 import-pilot rejection and corrective slice

The current-head/exact-donor launcher was run locally against UE `5.8.1-56057345` on canonical head `fbc2d0d48f10bb8e93937d41cc7f700ab150a8f1`. Preflight passed on the exact canonical branch, exact head and exact donor SHA-256, so the failure is valid isolated UE evidence rather than a stale-checkout artifact.

UE then rejected the base import pilot before donor import because `unreal.AssetImportTask` in this UE 5.8.1 build does not expose the legacy editor property named `async`:

`AssetImportTask: Failed to find property 'async' for attribute 'async' on 'AssetImportTask'`

This is an engine/Python API compatibility blocker in the pilot harness, not evidence that the donor itself is incompatible.

Corrective source slice:

- `b71e734fbb49f161b25d31eab160b132a6c75a16` removes the unsupported `task.set_editor_property("async", False)` write from `PASS45_REMINGTON870_UE58_IMPORT_PILOT.py`; the pilot still waits for `import_asset_tasks([task])` to return and then fail-closes on imported-object, skeletal-mesh, animation-count, positive-length and shared-skeleton evidence before any PASS marker;
- `c4d995653c50e4153727c54eae279fde6ae98bfb` updates `VERIFY_PASS45_REMINGTON870_UE58_IMPORT_PILOT.py` so the removed UE 5.8 property cannot silently return while all no-save/no-production/no-runtime-acceptance constraints remain guarded.

No local user `Changes` were modified by this failure investigation or by the remote corrective commits. No production Remington cutover occurred. PR #94 remains unmerged.

## Acceptance state

- Remington donor provenance/source intake: **PASS FOR ISOLATED UE IMPORT**.
- Articulated donor motion: **SOURCE EVIDENCE PASS**.
- Structure/skin separation: **SOURCE EVIDENCE PASS**.
- Source sibling-relative motion: **SOURCE EVIDENCE PASS**.
- Imported sibling-parent/relative-motion proof implementation: **PREPARED / SOURCE-GUARDED**.
- Current-head/exact-donor launcher preflight: **LOCAL PASS on `fbc2d0d4...`**.
- Local UE 5.8 imported-motion execution: **REJECTED on `fbc2d0d4...` by removed `AssetImportTask.async` harness property; corrective source prepared through `c4d99565...`; rerun pending**.
- Exact pump/fore-end identity: **UNPROVEN**.
- Standalone pump/manual-action sequence: **UNPROVEN**.
- Production Remington cutover: **NOT AUTHORIZED**.
- Item 16: **UNCHECKED**.
- Full gameplay runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- PR #94: **OPEN / UNMERGED**.
- Local user `Changes`: **NOT TOUCHED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Fast-forward the local canonical checkout to the new current canonical head and rerun `OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd` with the full pinned Git LFS donor present.

A PASS must prove in UE 5.8 that the imported named weapon-side tracks survive, that `PBody_058` / `Pmag_061` preserve the required sibling hierarchy, and that their relative transform changes non-trivially in at least one imported animation. It still cannot close item 16 by itself. Direct visual inspection must identify the actual fore-end/pump and determine whether an imported clip is a valid standalone manual-action sequence.

Until that evidence exists: do not populate production `ManualActionAnimationObjectPath`, do not create `/Game/Production/Weapons/Remington870` as accepted content, do not stack a second Remington donor, do not mark item 16 complete, and do not merge PR #94.
