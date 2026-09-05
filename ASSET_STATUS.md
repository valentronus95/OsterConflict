# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-05  
Canonical PASS45 branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Canonical PR: **#94 OPEN / UNMERGED**  
Latest code-repair checkpoint before this status commit: `386b84fa5282bef0552ab3e42fe50e1baaa7e666`  
Base/current main: `a1ad0e200611911102c48180956d82f73d0d8fc3`  
Factual asset progress: **70%**  
First unfinished asset checkpoint: `LOCAL-UE-ASSET-001`

## 1. SINGLE-BRANCH AUTHORITY

PASS45 has one work branch only:

`fix/pass45-runtime-rejection-material-closure-20260826`

`fix/pass45-asset-import-fail-closed-20260904` is retired and PR #98 is closed. It must not be resumed.

The binding rule is in `PASS45_RUNTIME_RECOVERY_TZ.md`: no second PASS45 branch/PR for audit, checkpoint, asset intake, temporary work, CI repair or individual fixes. Parallel chats use the same canonical branch.

The user's local Git workflow is GitHub Desktop. Local files shown under **Changes** are user-local work and are not discarded/reset/stashed by PASS45 maintenance.

## 2. OBSOLETE PACKET-RUN DEFECT — CLOSED

The earlier seven `code=1` failures with `\"C:\\...\\command.cmd\" is not recognized` were one Windows dispatch defect in the packet runner, not seven UE/content failures.

The canonical Python runner now dispatches `call`, the batch path and arguments separately. The escaped-quote failure must not be treated as current evidence anymore.

## 3. LATEST FACTUAL LOCAL UE 5.8 BATCH RUN — 2026-09-05

The first real run after the Windows batch-dispatch fix reported **41 tracked Changes** and preserved them.

Observed stage results:

- `ALL local/Fab assets: prepare + import + runtime bindings` — **FAIL code=1**, PowerShell `ParserError` surfaced;
- `Stein weapon materials + fresh-load` — **PASS**;
- `M700/Lever manual-action audio + fresh-load` — **PASS**;
- `Remington 870 skeletal pump + fresh-load` — **FAIL code=11**;
- `HMMWV + M2 + BTR-4 production intake` — **FAIL native code 0xFFFFFFFF / -1**;
- `Final OsterConflictEditor C++ build` — **FAIL code=6**;
- `Every required weapon opens in fresh UE` — **PASS**;
- `Strict authored material/dependency gate` — **FAIL code=11**;
- gameplay runtime did **not** start because five preflight blockers remained.

The old `aqProf.dll`, `VtuneApi.dll` and `VtuneApi32e.dll` lines are optional profiler-DLL diagnostics and are not accepted as the cause of these failures.

### Confirmed build cause

UE 5.8/MSVC rejected `OCPickupGunTruck.cpp` because a `UE_LOG` format argument was selected through a ternary `?:` expression. UE 5.8 compile-time format validation requires a literal `TCHAR` format array. The two messages are now emitted from separate `UE_LOG` calls.

### PowerShell path repair

`IMPORT_ALL_LOCAL_INBOX_UE58.cmd` passed `%~dp0` directly as a quoted PowerShell `-ProjectDir`; that value ends in `\`. It now uses a dot-qualified `%~dp0.` path, matching the already-safe production-vehicle recovery wrapper. This repair remains **CODED_UNTESTED** until the next local rerun.

### Diagnostics repair

The batch reporter now:

- suppresses `aqProf/Vtune` noise from the primary cause list;
- emits context around `ParserError` instead of only `CategoryInfo`;
- normalizes Windows unsigned `4294967295` to `-1`;
- prints more failure lines per stage;
- vehicle import and strict material wrappers now copy their fail markers and log tails into the single packet report.

### Remaining factual blockers

- Remington `code=11` means the wrapper reached the **production import commandlet failure** path; the exact UE/Python cause still requires the next current-head log output;
- vehicle `-1` means the production import commandlet terminated without a valid success sentinel; the exact crash/import cause still requires the next current-head log output;
- strict material `code=11` means its required success sentinel was absent. Because this stage followed a failed C++ build in the same run, do not diagnose it independently until the build is clean on the rerun.

No runtime or visual acceptance is claimed from this run.

## 4. CURRENT FACTUAL ASSET PROGRESS

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

No source-only fix, stale run, old branch CI or diagnostic improvement may raise this percentage.

## 5. PRODUCTION ASSET BOUNDARY

Code/import/runtime hookup exists for current production families including HMMWV, M2 Browning, BTR-4, pickup/technical, M249, Remington 870 and the broader weapon/local-Fab intake path.

Still required factually:

- fresh UE 5.8 import result on the canonical branch;
- current runtime binding evidence;
- gameplay/runtime acceptance;
- direct visual acceptance;
- cleanup only after manifest/SHA-proven acceptance.

M16/M4 remains a factual content gap until a fresh current-run binding snapshot proves at least one bound `M16_M4` asset.

## 6. FIRST REAL UNFINISHED CHECKPOINT — `LOCAL-UE-ASSET-001`

Next valid local cycle:

1. remain on `fix/pass45-runtime-rejection-material-closure-20260826`;
2. GitHub Desktop: **Fetch origin**, then **Pull origin** if offered;
3. do not Discard/Reset/Stash the user's unrelated local `Changes`;
4. launch only `START_HERE.cmd`;
5. choose `2. ПОВНИЙ RUNTIME-ТЕСТ (ПАКЕТНИЙ)`;
6. use the new single report first;
7. for any remaining Remington/vehicle/material failure, use the exact fail marker and log tail now surfaced by the packet report;
8. consume fresh `LOCAL_ASSET_STATUS.txt/json` when generated.

Acceptance path:

- fresh local UE import PASS: **70% → 80%**;
- live runtime PASS: **80% → 90%**;
- direct visual PASS + hash-proven cleanup: **90% → 100%**.

## 7. CONTINUATION RULE

- Do not create a new PASS45 branch.
- Do not create a new PASS45 PR while #94 is active.
- Do not resume PR #98 or its asset branch.
- Do not repeat DONE stages 1–7.
- Do not merge PR #94 to `main` before factual integrated UE 5.8 acceptance.
- Local user `Changes` remain untouched by remote GitHub work.
