# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

The previous complete ledger remains preserved by Git blob:

`f480ba6dc01de2b41e9d3970f7a91b8b4cee1966`

Use Git history and PASS45 archived ledgers for older implementation detail. This live ledger intentionally stays compact so new sessions resume from the current factual blocker rather than replaying already completed work.

## 1. Current context — 2026-09-03

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- **Pass 45 remains the ACTIVE corrective pass.**
- UE target: 5.8.x / Windows.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- Local user `Changes`, including local `PC_TEST/TEST_RESULTS` evidence differences, remain outside assistant mutation scope.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance passes required automated and direct visual/audio gates.

Current status token:

**PASS45 ACTIVE / ITEM16 OPEN / M700 LEGACY BONE-TRACK BLOCKER RECOVERED / LATEST LOCAL CHAIN REACHES LEVER ACTION / LEVER 51@60 ENVELOPE PROVEN INVALID AGAINST UE58 30FPS RESAMPLING GRID / 52@60 ENVELOPE WITH 0.85S MOTION END + ONE BIND-POSE PAD FRAME SOURCE-CODED / BOUNDED LOCAL RERUN REQUIRED / FULL GAMEPLAY RUNTIME NOT YET DUE / PR94 UNMERGED**

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure is incomplete.
- `CODED_UNTESTED` — source correction exists but current factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `AUDIO CONTENT GAP` — routing exists but accepted authored sound is absent/unverified.
- `RUNTIME REJECTED` — factual local gameplay or UE execution disproved the tested result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds on the tested head.
- `VERIFIED RUNTIME` — factual current-head UE/user playtest proves behavior/appearance.
- Green GitHub/source CI is structural evidence only, not UE runtime acceptance.
- Older runtime evidence remains factual for the head it tested, but cannot prove a later recovery head.
- Historical verifiers may not resurrect retired owners or stale asset/API paths.

## 3. Latest direct local UE truth — 2026-09-03

The newest supplied local log is from a bounded item-16 rerun. It again progressed beyond M700 and Remington into **phase 3/5 Lever Action**.

The decisive lines are:

```text
Script Stack: /Script/Engine.AnimationDataController.SetNumberOfFrames
Ensure condition failed: FMath::IsNearlyZero(ResampledFrameTime.GetSubFrame())
Incompatible resampling frame rate for animation sequence AN_PASS45_LeverAction_Cycle_Pilot, frame remainder of 0.50000000
Assertion failed: FMath::IsNearlyZero(SampleFrameTime.GetSubFrame())
ERROR: Lever Action UE 5.8 pilot failed with code 3.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

This refines the prior diagnosis. The `DerivedDataCache` foreground-worker crash is real, but the new log proves the immediate initiating fault is the Lever animation resampling grid during `SetNumberOfFrames`, before the pre-sampling asset-compilation barrier can run.

The rejected compatibility envelope was:

```text
initial_transient_grid=30 fps
compatibility_grid=60 fps
sequence_frames=51
sequence_duration=51/60=0.85 s
resampled_30fps_frames=25.5  <-- invalid fractional frame
```

Because the chain still stops at Lever Action, phases 4/5 mechanical-audio import/fresh-load and 5/5 calibration review remain incomplete.

## 4. Source recovery for the Lever resampling rejection

The real Lever gameplay/calibration motion remains exactly **0.85 s**. The source does **not** stretch that motion.

The compatibility sequence now uses:

```text
initial_transient_grid=30 fps
compatibility_grid=60 fps
motion_end_frame=51
motion_duration=0.85 s
sequence_frames=52
sequence_duration=0.8666666667 s
tail_pad_frames=1
resampled_30fps_frames=26
key_count=53
```

The important distinction is between the motion endpoint and the transient sequence envelope:

- frame 51 at 60 fps is still exactly 0.85 s and returns the Lever to bind pose;
- frame 52 is one additional bind-pose padding frame only;
- `52/60 * 30 = 26`, so UE 5.8 no longer has a 0.5 source-frame remainder to assert on;
- the regression guard rejects restoration of the old 51-frame envelope.

Current recovery commits:

- `3b66261b79a82deed7ebe698844205176cc92b20` — Lever 52-frame integral resampling envelope with exact 0.85 s motion endpoint preserved;
- `91da695e4dbc07d2a0890e0394e93ba066bc6a92` — regression guard for 30→60 integral frame mapping.

The previously added `finish_all_asset_compilation()` barriers remain in M700 and Lever as a separate post-mutation/teardown safety measure. They are no longer claimed as the root-cause correction for the latest rejection.

These changes remain **CODED_UNTESTED** until the user's real UE 5.8 bounded rerun accepts them.

## 5. Historical Pass 44 non-regression

### Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass 44 as a complete solution. Pass 45 is the active corrective pass. This historical rejection remains evidence and may not be erased by later source fixes.

### Pass 44 behavior retained unless disproved

The following accepted non-regression decisions remain protected unless newer factual evidence explicitly invalidates them:

- compact central-Oster playable extent, never restore the historical 2.4 km battlefield;
- normal local gameplay defaults to zero implicit filler bots unless explicitly requested;
- Museum BASE acceptance is based on the actual live pawn, not source-only spawnpoint existence;
- tactical-map bounds follow the compact central-Oster reference rather than legacy peripheral auto-fit;
- grey/BasicShape weapon material repair remains forbidden and authored material gaps stay fail-visible;
- the retired Pass37 weapon-palette compatibility owner stays physically deleted.

These retained facts do not authorize resurrection of any Pass 44 owner or repair path later rejected by Pass 45.

## 6. Item 16 current boundary

Item 16 requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus local UE 5.8 acceptance.

### Remington 870

Current source retains the guarded registered CC-BY-4.0 donor derivative, production skeletal visual and exact PumpCycle path:

- `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

The 2026-09-02 gameplay observation remains a rejection only of the older pre-cutover presentation where recoil occurred but the fore-end did not visibly pump. The newer production path still requires current-head direct visible-pump and mechanical-audio acceptance.

### M700

- factual weighted `BOLT` moving part exists;
- `BOLT_STOP` is **not** an accepted authored travel endpoint;
- bounded translation proof remains calibration-only;
- final bolt travel and bolt rotation require direct current-head UE 5.8 visual calibration before production authoring/cutover.

### Lever Action

- factual addressable `LEVER` moving part exists;
- current bounded local-X `-45°` excursion remains calibration-only;
- current technical blocker is UE 5.8 transient sequence resampling compatibility, not acceptance of that angle;
- final accepted lever angle still requires direct current-head UE 5.8 visual calibration after the bounded pilot becomes stable.

## 7. Binding reuse-first / non-regression rules

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

Protected rules:

- one runtime responsibility has one mutating owner;
- authored manual-action animation follows the existing replicated mechanical-action cycle, not a second gameplay timer;
- retired procedural whole-weapon/arms manual-action fallback stays physically retired;
- primary registered Remington donor is exhausted before any second donor is promoted;
- unverified licensing cannot be promoted;
- direct UE/gameplay evidence outranks source-only READY claims;
- source/docs/CI/pilot-only work never inflates canonical checklist progress.

## 8. Next factual operation

Do **not** run `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` yet.

The next local-only operation is to fast-forward the user's checked-out PASS45 branch to the newest remote head and rerun exactly:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The next acceptance boundary is narrow:

- M700 must still pass;
- Remington phase must still pass its bounded proof;
- Lever must emit `PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY` with `compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1`;
- the old `frame remainder of 0.50000000` and `SampleFrameTime.GetSubFrame()` assertion must be absent;
- Lever must then reach its normal pilot PASS and both asset-compilation barriers;
- only then may phases 4/5 and 5/5 continue.

After all five bounded phases pass, use the report plus direct current-head UE 5.8 visual observation to choose factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences, finish the intended weapon setup batch and run one consolidated full weapon runtime acceptance.

## 9. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Do not merge without current-head UE 5.8 runtime acceptance.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- Local user `Changes`: **DO NOT TOUCH**.
