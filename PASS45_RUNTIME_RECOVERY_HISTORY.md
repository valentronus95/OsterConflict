# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

Git history remains the raw source of truth. This live file intentionally records the newest factual continuation state needed to resume work without replaying already completed analysis.

## Binding continuation rule — 2026-09-03

The PASS45-specific continuation protocol is now binding:

`_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`

When the user asks to continue PASS45 from the last factual/current checkpoint, do **not** restart a full-project audit by default. Reconcile current branch/HEAD/PR/recent commits/CI/history, consume any newer parallel-chat commits, then inspect only the first factual open item and its direct dependency surface. A broad re-audit requires one of the explicit invalidation conditions in the protocol.

This rule is an efficiency rule, not a relaxation of acceptance. Runtime truth, reuse-first policy, legal provenance, branch hygiene and current-head evidence requirements remain unchanged.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Parent factual checkpoint before this continuation update: `940a00f7b31c51d05e9b83d83a1cd881f2f814ef`.
- Exact-head CI for that parent checkpoint was fully green before this continuation.
- PR #94 must remain unmerged until current-head local UE 5.8 runtime acceptance and remaining branch-hygiene gates pass.
- Official canonical checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- Local user `Changes`, including the two local `PC_TEST/TEST_RESULTS/*.zip` worktree differences, remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Source/docs/intake/CI/pilot-only work does not check item 16 and does not increase the official percentage.

## Item 16 — current factual source boundary

### Remington 870

The guarded current source contains the registered CC-BY-4.0 Remington donor derivative, production skeletal path and PumpCycle bridge. The last direct user gameplay evidence from 2026-09-02 rejected the **pre-cutover** presentation because the fore-end did not visibly pump. That observation does not accept or reject the later current-head production path; current-head direct visual/audio acceptance remains pending.

### M700

The committed Stein CC0 M700 source has a factual weighted `BOLT` joint. Source audit also proves `BOLT_STOP` is separate weighted geometry and must **not** be treated as an authored bolt-travel endpoint.

Current bounded pilot:

`OsterConflict/TRY_PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.cmd`

The pilot proves only that UE 5.8 can preserve/address `BOLT` and play a bounded non-trivial translation. Its travel is deliberately calibration-only, source-authored endpoint is false, bolt rotation remains pending, production cutover is false, and item 16 remains open.

### Lever Action

The committed Stein CC0 Lever Action source has a factual addressable `LEVER` bone but no source-authored lever endpoint.

Current bounded pilot:

`OsterConflict/TRY_PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.cmd`

The current `-45°` local-X excursion is explicitly a calibration pilot only. It is not accepted production motion and does not close item 16.

### One-shot local evidence chain

Canonical bounded launcher:

`OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

It covers M700, Remington 870, Lever Action and manual-action audio proof while preserving:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

## New calibration evidence review — 2026-09-03

To avoid repeating the M700/Lever source investigation after each new chat, current source now includes a bounded review step:

- `PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py`;
- `OsterConflict/REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`.

The review consumes the local UE 5.8 pilot JSON produced by M700 and Lever Action, verifies that all fail-closed/non-acceptance markers remain intact, and writes a small consolidated report under ignored `PC_TEST/TEST_RESULTS/`.

It does **not** choose final M700 travel/rotation, choose the final Lever angle, author/save production packages, run full gameplay, close item 16, or permit merge.

The next factual M700/Lever gate is therefore:

**MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION BEFORE PRODUCTION AUTHORING**.

Inventing final motion values from source-only heuristics is prohibited because neither source provides a factual authored endpoint.

## Weapon runtime cadence

Do **not** run the expensive full UE 5.8 gameplay acceptance after every small weapon tweak.

Required cadence:

1. bounded source/import/fresh-load/calibration checks while each weapon is configured;
2. finish the intended weapon setup set;
3. run one consolidated current-head weapon runtime acceptance across that configured set;
4. retain direct visual/audio/gameplay evidence;
5. close checklist items only from factual accepted results.

This changes test cadence only, not acceptance strictness.

## 2026-09-03 downloaded asset quarantine — remote audit complete

The user-downloaded batch remains quarantined on `asset-intake-20260903`, not merged into production PASS45.

The **remote quarantine audit** completed successfully on GitHub Actions after selectively hydrating only `models_game_OC/**` LFS payloads.

Factual result:

- archives: **18**;
- compressed bytes audited: **3,607,118,270**;
- exact duplicate groups: **0** after earlier Kar98k duplicate cleanup;
- unsafe/rejected archives: **0**;
- `AUDITABLE_CANDIDATE`: **4**;
- `NEEDS_PROVENANCE`: **10**;
- `NO_MODEL_PAYLOAD` at outer ZIP layer: **4**.

The four immediately auditable candidates are the M1911, M72, sardines prop and telephone-pole asset because their archives include model payload plus source/license clues. Ten candidates still need exact provenance resolution. Four outer archives need recursive/nested-payload inspection before any conclusion about their model contents.

This quarantine work does not preempt item 16, does not create checklist items 37+, and does not change runtime acceptance.

## Binding reuse-first / legal sequence

For external candidates:

`quarantine -> SHA/duplicate/safe-ZIP audit -> model/texture/rig/animation inventory -> exact provenance/license/public-repo permission -> quality audit -> REJECT/DONOR_ONLY/PRODUCTION_CANDIDATE -> selective SOURCE_ASSETS promotion -> individual third-party register record -> isolated UE 5.8 proof -> integration -> cutover -> obsolete-owner cleanup -> regression/runtime acceptance`

Unknown source/license remains fail-closed.

## Next factual operation

Do **not** run the full gameplay runtime yet.

On the current canonical head:

1. run `OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd` in local UE 5.8;
2. if its M700 and Lever pilots pass, run `OsterConflict\REVIEW_PASS45_ITEM16_M700_LEVER_CALIBRATION.cmd`;
3. use the generated calibration review plus direct UE visual observation to determine factual M700 travel/rotation and Lever angle;
4. only then author/cut over accepted production M700/Lever sequences;
5. finish the intended weapon setup batch;
6. run one consolidated current-head weapon runtime acceptance;
7. keep PR #94 OPEN / UNMERGED until all required runtime/hygiene gates pass.

Current formal state:

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```
