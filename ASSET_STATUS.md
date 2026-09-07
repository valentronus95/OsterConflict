# OSTER CONFLICT — ASSET STATUS

Date: 2026-09-05  
Canonical PASS45 branch: `fix/pass45-runtime-rejection-material-closure-20260826`  
Canonical PR: **#94 OPEN / UNMERGED**  
Latest code-repair checkpoint before this status commit: `3c0f29959da2e0c83f076251d925b4f6fd2c7364`  
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

The newest run reported **43 tracked Changes** and preserved them.

Observed stage results:

- `ALL local/Fab assets: prepare + import + runtime bindings` — **FAIL code=1**;
- `Stein weapon materials + fresh-load` — **PASS**;
- `M700/Lever manual-action audio + fresh-load` — **PASS**;
- `Remington 870 skeletal pump + fresh-load` — **FAIL code=11**;
- `HMMWV + M2 + BTR-4 production intake` — **FAIL code=-1**;
- `Final OsterConflictEditor C++ build` — **FAIL code=6**;
- `Every required weapon opens in fresh UE` — **PASS**;
- `Strict authored material/dependency gate` — **FAIL code=11**;
- gameplay runtime did **not** start because five preflight blockers remained.

### 3.1 ALL local/Fab assets — exact cause confirmed and repaired

The apparent `Missing closing '}'`, `UnexpectedToken` and `Array index expression` errors were cascading parser symptoms, not a genuinely missing brace at the final reported line.

A dedicated `windows-latest` CI job running the actual **Windows PowerShell 5.1 parser** reproduced the defect and exposed the root cause: `audit_local_model_inbox.ps1` and `prepare_all_local_inbox_assets.ps1` contained literal Ukrainian text in files stored as UTF-8 without BOM. Windows PowerShell 5.1 interpreted that source as ANSI, converted the Cyrillic to mojibake and then parsed the damaged strings as code.

Repair:

- both affected scripts are now ASCII-only source;
- Ukrainian filename/category matching is preserved through .NET regex `\uXXXX` escapes;
- `audit_local_model_inbox.ps1` uses explicit final `if/else` status assignment;
- `prepare_all_local_inbox_assets.ps1` no longer uses `.NET Core`-only `Path.GetRelativePath`; it uses a Windows PowerShell 5.1 / .NET Framework compatible prefix/substr relative-path calculation;
- the safe ZIP helper call again uses its declared `-Destination` parameter;
- GitHub workflow `Pass 45 local build import regression` now includes a real `windows-powershell51-parse` job so this class of parser regression is caught before another local UE run.

On checkpoint `3c0f29959da2e0c83f076251d925b4f6fd2c7364`, both the source regression verifier and the Windows PowerShell 5.1 parser job completed **SUCCESS**.

### 3.2 Final C++ build — exact cause confirmed and repaired

The latest local compile failure pointed to:

`OCPass45StreetInfrastructureSubsystem.cpp(117,49)`

The code passed `*World` into two `TActorIterator` constructors. UE 5.8 expects `UWorld*`; dereferencing produced a `UWorld&` that MSVC could not convert.

Both iterators now receive `World` directly:

- `TActorIterator<AStaticMeshActor> Existing(World)`;
- `TActorIterator<AOCWorldSectorOster> It(World)`.

This exact regression is now guarded by `VERIFY_PASS45_LOCAL_BUILD_IMPORT_REGRESSION.py`. The source guard is green; factual UE 5.8 compilation remains pending the next local run.

### 3.3 Reporter noise — repaired

`GPUReshape`, `PixWinPlugin`, `RenderDocPlugin`, `WinPixGpuCapturer.dll`, `aqProf.dll` and VTune diagnostics are optional developer/profiler/capture startup chatter. They are not accepted as the primary cause of PASS45 commandlet failure.

The packet reporter now scans the full log and prioritizes:

- `PASS45_*_FAIL` / `PASS45_*_GAP`;
- `[STOP]` / `[ERROR]`;
- Python `Traceback` / `RuntimeError`;
- PowerShell `ParserError` / `UnexpectedToken`;
- compiler `error Cxxxx` / fatal errors / `Result: Failed`.

The optional profiler/capture lines are filtered from the primary cause list, and Windows native `0xFFFFFFFF` is displayed as signed `-1`.

### 3.4 Remington / vehicle / strict material — still factual pending, not guessed

The latest Remington `code=11`, vehicle `code=-1` and strict-material `code=11` occurred in a run where the current C++ source failed to compile. Therefore those UE-dependent stages could have executed against stale compiled modules.

Do **not** convert the PIX/RenderDoc/GPUReshape chatter into a diagnosis. Do **not** claim these three causes are fixed yet.

The next current-head run will rebuild through the aggregate local/Fab stage before subsequent production commandlets. If these stages still fail after a clean current-source build, the repaired packet reporter and wrapper log tails must expose the exact UE/Python fail marker for the next targeted fix.

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
6. use the single report first;
7. if current C++ build is clean but Remington/vehicle/material still fail, use only their new exact fail markers/log tails as the next blockers;
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
