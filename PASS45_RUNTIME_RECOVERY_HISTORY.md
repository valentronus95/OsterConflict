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
- Latest substantive item-16 hardening head before this ledger update: `0c60946a841e0c97467c7216bf89c6e59fec2f5e`.
- PR #94 must remain unmerged until a **current-head** local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/audio acceptance.
- Official checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source/docs work does not increase it.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items because item 16 still contains content/runtime blockers.

## Binding reuse-first state

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

- Do not promote or stack a second Remington donor while the registered primary donor still has an unresolved semantic/visual proof path rather than a factual incompatibility rejection.
- Authored discrete weapon actions belong in AnimSequence/Montage timing with IK/Control Rig only as needed; MetaSounds cannot become a second action timer.
- Compile/source green is not runtime acceptance.
- Unknown or incompatible license cannot be promoted.
- The retired procedural whole-weapon/arms manual-action fallback must not return.

## Current item-16 factual state

- M700 exact authored manual-action animation: **CONTENT GAP**.
- Lever Action exact authored manual-action animation: **CONTENT GAP**.
- Remington exact production `.uasset`: absent/unaccepted.
- Primary Remington donor: registered CC-BY-4.0 `8sianDude` GLB, exact SHA-256 / LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`, exact payload size `20,621,580` bytes.
- Third-party register state remains `PILOT / APPROVED_FOR_UE_IMPORT`; `runtime_ready=false`, `item16_checked=false`.
- Pump audio source fallback remains project-owned `/Game/R13/Audio/shotguncock`.
- Repository-owned CC0 bolt and lever mechanical WAV derivatives remain source payloads pending UE import/runtime wiring.

## Registered Remington donor source truth

Pinned donor evidence proves articulated reload motion and distinct skinned parts, including `PBody_058` and `Pmag_061`. Source transport/relative-motion audits prove non-trivial sibling-relative motion rather than only rigid whole-weapon movement.

Enforced semantic boundary remains:

`ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN`.

Source evidence still does **not** prove that `Pmag_061` is physically the fore-end/pump and does not prove any imported clip is a valid standalone post-shot pump cycle.

## Registered donor local UE 5.8 imported-motion proof

The current-head/exact-donor pilot was exercised locally against UE `5.8.1-56057345` after the UE 5.8 track-enumeration correction.

Latest factual result:

`PASS: UE 5.8 preserved named Remington weapon-side tracks and non-trivial imported motion on the current canonical HEAD with the exact pinned donor payload.`

Classification remains deliberately narrower:

`IMPORTED-MOTION PROOF ONLY`.

That PASS supersedes the earlier `RERUN PENDING` state for imported sibling-relative motion. It proves the registered exact donor survives the tested UE 5.8 import/motion path. It still does **not** identify the physical pump/fore-end, validate a standalone post-shot pump sequence, authorize production cutover, prove gameplay presentation/audio, or close item 16.

The earlier UE 5.8.1 `get_animation_track_names()` false negative remains historical evidence only. Direct bone addressability plus sampled non-trivial motion is authoritative; diagnostic track-name enumeration may not override it.

## Separate local MotionLab candidate quarantine

A different local payload named `Remington_870_FREE.glb` has also produced a local MotionLab imported-motion proof with actions including `PumpAction` and `Cube.002Action`.

That payload is **not** the registered 8sianDude donor and may not inherit its provenance, license, hash, node identity, material identity or acceptance state.

Current classification:

`UNREGISTERED_LOCAL_CANDIDATE / IMPORTED-MOTION PROOF ONLY`.

Repository-owned quarantine controls were added in `eb7850da270fdebad830c05744de2cf567f9c8cf` and wired into `RUN_ALL_VERIFY.py` in `d711de892a6fd79e3d55d94f8a79d078fec4fe45`:

- `PASS45_REMINGTON870_LOCAL_CANDIDATE_IDENTITY_AUDIT.py`;
- `VERIFY_PASS45_REMINGTON870_LOCAL_CANDIDATE_QUARANTINE.py`;
- `_DOCS/PASS45_REMINGTON870_LOCAL_CANDIDATE_QUARANTINE_2026-09-02.md`;
- `.github/workflows/pass45-remington870-local-candidate-quarantine.yml`.

Promotion remains blocked until exact payload fingerprint, source URL, creator, license/public-repository permission and content identity are verified. Reuse-first additionally prevents this candidate from silently replacing or stacking beside the primary registered donor while the primary donor is still awaiting visual/semantic proof rather than factual rejection.

## 2026-09-02 quarantine verifier CI correction

Exact head `d711de892a6fd79e3d55d94f8a79d078fec4fe45` exposed one regression in the newly added quarantine verifier:

`AttributeError: 'str' object has no attribute 'as_posix'`

Cause: `SELF = Path(__file__).name` already returns a string, but the allow-list attempted `SELF.as_posix()`.

Corrective commit:

- `0c60946a841e0c97467c7216bf89c6e59fec2f5e` — uses `SELF` directly and changes no production Remington/gameplay content.

Exact-head verification on `0c60946a841e0c97467c7216bf89c6e59fec2f5e`:

- `Source verification #2727` / run `33639013165`: **SUCCESS**;
- `Pass 45 Remington 870 local candidate quarantine #3` / run `33639013111`: **SUCCESS**;
- all returned current-head PASS45/source workflows in that verification set completed **SUCCESS**.

The failed `d711de89...` runs are superseded by this corrected exact-head evidence and remain useful only as regression history.

## Acceptance state

- Registered Remington donor provenance/source intake: **PASS FOR ISOLATED UE IMPORT**.
- Articulated donor motion: **SOURCE EVIDENCE PASS**.
- Structure/skin separation: **SOURCE EVIDENCE PASS**.
- Source sibling-relative motion: **SOURCE EVIDENCE PASS**.
- Exact registered donor UE 5.8 import: **LOCAL EVIDENCE PASS**.
- Imported `PBody_058` / `Pmag_061` bone addressability: **LOCAL EVIDENCE PASS**.
- Imported per-bone non-trivial motion: **LOCAL EVIDENCE PASS**.
- Imported sibling-relative motion after the UE 5.8 enumeration correction: **LOCAL EVIDENCE PASS**.
- Exact pump/fore-end identity: **UNPROVEN**.
- Standalone pump/manual-action sequence: **UNPROVEN**.
- Separate `Remington_870_FREE.glb` provenance/license: **UNPROVEN / QUARANTINED**.
- Production Remington cutover: **NOT AUTHORIZED**.
- Item 16: **UNCHECKED**.
- Full gameplay runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- PR #94: **OPEN / UNMERGED**.
- Local user `Changes`: **NOT TOUCHED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Continue the registered primary donor first. The immediate blocker is no longer imported-motion preservation; it is **visual/semantic pump identity**.

Create/use an isolated UE 5.8 visual-inspection proof that makes the registered donor's candidate moving parts (`PBody_058`, `Pmag_061`) directly distinguishable in rendered/editor evidence while sampling the already-proven imported motion. The proof must determine which physical geometry is the fore-end/pump without renaming a bone based on guesswork.

Only after the physical pump is identified may a candidate imported action/window be evaluated as a factual standalone post-shot manual-action sequence. Production `ManualActionAnimationObjectPath` and `/Game/Production/Weapons/Remington870` remain blocked until that sequence and the production import are accepted.

The quarantined `Remington_870_FREE.glb` is not the next production route. Its next legal operation, if it becomes necessary after a factual primary-donor rejection, is exact local fingerprint capture followed by exact source/creator/license/public-repository verification.

Until these gates pass: do not mark item 16 complete and do not merge PR #94.
