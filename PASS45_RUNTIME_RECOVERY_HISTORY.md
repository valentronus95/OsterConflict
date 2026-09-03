# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`. Git history remains the raw source of truth; this file records the newest factual continuation state so new chats do not replay completed analysis.

## Binding continuation rule — 2026-09-03

Canonical PASS45 continuation protocol:

`_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`

When the user asks to continue from the latest factual/current checkpoint, do **not** restart a full-project audit by default. Reconcile current branch/HEAD/PR/recent commits/CI/history, consume any newer parallel-chat commits, then inspect only the first factual open item and its direct dependencies. Broad re-audit is allowed only under the invalidation conditions recorded in the protocol.

This behavior is now also bound globally by `AGENTS.md` mandatory workflow rule 30, so a fresh chat must not replay accepted PASS45 analysis simply because the conversation context is new.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Parent checkpoint before continuation-rule work: `940a00f7b31c51d05e9b83d83a1cd881f2f814ef`.
- Continuation/calibration source checkpoint: `fea79ea7a25d115d1a6833267bd934880bd58a8d`.
- Five-phase item-16 evidence-chain checkpoint before AGENTS binding: `39ca1c981cba2d54b570b4d1ebad1accfa3b62ec`.
- Rejected local item-16 test head: `cf75b86ce5988ef489f0ef653d3f1b3f637278fd`.
- M700 UE 5.8 bone-curve recovery: `89bb635d67b24afdb5e32bccd91092401b6024d6`.
- Lever UE 5.8 bone-curve recovery: `3dc5d1b57a6b908b0bd5356e0b01b681e397d285`.
- Narrow continuation/calibration contract: **SUCCESS**.
- Five-phase item-16 evidence-chain source contract: **SUCCESS**.
- Exact-head GitHub Actions on pre-documentation recovery head `3dc5d1b5...`: **SUCCESS** across the returned workflow set; this is structural/source evidence only.
- Official canonical checklist remains **22/36 = 61.1% complete, 38.9% remaining**.
- Local user `Changes`, including local `PC_TEST/TEST_RESULTS` worktree differences, remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Source/docs/intake/CI/pilot-only work does not check item 16 and does not increase the official percentage.

## Item 16 — factual source boundary

### Remington 870

The current source has the guarded registered CC-BY-4.0 donor derivative, production skeletal path and PumpCycle bridge. The last direct user playtest on 2026-09-02 rejected the **pre-cutover** presentation because the fore-end did not visibly pump. The later current-head path therefore still requires new direct visual/audio acceptance.

### M700

The committed Stein CC0 source has a factual weighted `BOLT` joint. `BOLT_STOP` is separate weighted geometry and is explicitly rejected as an authored bolt-travel endpoint. The bounded pilot travel is calibration-only, source-authored endpoint is false, bolt rotation remains pending and production cutover is false.

### Lever Action

The committed Stein CC0 source has an addressable `LEVER` bone but no source-authored lever endpoint. The current `-45°` local-X excursion is calibration-only, not accepted production motion.

Therefore the next factual M700/Lever gate remains:

**MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION BEFORE PRODUCTION AUTHORING**.

Inventing final travel/rotation/angle values from source-only heuristics is prohibited.

## One-shot item-16 local evidence chain — five phases

Canonical local launcher:

`OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

It performs, fail-closed, in one local command:

1. M700 bounded BOLT translation UE 5.8 pilot;
2. Remington 870 derived pump + assembly UE 5.8 proof;
3. Lever Action LEVER UE 5.8 pilot;
4. Bolt/Lever manual-action audio import + fresh-load;
5. M700/Lever calibration evidence consolidation.

Phase 5 uses:

- `PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py`;
- `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`.

It reads factual pilot JSON and records pilot travel/angle in ignored `PC_TEST/TEST_RESULTS/`. It does not select final production values, save production animation packages, run full gameplay, close item 16 or permit merge.

The chain preserves:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

## 2026-09-03 local item-16 UE 5.8 rejection and current recovery

The newest supplied local UE evidence ran the five-phase chain on:

`cf75b86ce5988ef489f0ef653d3f1b3f637278fd`

It failed closed at **phase 1/5 M700** with:

```text
PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_FAIL bolt_bone_track_creation_failed=1
ERROR: item-16 evidence chain stopped at M700. rc=36
```

Because the chain stopped at M700, that run produced **no factual verdict** for the later Remington, Lever, audio-import or calibration-review phases.

Root cause is bounded to transient animation-authoring compatibility in UE 5.8: the legacy `AnimationDataController.add_bone_track()` path returned an invalid result for the imported BOLT bone. The failure does not justify changing weapon gameplay timing, source donor identity, final bolt travel, or production content.

Two narrow source recovery commits now supersede the rejected local head:

- `89bb635d67b24afdb5e32bccd91092401b6024d6` replaces the M700 pilot's legacy bone-track creation with UE 5.8 `add_bone_curve()` while retaining `set_bone_track_keys()` and the exact 1.10 s bounded proof cadence;
- `3dc5d1b57a6b908b0bd5356e0b01b681e397d285` applies the same UE 5.8 bone-curve creation compatibility to the Lever pilot while retaining its exact 0.85 s bounded proof cadence.

These are proof-only compatibility corrections. They do not accept M700 travel, invent bolt rotation, accept the Lever angle, save production M700/Lever animation packages, close item 16, or permit merge.

The next local run must therefore use a branch fast-forwarded beyond the rejected `cf75b86c...` state. Do not diagnose the old `bolt_bone_track_creation_failed=1` as still current unless it reproduces on the new recovery head.

## Weapon runtime cadence

Do **not** run the expensive full UE 5.8 gameplay acceptance after every small weapon tweak.

Required cadence:

1. bounded source/import/fresh-load/calibration checks while configuring individual weapons;
2. finish the intended weapon setup set;
3. run one consolidated current-head weapon runtime acceptance across that set;
4. retain direct visual/audio/gameplay evidence;
5. close checklist items only from factual accepted results.

## 2026-09-03 asset quarantine — remote audit complete

The downloaded batch remains quarantined on `asset-intake-20260903`, never wholesale merged into production PASS45.

The **remote quarantine audit** completed successfully after selectively hydrating only `models_game_OC/**` LFS payloads:

- archives: **18**;
- compressed bytes: **3,607,118,270**;
- exact duplicate groups: **0** after earlier Kar98k duplicate cleanup;
- unsafe/rejected archives: **0**;
- `AUDITABLE_CANDIDATE`: **4**;
- `NEEDS_PROVENANCE`: **10**;
- `NO_MODEL_PAYLOAD` at outer ZIP layer: **4**.

Immediately auditable: M1911, M72, sardines prop, telephone-pole asset. Ten still need exact provenance; four outer archives need recursive nested-payload inspection. Quarantine does not preempt item 16 and creates no checklist items 37+.

## Binding reuse-first / legal sequence

`quarantine -> SHA/duplicate/safe-ZIP audit -> model/texture/rig/animation inventory -> exact provenance/license/public-repo permission -> quality audit -> REJECT/DONOR_ONLY/PRODUCTION_CANDIDATE -> selective SOURCE_ASSETS promotion -> individual third-party register record -> isolated UE 5.8 proof -> integration -> cutover -> obsolete-owner cleanup -> regression/runtime acceptance`

Unknown source/license remains fail-closed.

## 2026-09-03 stale literal-marker CI fix

On exact head `91b97305529e9b38559f7fa843d7f216fe7dc31e`, the asset-intake contract failed twice only because `VERIFY_PASS45_ASSET_INTAKE_20260903.py` required three obsolete literal sentences from the living history file. Runtime/source behavior was not the failure.

The verifier has been corrected to validate current semantic invariants instead of frozen wording:

- bounded checks before consolidated weapon runtime;
- PR #94 remains `OPEN / UNMERGED`;
- frozen 36-item architecture, with no checklist items 37+.

Do not restore stale history wording merely to satisfy an old verifier. This is an application of `AGENTS.md` rule 19: verifier truth follows current behavior, not history.

## Next factual operation

Do **not** run the full gameplay runtime yet.

On a local PASS45 checkout fast-forwarded to the newest canonical branch head, run only:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The chain must clear all five phases. After it passes, use the report plus direct current-head UE 5.8 visual observation to determine factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences, finish the intended weapon setup batch and run one consolidated full weapon runtime acceptance.

PR #94 remains OPEN / UNMERGED.

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```
