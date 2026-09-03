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

**PASS45 ACTIVE / ITEM16 OPEN / M700 BOUNDED UE58 PROOF PASS / REMINGTON BOUNDED UE58 PUMP+ASSEMBLY PROOF PASS / LEVER 52@60 INTEGRAL RESAMPLE GRID + PRE-SAMPLING COMPILATION BARRIER FACTUALLY PASS / LATEST LEVER REJECTION IS STALE 0.85-VS-0.866667 DURATION ASSERTION / CURRENT SOURCE BRIDGES MOTION DURATION VS TECHNICAL ENVELOPE / COMPONENT-FIRST REMOTE PREFLIGHT GREEN / USER LOCAL CHECKS PAUSED BY EXPLICIT INSTRUCTION / REMOTE SOURCE+CI WORK CONTINUES / FULL FIVE-PHASE CHAIN DEFERRED / FULL GAMEPLAY DEFERRED / PR94 UNMERGED**

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

Historical status for those source corrections remains recorded as **CODED_UNTESTED** unless a later factual local build/import result explicitly verifies that exact recovery. These markers exist to prevent regression; they do not outrank newer runtime truth.

## 4. Latest direct local item-16 UE truth — 2026-09-03

The newest supplied local five-phase item-16 run produced a more advanced result than the previous crash:

### Phase 1/5 — M700

**PASS — bounded BOLT translation proof only.**

UE 5.8 preserved the factual `BOLT` joint and non-trivial bounded translation. Final bolt travel, bolt rotation, production cutover and runtime acceptance remain pending.

### Phase 2/5 — Remington 870

**PASS — derived pump + imported assembly evidence only.**

UE 5.8 preserved the derived pump proof and recorded the imported Remington assembly shape. Production visual completeness and direct gameplay/audio acceptance remain pending.

### Phase 3/5 — Lever Action

The prior 25.5-frame compression assertion is no longer the factual blocker. The run emitted:

```text
PASS45_LEVERACTION_UE58_RESAMPLE_GRID_READY initial_fps=30 compat_fps=60 compat_frames=52 source_frames=26 motion_end_frame=51 tail_pad_frames=1
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_BEGIN stage=after_set_bone_track_keys_before_sampling
PASS45_LEVERACTION_UE58_ASSET_COMPILATION_BARRIER_END stage=after_set_bone_track_keys_before_sampling
```

Therefore the 52-frame integral resampling envelope and the pre-sampling compilation barrier are factually working on the tested local UE 5.8 environment.

The run then failed in the base pilot's older duration assertion:

```text
PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_FAIL pilot_sequence_duration_mismatch=1 expected=0.85 actual=0.8666666746139526
ERROR: Lever Action UE 5.8 pilot failed with code -1.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

This is a validation-contract mismatch, not a new animation-compression crash:

- factual Lever motion duration = `0.85 s`;
- factual motion endpoint = frame `51 @ 60 fps`;
- legal UE 5.8 technical sequence envelope = `52 / 60 = 0.866666... s`;
- tail pad = one returned-bind-pose frame;
- initial 30 fps resample envelope = exactly `26` frames.

### Phases 4/5 and 5/5

Not reached in this run. Manual-action audio import/fresh-load and calibration review therefore have no new factual local verdict from this execution.

## 5. Current source recovery after the latest Lever rejection

Current source now separates motion duration from the technical UE 5.8 sequence envelope instead of pretending they are the same value.

Recovery commits:

- `a03b128c902b4103eabf60b7a85f779992ff24cd` — Lever compatibility shim arms the legacy base duration validation only for the legal `0.866666... s` envelope, then restores factual `0.85 s` before sampling/evidence/PASS output;
- `48d097d801e4e515bec9d728d59a6eec55f758c0` — regression guard requires that bridge and preserves the 52-frame / 53-key integral-grid contract.

The shim remains proof-only:

```text
motion_duration=0.85
motion_end_frame=51
sequence_frames=52
sequence_duration=0.866666...
tail_pad_frames=1
source30_frames=26
production_cutover=0
runtime_acceptance=0
item16_checked=0
```

This recovery is **CODED_UNTESTED** locally until factual UE 5.8 evidence supersedes it.

## 6. Binding component-first local UE debugging cadence

`_DOCS/PASS45_COMPONENT_FIRST_UE_DEBUGGING_PROTOCOL.md` is binding together with `AGENTS.md` workflow rule 31.

After a chain stops at one component:

1. exhaust source/static/preflight checks first;
2. inspect current UE APIs, frame/key/duration/envelope math, base/wrapper assertions, async teardown, launcher return contracts and stale verifiers;
3. rerun only the failed component;
4. do not repeat earlier phases that already passed;
5. run the full multi-phase chain once only after affected components individually pass;
6. run full gameplay only as consolidated acceptance, never as a script debugger.

For the current checkpoint, M700 and Remington are not rerun while Lever is the unresolved component. User-local execution itself is now paused by explicit instruction, so the active queue remains remote/source/CI only.

## 7. Historical Pass 44 non-regression

### Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass 44 as a complete solution. Pass 45 is the active corrective pass.

### Pass 44 behavior retained unless disproved

Protected retained behavior unless newer factual evidence invalidates it:

- compact central-Oster playable extent; never restore the historical 2.4 km battlefield;
- zero implicit filler bots in normal local gameplay;
- actual live Museum BASE pawn proof, not source-only spawnpoint existence;
- compact-reference tactical-map bounds;
- authored material gaps stay fail-visible; grey/BasicShape repair stays forbidden;
- retired Pass37 weapon-palette compatibility owner stays physically deleted.

## 8. Item 16 current boundary

Item 16 still requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus local UE 5.8 acceptance.

### Remington 870

Current source retains:

- `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

The bounded local assembly proof now passes, but direct current-head visible-pump and mechanical-audio gameplay acceptance remain pending.

### M700

- factual weighted `BOLT` moving part exists;
- bounded translation proof now passes locally;
- `BOLT_STOP` is not an accepted authored travel endpoint;
- final travel + bolt rotation still require direct current-head visual calibration before production authoring/cutover.

### Lever Action

- factual `LEVER` moving part exists;
- 52-frame UE 5.8 technical envelope and pre-sampling barrier now have factual local evidence;
- current `-45°` excursion remains calibration-only;
- the duration-validation bridge remains source-corrected but locally unverified on the newest head.

The eventual production-authoring boundary remains **MANUAL CURRENT-HEAD UE 5.8 VISUAL CALIBRATION**. The user's current refusal to perform PC checks does not convert pending evidence into acceptance.

## 9. Current CI truth

On `4c98f55db606ea17061cfa4ae43850cd0aa7cfdb`, the component-first preflight and item-16-specific source contracts are green, including frame-rate compatibility and the five-phase evidence-chain contract.

The remaining red workflows are stale checkpoint/verifier compatibility failures rather than new UE/gameplay regressions:

- asset-intake history markers were dropped from the compact history;
- checkpoint verifier required an old exact quarantine/calibration phrase;
- Pass44 verifier claimed to be semantic but still required frozen ledger headings.

Current remote work repairs those checkpoint/verifier contracts without resurrecting rejected Pass44 behavior and without changing runtime acceptance.

## 10. Explicit user-local execution boundary

Latest explicit user instruction on 2026-09-03: **no PC-side checking. Continue `PASS45_RUNTIME_RECOVERY_TZ.md` from the latest repository/checkpoint.**

Binding effect for future continuation:

- do not send the user to GitHub Desktop, CMD, UE Editor or local launchers as the active next step;
- continue assistant-owned GitHub/source/CI work whenever technically possible;
- local runtime/visual evidence may remain pending, but must never be fabricated or promoted from CI;
- do not merge PR #94 while runtime acceptance remains pending;
- local execution can return to the queue only if the user later explicitly resumes it.

## 11. Next factual operation

The active operation is remote, not user-local:

1. settle stale exact-head CI failures caused by checkpoint/verifier wording drift;
2. keep item 16 open and fail-closed;
3. continue remote-preparable item-16 source/acceptance hardening that does not invent final M700/Lever calibration;
4. preserve deferred consolidated runtime acceptance as a later factual gate, not a current instruction to the user.

Do **not** run or request `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` now.

Do **not** merge PR #94.

## 12. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- `runtime_acceptance=0`.
- `merge_permitted=0`.
- `user_local_execution_requested=0`.
- Local user `Changes`: **DO NOT TOUCH**.