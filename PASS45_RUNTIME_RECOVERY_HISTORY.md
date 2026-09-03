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
- Lever/M700 asset-compilation barriers: `7b70c56e0c1e77c6642ba517d45310d7879be343`, `2ac5b9560b63a51be3f57c770c6a93d2c302373c`.
- Async-compilation barrier regression guard: `03ab7bded49fc23ea1c19c23586b86797aaeba93`.
- Lever integral 30 fps resampling-grid recovery: `3b66261b79a82deed7ebe698844205176cc92b20`.
- Lever resampling-grid regression guard: `91da695e4dbc07d2a0890e0394e93ba066bc6a92`.
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

The next factual production boundary remains **MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION** after the bounded technical evidence chain itself is stable.

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

## 2026-09-03 latest local Lever resampling-grid rejection

The newest factual local item-16 rerun again progressed beyond M700 and Remington into phase 3/5 Lever Action. It now exposes the failure *before* the previously added asset-compilation barrier can execute.

UE emitted:

```text
Script Stack: /Script/Engine.AnimationDataController.SetNumberOfFrames
Ensure condition failed: FMath::IsNearlyZero(ResampledFrameTime.GetSubFrame())
Incompatible resampling frame rate for animation sequence AN_PASS45_LeverAction_Cycle_Pilot, frame remainder of 0.50000000
Assertion failed: FMath::IsNearlyZero(SampleFrameTime.GetSubFrame())
ERROR: Lever Action UE 5.8 pilot failed with code 3.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

This supersedes the earlier working theory that commandlet teardown itself was the primary blocker. The `DerivedDataCache` foreground-worker stack remains real, but in this run it is downstream of animation compression started by an invalid fractional resampling grid.

The arithmetic is exact:

- transient UE sequence starts on a 30 fps grid;
- compatibility pilot was 60 fps with 51 playable frames;
- `51 / 60 = 0.85 s`;
- `0.85 * 30 = 25.5` source frames;
- UE 5.8 rejects that half-frame remainder during `SetNumberOfFrames` / compression.

The crash therefore happens before `set_bone_track_keys()` and before the pre-sampling compilation barrier. Adding more shutdown waits would merely decorate the wrong side of the failure, a favorite human tradition that the engine has thankfully made impossible with an assertion.

## UE 5.8 Lever integral-grid recovery

The recovery keeps the real Lever motion endpoint exactly at `0.85 s`, which is frame 51 at 60 fps, but gives the transient sequence one additional bind-pose tail frame:

```text
initial_grid_fps=30
compat_fps=60
motion_end_frame=51
motion_duration=0.85
sequence_frames=52
sequence_duration=0.8666666667
tail_pad_frames=1
resampled_source_frames=26
keys=53
```

Why 52 frames:

- `52 / 60 = 0.866666... s`;
- that envelope maps to exactly `26` frames at 30 fps;
- frame 51 remains exactly `0.85 s`, so the authored calibration motion itself is not stretched;
- frame 52 is only a returned-bind-pose padding key because the authoritative motion function clamps at the 0.85 s endpoint.

Current code emits `PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY` before calling `SetNumberOfFrames` and fails closed if the compatibility envelope stops mapping to an integral initial-grid frame.

The previously added `finish_all_asset_compilation()` barriers remain as an independent safety guard after key mutation and before commandlet exit. They are no longer claimed as the root-cause fix for this specific rejection.

Current recovery commits:

- `3b66261b79a82deed7ebe698844205176cc92b20` — 52-frame / 53-key integral resampling envelope while preserving the exact 0.85 s motion endpoint;
- `91da695e4dbc07d2a0890e0394e93ba066bc6a92` — regression contract rejects the old 51-frame envelope and requires the 30→60 integral-grid rule.

These changes remain **CODED_UNTESTED** locally until a new UE 5.8 bounded chain run clears Lever and proceeds into phases 4/5 and 5/5.

## Weapon runtime cadence

Do **not** run the expensive full UE 5.8 gameplay acceptance after every small weapon tweak.

Required cadence:

1. bounded source/import/fresh-load/calibration checks while configuring individual weapons;
2. finish the intended weapon setup set;
3. run one consolidated current-head weapon runtime acceptance across that set;
4. retain direct visual/audio/gameplay evidence;
5. close checklist items only from factual accepted results.

## Asset quarantine

The downloaded batch remains quarantined on `asset-intake-20260903`, never wholesale merged into production PASS45. The earlier **remote quarantine audit** remains factual source-intake evidence; quarantine does not preempt item 16 and creates no checklist items 37+.

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
- Lever prints `PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY` with `compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1`;
- no `frame remainder of 0.50000000` / `SampleFrameTime.GetSubFrame()` assertion occurs;
- Lever reaches its normal pilot PASS and both asset-compilation barriers;
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
