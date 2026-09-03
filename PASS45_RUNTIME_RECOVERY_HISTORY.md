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
- Narrow continuation/calibration contract on `39ca1c98...`: **SUCCESS**.
- Five-phase item-16 evidence-chain contract on `39ca1c98...`: **SUCCESS**.
- Wider exact-head CI may still be running after a new governance commit and must not be called fully green until final conclusions exist.
- Official canonical checklist remains **22/36 = 61.1% complete, 38.9% remaining**.
- Local user `Changes`, including two local `PC_TEST/TEST_RESULTS/*.zip` worktree differences, remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Source/docs/intake/CI/pilot-only work does not check item 16 and does not increase the official percentage.

## Item 16 — factual source boundary

### Remington 870

The current source has the guarded registered CC-BY-4.0 donor derivative, production skeletal path and PumpCycle bridge. The last direct user playtest on 2026-09-02 rejected the **pre-cutover** presentation because the fore-end did not visibly pump. The later current-head path therefore still requires new direct visual/audio acceptance.

### M700

The committed Stein CC0 source has a factual weighted `BOLT` joint. `BOLT_STOP` is separate weighted geometry and is explicitly rejected as an authored bolt-travel endpoint. The current bounded pilot proves UE 5.8 can preserve/address `BOLT` and play a non-trivial translation, but the travel is calibration-only, source-authored endpoint is false, bolt rotation remains pending and production cutover is false.

### Lever Action

The committed Stein CC0 source has an addressable `LEVER` bone but no source-authored lever endpoint. The current `-45°` local-X excursion is calibration-only, not accepted production motion.

Therefore the next factual M700/Lever gate is:

**MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION BEFORE PRODUCTION AUTHORING**.

Inventing final travel/rotation/angle values from source-only heuristics is prohibited.

## One-shot item-16 local evidence chain — now five phases

Canonical local launcher:

`OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

It now performs, fail-closed, in one local command:

1. M700 bounded BOLT translation UE 5.8 pilot;
2. Remington 870 derived pump + assembly UE 5.8 proof;
3. Lever Action LEVER UE 5.8 pilot;
4. Bolt/Lever manual-action audio import + fresh-load;
5. M700/Lever calibration evidence consolidation.

Phase 5 uses:

- `PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py`;
- `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`.

It reads the factual pilot JSON and records pilot travel/angle in ignored `PC_TEST/TEST_RESULTS/`. It does not select final production values, save production animation packages, run full gameplay, close item 16 or permit merge.

The chain preserves:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

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

## Next factual operation

Do **not** run the full gameplay runtime yet.

On the newest canonical head, run only:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The chain itself now creates the M700/Lever calibration review after its four proof/import phases. After it passes, use the report plus direct current-head UE 5.8 visual observation to determine factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences, finish the intended weapon setup batch and run one consolidated full weapon runtime acceptance.

PR #94 remains OPEN / UNMERGED.

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```
