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

**PASS45 ACTIVE / ITEM16 OPEN / M700 LEGACY BONE-TRACK BLOCKER RECOVERED / LATEST LOCAL CHAIN NOW REACHES LEVER ACTION / LEVER COMMANDLET CRASHES IN DERIVEDDATACACHE FOREGROUND WORKER DURING SHUTDOWN / UE58 ASSET-COMPILATION BARRIERS SOURCE-CODED / BOUNDED LOCAL RERUN REQUIRED / FULL GAMEPLAY RUNTIME NOT YET DUE / PR94 UNMERGED**

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

The newest supplied screenshot is from a bounded item-16 rerun after the earlier M700 API recovery. The screenshot does **not** display the exact local Git SHA, so no exact tested head is invented here.

What the run factually proves:

- the chain progressed beyond the old phase-1 M700 `bolt_bone_track_creation_failed=1` blocker;
- it reached **phase 3/5 Lever Action**;
- UE crashed in a runnable foreground worker with stack frames in `UnrealEditor-DerivedDataCache.dll` and `UnrealEditor-Core.dll`;
- the wrapper reported:

```text
ERROR: Lever Action UE 5.8 pilot failed with code 3.
ERROR: item-16 evidence chain stopped at Lever Action. rc=7
```

Therefore the earlier M700 failure is no longer the current factual blocker for this run. The current blocker is **Lever transient-animation / async asset-DDC teardown stability in the UE 5.8 commandlet host**.

Because the chain stopped at Lever Action, phases 4/5 mechanical-audio import/fresh-load and 5/5 calibration review did not complete in this run.

## 4. Source recovery for the current Lever/DDC rejection

The current source already uses UE 5.8 `add_bone_curve()` for M700 BOLT and Lever LEVER and retains `set_bone_track_keys()` for key writes.

The new crash is not handled by changing gameplay timing, donor identity, final bolt travel or final lever angle. The bounded source recovery addresses commandlet teardown only.

Epic UE 5.8 exposes `AutomationUtilsBlueprintLibrary.finish_all_asset_compilation()` specifically to block until in-flight asset compilation finishes and render-thread follow-up commands are drained. Current PASS45 source now uses explicit barriers in both transient M700 and Lever compatibility shims:

1. immediately after `set_bone_track_keys()` and **before sequence sampling**;
2. once again after the proof returns and **before PythonScriptCommandlet exit**.

Recovery commits:

- `7b70c56e0c1e77c6642ba517d45310d7879be343` — Lever UE 5.8 asset-compilation/DDC barriers;
- `2ac5b9560b63a51be3f57c770c6a93d2c302373c` — same teardown policy for M700 to prevent the shutdown race moving between phases;
- `03ab7bded49fc23ea1c19c23586b86797aaeba93` — regression verifier now requires both bone-curve API recovery and both async-compilation barriers.

These changes are **CODED_UNTESTED** locally until the user's actual UE 5.8 rerun clears Lever without a DDC crash.

The barriers do not save production animation packages and preserve:

```text
runtime_visual_acceptance=0
runtime_acceptance=0
item16_checked=0
merge_permitted=0
```

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
- current technical blocker is commandlet/DDC stability, not acceptance of that angle;
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
- Lever must pass without `DerivedDataCache`/foreground-worker crash and emit its normal pilot PASS;
- only then may phases 4/5 and 5/5 continue.

After all five bounded phases pass, use the report plus direct current-head UE 5.8 visual observation to choose factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences, finish the intended weapon setup batch and run one consolidated full weapon runtime acceptance.

## 9. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Do not merge without current-head UE 5.8 runtime acceptance.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- Local user `Changes`: **DO NOT TOUCH**.
