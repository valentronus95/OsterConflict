# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`. Git history remains the raw source of truth; this file records the newest factual continuation state so new chats do not replay completed analysis.

## Binding continuation rule — 2026-09-03

Canonical PASS45 continuation protocol:

`_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md`

When the user asks to continue from the latest factual/current checkpoint, do **not** restart a full-project audit by default. Reconcile current branch/HEAD/PR/recent commits/CI/history, consume any newer parallel-chat commits, then inspect only the first factual open item and its direct dependencies. Broad re-audit is allowed only under the invalidation conditions recorded in the protocol.

This behavior is also bound globally by `AGENTS.md` mandatory workflow rule 30.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Rejected local M700 API test head: `cf75b86ce5988ef489f0ef653d3f1b3f637278fd`.
- M700 UE 5.8 bone-curve recovery: `89bb635d67b24afdb5e32bccd91092401b6024d6`.
- Lever UE 5.8 bone-curve recovery: `3dc5d1b57a6b908b0bd5356e0b01b681e397d285`.
- Bone-curve regression guard: `3de85c46a9c12aa9dd43a3950a888872cf266e6f`.
- Lever UE 5.8 DDC/asset-compilation teardown recovery: `7b70c56e0c1e77c6642ba517d45310d7879be343`.
- M700 matching asset-compilation teardown policy: `2ac5b9560b63a51be3f57c770c6a93d2c302373c`.
- Async-compilation barrier regression guard: `03ab7bded49fc23ea1c19c23586b86797aaeba93`.
- Latest live-ledger checkpoint after the Lever crash: `5a492238a9ee7f490175dbe3213ec6eb04289341`.
- Official canonical checklist remains **22/36 = 61.1% complete, 38.9% remaining**.
- Local user `Changes` remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Source/docs/intake/CI/pilot-only work does not check item 16 and does not increase the official percentage.

## Item 16 — factual source boundary

### Remington 870

The current source has the guarded registered CC-BY-4.0 donor derivative, production skeletal path and PumpCycle bridge. The last direct user playtest on 2026-09-02 rejected the **pre-cutover** presentation because the fore-end did not visibly pump. The later current-head path still requires new direct visual/audio acceptance.

### M700

The committed Stein CC0 source has a factual weighted `BOLT` joint. `BOLT_STOP` is separate weighted geometry and is explicitly rejected as an authored bolt-travel endpoint. The bounded pilot travel is calibration-only, source-authored endpoint is false, bolt rotation remains pending and production cutover is false.

### Lever Action

The committed Stein CC0 source has an addressable `LEVER` bone but no source-authored lever endpoint. The current `-45°` local-X excursion is calibration-only, not accepted production motion.

## One-shot item-16 local evidence chain

Canonical local launcher:

`OsterConflict/RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

It performs, fail-closed:

1. M700 bounded BOLT translation UE 5.8 pilot;
2. Remington 870 derived pump + assembly UE 5.8 proof;
3. Lever Action LEVER UE 5.8 pilot;
4. Bolt/Lever manual-action audio import + fresh-load;
5. M700/Lever calibration evidence consolidation.

It preserves:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

## 2026-09-03 local M700 API rejection — superseded blocker

A factual local run on `cf75b86ce5988ef489f0ef653d3f1b3f637278fd` failed in phase 1/5 M700 with:

```text
PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_FAIL bolt_bone_track_creation_failed=1
ERROR: item-16 evidence chain stopped at M700. rc=36
```

That failure was bounded to the deprecated/invalid UE 5.8 `add_bone_track()` creation path and is superseded by the M700/Lever `add_bone_curve()` recovery commits listed above.

Do not return to the old API merely because a historical verifier once accepted it.

## 2026-09-03 latest local Lever/DDC rejection

The newest user screenshot is from a later bounded rerun after the earlier M700 API recovery. The screenshot does **not** display the exact local Git SHA, so no exact tested head is claimed.

Factual result:

- the chain progressed past the prior M700 blocker;
- the run reached **Lever Action**;
- UE reported a crash in `Foreground Worker #1` with stack frames inside `UnrealEditor-DerivedDataCache.dll` and `UnrealEditor-Core.dll` during shutdown;
- wrapper result:

```text
ERROR: Lever Action UE 5.8 pilot failed with code 3.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

This means phases 4/5 audio import/fresh-load and 5/5 calibration review did not complete in that run.

The failure is treated as a transient animation/asset-compilation teardown stability issue, not as evidence for changing Lever gameplay timing or accepting/rejecting the `-45°` calibration angle.

## UE 5.8 asset-compilation/DDC teardown recovery

Epic UE 5.8 exposes `AutomationUtilsBlueprintLibrary.finish_all_asset_compilation()` to block until in-flight asset compilation finishes and render-thread follow-up work is drained.

PASS45 now uses that API in both M700 and Lever compatibility shims at two bounded points:

1. after `set_bone_track_keys()` and before sequence sampling;
2. after the proof returns and before PythonScriptCommandlet exit.

Reason for both barriers:

- `set_bone_track_keys()` can trigger animation compression/DDC work through TrackChanged notifications;
- the base pilot immediately samples the transient sequence;
- after sampling/evidence work, commandlet shutdown must not destroy transient imported/animation objects while foreground workers still own compilation follow-up work.

The current regression verifier requires:

- UE 5.8 `add_bone_curve()` creation for BOLT/LEVER;
- no direct `.add_bone_track(` in compatibility shims;
- `finish_all_asset_compilation()` through one centralized helper per shim;
- a pre-sampling barrier;
- a pre-commandlet-exit barrier;
- no production save/cutover/acceptance mutation.

These changes remain **CODED_UNTESTED** locally until the bounded UE 5.8 chain is rerun on a checkout fast-forwarded to the newest canonical branch head.

## Weapon runtime cadence

Do **not** run the expensive full UE 5.8 gameplay acceptance after every small weapon tweak.

Required cadence:

1. bounded source/import/fresh-load/calibration checks while configuring individual weapons;
2. finish the intended weapon setup set;
3. run one consolidated current-head weapon runtime acceptance across that set;
4. retain direct visual/audio/gameplay evidence;
5. close checklist items only from factual accepted results.

## Asset quarantine

The downloaded batch remains quarantined on `asset-intake-20260903`, never wholesale merged into production PASS45. Quarantine does not preempt item 16 and creates no checklist items 37+.

## Binding reuse-first / legal sequence

`quarantine -> SHA/duplicate/safe-ZIP audit -> model/texture/rig/animation inventory -> exact provenance/license/public-repo permission -> quality audit -> REJECT/DONOR_ONLY/PRODUCTION_CANDIDATE -> selective SOURCE_ASSETS promotion -> individual third-party register record -> isolated UE 5.8 proof -> integration -> cutover -> obsolete-owner cleanup -> regression/runtime acceptance`

Unknown source/license remains fail-closed.

## Next factual operation

Do **not** run the full gameplay runtime yet.

On a local PASS45 checkout fast-forwarded to the newest canonical branch head, run only:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The next narrow acceptance target is:

- M700 still passes;
- Remington bounded phase still passes;
- Lever emits its normal pilot PASS and commandlet exits cleanly without DerivedDataCache/foreground-worker crash;
- phases 4/5 and 5/5 then complete.

After all five bounded phases pass, use the report plus direct current-head UE 5.8 visual observation to determine factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences and later run the consolidated full weapon runtime acceptance.

PR #94 remains OPEN / UNMERGED.

```text
official_progress=22/36=61.1%
remaining=38.9%
item16_checked=0
runtime_acceptance=0
merge_permitted=0
```
