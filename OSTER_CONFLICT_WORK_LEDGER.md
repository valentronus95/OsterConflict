# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

The previous complete ledger remains preserved by Git history. This live ledger stays compact so new sessions resume from the current factual blocker instead of replaying completed work.

## 1. Current context — 2026-09-03

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Pass 45 remains the ACTIVE corrective pass.
- UE target: 5.8.x / Windows.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- Local user `Changes`, including local `PC_TEST/TEST_RESULTS` evidence ZIPs, remain outside assistant mutation scope.
- Latest explicit user instruction: **do not require/request PC-side checks; continue PASS45 remotely from repository/CI/checkpoint state**.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance passes required automated and direct visual/audio gates.

Current status token:

**PASS45 ACTIVE / ITEM16 OPEN / M700 BOUNDED UE58 PROOF PASS / REMINGTON BOUNDED UE58 PUMP+ASSEMBLY PROOF PASS / LEVER 52@60 INTEGRAL RESAMPLE GRID + PRE-SAMPLING COMPILATION BARRIER FACTUALLY PASS / CURRENT LEVER DURATION-ENVELOPE BRIDGE CODED_UNTESTED LOCALLY / MANUAL-ACTION AUDIO PLAYBACK DISPATCH INSTRUMENTED / EXACT CURRENT PLAYBACK OBJECT IDENTITY PINNED / AUDIO+AUTHORED ANIMATION BOUND TO SAME BActionCycling RISING EDGE BY SOURCE CONTRACT / USER LOCAL CHECKS PAUSED / FULL GAMEPLAY DEFERRED / PR94 UNMERGED**

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
- Historical verifiers may preserve factual rejected evidence but may not resurrect retired owners or stale runtime behavior.

## 3. Historical local build/import rejection — retained regression evidence

This historical fact remains required non-regression evidence and is not the current item-16 blocker.

**LOCAL UE BUILD REJECTED — 2026-08-25.** Local UE 5.8.1 / MSVC rejected the tactical-road `FVector2D` table with **C2131** when it was authored as `constexpr`. The source fix changed that table to ordinary `const` while retaining `UE_ARRAY_COUNT`.

The same local recovery cycle also exposed the deprecated Interchange property **`auto_detect_mesh_type`** for HMMWV/M2 GLB intake. Current source instead forces the static-mesh type and disables skeletal import through the supported UE 5.8 policy.

Historical status for those source corrections remains **CODED_UNTESTED** unless a later factual local build/import result explicitly verifies that exact recovery.

## 4. Latest direct local item-16 UE truth — 2026-09-03

### Phase 1/5 — M700

**PASS — bounded BOLT translation proof only.**

UE 5.8 preserved the factual `BOLT` joint and non-trivial bounded translation. Final bolt travel, bolt rotation, production cutover and runtime acceptance remain pending.

### Phase 2/5 — Remington 870

**PASS — derived pump + imported assembly evidence only.**

UE 5.8 preserved the derived pump proof and recorded the imported Remington assembly shape. Production visual completeness and direct gameplay/audio acceptance remain pending.

### Phase 3/5 — Lever Action

The run proved:

```text
PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY initial_fps=30 compat_fps=60 compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_BEGIN stage=after_set_bone_track_keys_before_sampling
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=after_set_bone_track_keys_before_sampling
```

It then stopped on the old duration assertion:

```text
expected=0.85
actual=0.8666666746139526
```

Current source correctly separates factual `0.85 s` motion duration from the legal `52/60 = 0.866666... s` technical sequence envelope and restores `0.85 s` before sampling/evidence. That bridge remains **CODED_UNTESTED** locally.

### Phases 4/5 and 5/5

Not reached in the latest factual local run. No new local audio/calibration verdict may be inferred from source CI.

## 5. Item 16 current production boundary

Item 16 still requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus current-head UE 5.8 acceptance.

### Remington 870

Production source remains:

- `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

The bounded local assembly proof passes, but direct current-head visible-pump and mechanical-audio gameplay acceptance remain pending.

### M700

- factual weighted `BOLT` moving part exists;
- bounded translation proof passes locally;
- `BOLT_STOP` is not an accepted travel endpoint;
- final travel + bolt rotation still require direct current-head visual calibration before production authoring/cutover.

### Lever Action

- factual `LEVER` moving part exists;
- 52-frame UE 5.8 technical envelope and pre-sampling barrier have factual local evidence;
- current `-45°` excursion remains calibration-only;
- final lever angle remains unaccepted.

The eventual production-authoring boundary remains **MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION**. The user's current pause on PC checks does not convert pending evidence into acceptance.

## 6. Current manual-action audio/evidence hardening

Current source emits factual playback evidence from the real `HandleStateEventLocal()` path:

```text
PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_DISPATCHED
weapon=...
action=...
sound=...
route=local2d
bus_gt_zero=1
effective_volume_gt_zero=1
second_gameplay_timer=0
runtime_acceptance=0
```

Failure to dispatch a usable sound emits `PASS45_MANUAL_ACTION_AUDIO_PLAYBACK_FAIL`, which the runtime verifier rejects.

The runtime gate requires the exact current expected playback object:

- `OC_SNP1` → `/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor`;
- `OC_SG1` → `/Game/R13/Audio/shotguncock.shotguncock`;
- `R13_LEVER4570` → `/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor`.

The static guard binds these runtime expectations to the actual source `LoadSound()` paths, and the executable adversarial contract rejects a wrong M700 playback object.

Commit `80d0f727f02194530497a84ba5bc389f018c4c29` adds the next structural guard: both the `ManualActionCycle` audio dispatch and the authored moving-part animation start must remain inside the **same** `if (bActionCycling && !State.bWasActionCycling)` rising-edge block, in that order, with no presentation-owned timer or early return. It also adversarially rejects loss of the rising-edge gate or authored start attempt.

Exact-head `Pass 45 item16 manual-action timing semantics` and `Pass 45 manual-action audio provenance` both passed this guard.

These are source/runtime-evidence contracts, not direct audible or visual quality acceptance.

## 7. Exact-head acceptance integrity

`RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd` already:

1. resolves and pins current `git HEAD` before runtime;
2. rejects tracked unstaged or staged changes before runtime;
3. executes the actual playflow/runtime route;
4. resolves HEAD again after runtime/material gates and rejects head drift;
5. rejects tracked mutations produced during runtime/import stages;
6. only then executes strict evidence verifiers, including the M700/870/Lever manual-action verifier.

Therefore a stale arbitrary working-tree log is not accepted as an exact-head runtime result by the strict main route. This source audit does not replace the deferred actual runtime execution.

## 8. Binding component-first cadence and local execution boundary

`_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md` and `AGENTS.md` rules 30–31 remain binding.

User-local execution is currently paused by explicit instruction. Therefore:

- do not send the user to GitHub Desktop, CMD, UE Editor or local launchers as the active next step;
- continue assistant-owned GitHub/source/CI work whenever technically possible;
- local runtime/visual evidence may remain pending, but must never be fabricated or promoted from CI;
- do not merge PR #94 while runtime acceptance remains pending;
- local execution can return to the queue only if the user later explicitly resumes it.

The `asset-intake-20260903` branch remains quarantine-only and must never be merged wholesale. No checklist items 37+ are created from quarantine inventory.

## 9. Historical Pass 44 non-regression

### Pass 44 historical runtime rejection

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass 44 as a complete solution. Pass 45 is the active corrective pass.

### Pass 44 behavior retained unless disproved

Protected retained behavior unless newer factual evidence invalidates it:

- compact central-Oster playable extent, never restore the historical 2.4 km battlefield;
- zero implicit filler bots in normal local gameplay;
- actual live Museum BASE pawn proof, not source-only spawnpoint existence;
- compact-reference tactical-map bounds;
- authored material gaps stay fail-visible and grey/BasicShape repair stays forbidden;
- retired Pass37 weapon-palette compatibility owner stays physically deleted.

These retained behaviors are historical non-regression constraints only. They do not restore Pass 44 as active or verified.

## 10. Current exact-head CI truth

`46c400c228661a4200248ae35bb819c420ce5a2d` had a fully green pull-request workflow matrix, including all item-16 audio, calibration, production-cutover, UE58 compatibility, Remington, strict runtime harness, main launcher and Source verification gates.

On `80d0f727f02194530497a84ba5bc389f018c4c29`:

- `Pass 45 item16 manual-action timing semantics`: **SUCCESS**;
- `Pass 45 manual-action audio provenance`: **SUCCESS**;
- Source verification reached the full structural suite and exposed only a **documentation regression** introduced by the previous ledger compaction: the required historical Pass44 rejection/non-regression section had been removed. Runtime/gameplay source was not implicated.

This ledger revision restores that historical section instead of weakening the verifier. The next exact-head Source verification must confirm the repair.

Green source CI never substitutes for current-head UE 5.8 direct visual/audio acceptance.

## 11. Next factual operation

The active operation remains remote, not user-local:

1. confirm exact-head Source verification after restoring the required Pass44 historical ledger contract;
2. keep the same-rising-edge audio/authored-animation source guard green;
3. keep final M700 travel/rotation and Lever angle unaccepted until factual visual calibration exists;
4. keep Remington visible-pump/mechanical-audio acceptance pending factual current-head gameplay evidence;
5. preserve one later consolidated runtime acceptance instead of repeated full-game launches;
6. do not merge PR #94 and do not promote runtime acceptance from CI alone.

Do **not** run or request `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` now.

## 12. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- `runtime_acceptance=0`.
- `item16_checked=0`.
- `merge_permitted=0`.
- `user_local_execution_requested=0`.
- Local user `Changes`: **DO NOT TOUCH**.