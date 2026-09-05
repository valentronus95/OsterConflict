# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-05  
Canonical PASS45 branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Canonical PR: **#94 OPEN / UNMERGED**  
Current branch checkpoint after consolidation: `afeea2b9660d0745cf819b0c4dd9989ed00db550`  
Base/current main: `a1ad0e200611911102c48180956d82f73d0d8fc3`  
Factual asset progress: **70%**  
First unfinished asset checkpoint: `LOCAL-UE-ASSET-001`

## 1. SINGLE-BRANCH AUTHORITY

PASS45 now has one work branch only:

`fix/pass45-runtime-rejection-material-closure-20260826`

`fix/pass45-asset-import-fail-closed-20260904` is a duplicate work line and is retired. Its PR #98 is not an independent authority and must not be resumed.

The binding branch rule is now written directly into `PASS45_RUNTIME_RECOVERY_TZ.md`: no second PASS45 branch/PR for audit, checkpoint, asset intake, temporary work, CI repair or individual fixes. Parallel chats use the same canonical branch.

## 2. WHAT WAS CARRIED FROM THE DUPLICATE ASSET BRANCH

The duplicate asset branch had only **6 unique commits** relative to the common ancestor but was **1039 commits behind** the current canonical PASS45 branch. Therefore it was not safe to merge or replace the canonical branch wholesale.

Useful factual information was carried here instead of restoring older code:

- the latest local packet run reported **41 tracked Changes** and correctly did not reset/stash/discard them;
- seven preflight stages returned `code=1` with the same Windows dispatch signature;
- even UE `Build.bat` failed with the same signature;
- this proved one common launcher/quoting defect in that obsolete packet implementation, not seven independent UE/content failures;
- the visible error form was an executable path treated as a literal command containing escaped quotes, e.g. `\"C:\\...\\command.cmd\"`;
- the old packet run therefore stopped before gameplay and cannot count as current runtime acceptance;
- factual asset progress remains **70%** until a fresh current-head UE 5.8 run passes.

The duplicate branch's PowerShell packet owner is **not** copied over the newer canonical implementation. The canonical branch already owns packet runtime through:

- `START_HERE.cmd` option `2. ПОВНИЙ RUNTIME-ТЕСТ (ПАКЕТНИЙ)`;
- `OsterConflict/PASS45_BATCH_RUNTIME.cmd`;
- `OsterConflict/Scripts/pass45_batch_runtime.py`.

That newer Python orchestrator is the authority and preserves the user's local tracked Changes while collecting one batch report.

## 3. CURRENT FACTUAL ASSET PROGRESS

| Stage | State | Progress |
|---|---|---:|
| 1. Local inbox / intake contract | DONE | 10% |
| 2. Prepare / extract / classify | DONE | 10% |
| 3. Exact duplicate removal | DONE | 10% |
| 4. Fab / Marketplace / project discovery | DONE | 10% |
| 5. Production import logic | DONE | 10% |
| 6. Fail-closed aggregate/binding result | DONE | 10% |
| 7. Source/regression/finalization guards | DONE | 10% |
| 8. Fresh local UE 5.8 import on current canonical HEAD | WAIT | 0% |
| 9. Live gameplay/runtime hookup | WAIT | 0% |
| 10. Direct visual acceptance + safe ZIP cleanup | WAIT | 0% |

Current factual total: **70%**. Remaining: **30%**.

No stale run, old branch CI, source-only proof or local launcher mismatch may raise this percentage.

## 4. PRODUCTION ASSET BOUNDARY

Code/import/runtime hookup exists for the current production families, including HMMWV, M2 Browning, BTR-4, pickup/technical, M249, Remington 870 and the broader weapon/local-Fab intake path.

Still required factually:

- fresh UE 5.8 import result on the canonical branch;
- current runtime binding evidence;
- gameplay/runtime acceptance;
- direct visual acceptance;
- cleanup only after manifest/SHA-proven acceptance.

M16/M4 remains a factual content gap until a fresh current-run binding snapshot proves at least one bound `M16_M4` asset.

## 5. FIRST REAL UNFINISHED CHECKPOINT — `LOCAL-UE-ASSET-001`

Next valid local cycle:

1. keep the user on `fix/pass45-runtime-rejection-material-closure-20260826`;
2. synchronize that same branch to its current remote HEAD without creating another work branch;
3. do not discard/reset/stash unrelated local `Changes` as part of acceptance;
4. launch only `START_HERE.cmd`;
5. choose `2. ПОВНИЙ RUNTIME-ТЕСТ (ПАКЕТНИЙ)`;
6. read `Logs/PASS45_BATCH_RUNTIME_REPORT.txt` first;
7. inspect only the concrete failing stage logs;
8. consume fresh `LOCAL_ASSET_STATUS.txt/json` when generated.

Acceptance path:

- fresh local UE import PASS: **70% → 80%**;
- live runtime PASS: **80% → 90%**;
- direct visual PASS + hash-proven cleanup: **90% → 100%**.

## 6. CONTINUATION RULE

- Do not create a new PASS45 branch.
- Do not create a new PASS45 PR while #94 is active.
- Do not resume PR #98 or its asset branch.
- Do not repeat DONE stages 1–7.
- Do not merge PR #94 to `main` before factual integrated UE 5.8 acceptance.
- Local user `Changes` remain untouched by remote GitHub work.
