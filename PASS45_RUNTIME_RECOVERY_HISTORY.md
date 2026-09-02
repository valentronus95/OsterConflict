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
- Latest factual local verdict: **RUNTIME REJECTED 2026-08-31**.
- Latest substantive item-16 source-hardening head before this ledger rotation: `4909dcc3145a75340ba33688457beeb1f3bff43c`.
- PR #94 must remain unmerged until a **current-head** local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/audio acceptance.
- Official checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source/docs work does not increase it.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items because item 16 contains content/runtime blockers.

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
- Donor state remains `APPROVED_FOR_UE_IMPORT`; `runtime_ready=false`, `ue58_import_pending=true`, `item16_checked=false`.
- Pump audio source fallback remains project-owned `/Game/R13/Audio/shotguncock`.
- Repository-owned CC0 bolt and lever mechanical WAV derivatives remain source payloads pending UE import/runtime wiring.

## 2026-09-02 Remington structure/skin truth

Pinned donor structure/skin evidence is persisted at `_DOCS/PASS45_REMINGTON870_STRUCTURE_AUDIT_2026-09-02.json`.

Key factual result:

- `Pmag_061` is a real separate animated/skinned part, not an empty name: joint slot 79 controls distinct skinned mesh `Object_95`, 4,411 vertices at full weight;
- reload motion reaches `0.606719` donor units with dominant Y translation;
- `PBody_058` separately controls `Object_91`, 7,318 vertices at full weight;
- this still does **not** prove that `Pmag_061` is the physical fore-end/pump and does not prove any imported clip is a standalone post-shot pump cycle.

Enforced status remains:

`ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN`.

Source guard: `VERIFY_PASS45_REMINGTON870_STRUCTURE_AUDIT.py`, bound into `RUN_ALL_VERIFY.py`.

## 2026-09-02 current-head / exact-donor pilot hardening

A source audit found that `OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd` previously checked only that the donor file existed and was larger than a threshold. It did not prove that the local checkout matched the current remote canonical HEAD or that the donor bytes matched the registered payload. That could create stale-head or payload-drift evidence.

Implemented commits:

- `cc900dfa569a1537f5ec8b5ff16cd1a61abdb447` — fail closed on stale canonical HEAD or donor drift;
- `bc682751d6548b38b1e5649282b377bbf0761b8b` — guard the new preflight contract in the source verifier;
- `4909dcc3145a75340ba33688457beeb1f3bff43c` — align the verifier with the final launcher wording.

The launcher now performs read-only preflight before invoking UE 5.8:

- requires exact canonical branch;
- compares local `HEAD` with `git ls-remote origin refs/heads/<canonical>` and fails closed if stale or unreachable;
- checks only the tracked files that define the isolated pilot and aborts if they differ from HEAD without modifying them;
- requires donor size exactly `20,621,580` bytes;
- computes SHA-256 with `Get-FileHash` and requires exact `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- performs no checkout, pull, fetch, reset, clean, stash, restore, switch or LFS mutation;
- still preserves `pump_node_identity=UNPROVEN`, `standalone_pump_clip=UNPROVEN`, `runtime_acceptance=0`, `item16_checked=0`.

Exact-head source CI for `4909dcc3145a75340ba33688457beeb1f3bff43c` completed successfully. In particular:

- `Pass 45 Remington 870 UE58 imported motion pilot contract` run `33579145722`: **SUCCESS**;
- `Source verification` run `33579144955`: **SUCCESS**;
- `Runtime recovery Pass 45` run `33579145881`: **SUCCESS**;
- Remington source intake and donor-motion audit: **SUCCESS**;
- exact-head check-runs had no queued, in-progress or failed checks when this checkpoint was written.

These are source checks only. They do not change runtime truth.

## Acceptance state

- Remington donor provenance/source intake: **PASS FOR ISOLATED UE IMPORT**.
- Articulated donor motion: **SOURCE EVIDENCE PASS**.
- Structure/skin separation evidence: **SOURCE EVIDENCE PASS**.
- Imported named-track/motion proof implementation: **PREPARED / SOURCE-GUARDED**.
- Current-head/exact-donor launcher preflight: **SOURCE-GUARDED**.
- Local UE 5.8 imported-motion execution: **PENDING**.
- Exact pump/fore-end identity: **UNPROVEN**.
- Standalone pump/manual-action sequence: **UNPROVEN**.
- Production Remington cutover: **NOT AUTHORIZED**.
- Item 16: **UNCHECKED**.
- Runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- PR #94: **OPEN / UNMERGED**.
- Local user `Changes`: **NOT TOUCHED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Run `OsterConflict/TRY_PASS45_REMINGTON870_UE58_IMPORTED_MOTION_PILOT.cmd` on the **current canonical checkout** with the full pinned Git LFS donor present. The launcher now independently rejects a stale checkout or donor-byte drift before UE starts.

A PASS may prove that UE 5.8 preserved the named weapon-side tracks and non-trivial imported motion. It still cannot close item 16 by itself. Direct visual inspection must then identify the actual fore-end/pump and determine whether an imported clip is a valid standalone manual-action sequence.

Until that evidence exists: do not populate production `ManualActionAnimationObjectPath`, do not create `/Game/Production/Weapons/Remington870` as accepted content, do not stack a second Remington donor, do not mark item 16 complete, and do not merge PR #94.
