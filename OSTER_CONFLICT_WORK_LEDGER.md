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
- UE target: 5.8.x / Windows.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- Local user `Changes`, including local `PC_TEST/TEST_RESULTS` evidence differences, remain outside assistant mutation scope.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance passes required automated and direct visual/audio gates.

Current status token:

**PASS45 ACTIVE / ITEM16 OPEN / LOCAL CF75B86C M700 PILOT REJECTED ON LEGACY UE58 BONE-TRACK CREATION / CURRENT SOURCE RECOVERED TO UE58 BONE-CURVE API FOR M700+LEVER / BOUNDED LOCAL ITEM16 EVIDENCE RERUN REQUIRED / FULL GAMEPLAY RUNTIME NOT YET DUE / PR94 UNMERGED**

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

The newest supplied local evidence tested branch head:

`cf75b86ce5988ef489f0ef653d3f1b3f637278fd`

The user ran the canonical bounded item-16 chain:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The chain failed closed in **phase 1/5 M700** before Remington, Lever, audio import or calibration review could run.

Factual failure:

`PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_FAIL bolt_bone_track_creation_failed=1`

and:

`ERROR: item-16 evidence chain stopped at M700. rc=36`

This is a valid local UE 5.8 rejection of the `cf75b86c...` compatibility path. It is **not** a Remington or Lever runtime verdict because those phases were never reached.

## 4. Current source recovery after that rejection

The local failure exposed a UE 5.8 animation-controller API incompatibility: the legacy `add_bone_track()` path returned an invalid result for the transient imported moving-part bone.

Current branch source has already advanced beyond the rejected head with two narrow compatibility fixes:

- `89bb635d67b24afdb5e32bccd91092401b6024d6` — **M700** pilot now creates the BOLT curve through UE 5.8 `add_bone_curve()` and then writes keys through `set_bone_track_keys()`;
- `3dc5d1b57a6b908b0bd5356e0b01b681e397d285` — **Lever Action** pilot applies the same UE 5.8 bone-curve API correction for LEVER.

These changes preserve the bounded proof contract:

- no production animation package is saved by the M700/Lever motion pilots;
- M700 pilot travel remains calibration-only and bolt rotation remains pending;
- Lever `-45°` remains calibration-only, not an accepted production endpoint;
- `runtime_visual_acceptance=0`;
- `runtime_acceptance=0`;
- `item16_checked=0`;
- `merge_permitted=0`.

Exact-head GitHub Actions on `3dc5d1b5...` are structurally green, including the item-16 evidence-chain, M700, Lever and source-verification contracts. That does **not** substitute for rerunning the chain in the user's actual UE 5.8 installation.

## 5. Item 16 current boundary

Item 16 requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus local UE 5.8 acceptance.

### Remington 870

Current source retains the guarded registered CC-BY-4.0 donor derivative, production skeletal visual and exact PumpCycle path:

- `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

The 2026-09-02 gameplay observation remains a rejection only of the older pre-cutover presentation where recoil occurred but the fore-end did not visibly pump. The newer production path still requires current-head direct visible-pump and mechanical-audio acceptance.

### M700

- factual weighted `BOLT` moving part exists;
- `BOLT_STOP` is **not** an accepted authored travel endpoint;
- bounded translation proof is calibration-only;
- final bolt travel and bolt rotation require direct current-head UE 5.8 visual calibration before production authoring/cutover.

### Lever Action

- factual addressable `LEVER` moving part exists;
- current bounded local-X `-45°` excursion is calibration-only;
- final accepted lever angle requires direct current-head UE 5.8 visual calibration before production authoring/cutover.

## 6. Binding reuse-first / non-regression rules

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

Protected rules:

- one runtime responsibility has one mutating owner;
- authored manual-action animation follows the existing replicated mechanical-action cycle, not a second gameplay timer;
- retired procedural whole-weapon/arms manual-action fallback stays physically retired;
- primary registered Remington donor is exhausted before any second donor is promoted;
- unverified licensing cannot be promoted;
- direct UE/gameplay evidence outranks source-only READY claims;
- source/docs/CI/pilot-only work never inflates canonical checklist progress.

## 7. Next factual operation

Do **not** run `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` yet.

The next local-only operation is to fast-forward the user's checked-out PASS45 branch to the current remote head and run exactly the bounded chain:

`OsterConflict\RUN_PASS45_ITEM16_LOCAL_UE58_EVIDENCE.cmd`

The chain must reach all five phases:

1. M700 bounded BOLT translation proof;
2. Remington derived pump + assembly proof;
3. Lever LEVER motion proof;
4. Bolt/Lever mechanical-audio import + independent fresh-load;
5. M700/Lever calibration evidence consolidation.

After that bounded chain passes, use its report plus direct current-head UE 5.8 visual observation to choose factual M700 travel/rotation and Lever angle. Only then author/cut over accepted M700/Lever production sequences, finish the intended weapon setup batch and run one consolidated full weapon runtime acceptance.

## 8. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Do not merge without current-head UE 5.8 runtime acceptance.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- Local user `Changes`: **DO NOT TOUCH**.
