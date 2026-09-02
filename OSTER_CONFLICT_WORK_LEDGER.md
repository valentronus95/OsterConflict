# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

The previous complete ledger remains preserved by Git blob:

`f480ba6dc01de2b41e9d3970f7a91b8b4cee1966`

Use Git history and PASS45 archived ledgers for older implementation detail. This live ledger intentionally stays compact so new sessions resume from the current factual blocker rather than replaying already completed work.

## 1. Current context — 2026-09-02

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Pass 45 remains the active corrective pass.
- Latest Remington strict-runtime implementation head before the current ledger/docs checkpoint: `43588a0adce133608901fc17778d35928c87ea86`.
- PASS45 persistent-history refresh after that implementation: `d39ab31c73b013bf6fe39d288cecf7ff46aee29f`.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- UE target: 5.8.x / Windows.
- Canonical full acceptance launcher: `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ`.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance passes required automated and direct visual/audio gates.
- Local user `Changes` must not be reset, cleaned, overwritten or casually committed.

Current status token:

**PASS45 ACTIVE / ITEM16 OPEN / REMINGTON PRODUCTION SKELETAL+PUMPCYCLE SOURCE-WIRED / STRICT RUNTIME EVIDENCE NOW REQUIRES ACTUAL OC_SG1 PUMP ACTIVATION / CURRENT-HEAD DIRECT UE58 VISUAL+AUDIO ACCEPTANCE PENDING / M700+LEVER AUTHORED ACTION GAPS REMAIN / PR94 UNMERGED**

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure is incomplete.
- `CODED_UNTESTED` — source correction exists but current factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `AUDIO CONTENT GAP` — routing exists but accepted authored sound is absent/unverified.
- `RUNTIME REJECTED` — factual local gameplay disproved the tested result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds on the tested head.
- `VERIFIED RUNTIME` — factual current-head UE/user playtest proves behavior/appearance.
- Green source CI is structural evidence only, not UE runtime acceptance.
- Older runtime evidence remains factual for the head it tested, but cannot prove a later cutover head.
- Historical verifiers may not resurrect retired owners or stale asset paths.

## 3. Latest direct runtime truth — 2026-09-02

The latest direct user run reached gameplay. It proved:

- the gameplay world loaded;
- M700 and Lever Action were present/usable in that tested session;
- Remington 870 fired and showed recoil;
- the Remington fore-end did **not** visibly pump after firing.

That remains a valid rejection of the **pre-production-cutover Remington presentation**:

`GAMEPLAY_REACHED / REMINGTON_RECOIL_PRESENT / REMINGTON_PUMP_PRESENTATION_MISSING`.

Later source work has replaced the old static/generic Remington route with a guarded skeletal production path and authored PumpCycle. Therefore the older runtime result no longer proves the current head's visual outcome. Current-head UE 5.8 runtime evidence is still mandatory.

### Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass44 as a complete solution: spawn/result framing was wrong, the map was still perceived as excessively large/empty, weapon visuals/materials were not production-ready, production-model claims were unreliable, and FPS could collapse severely. Pass45 supersedes Pass44 as the active corrective pass; this historical rejection may not be erased by later source fixes.

### Pass 44 behavior retained unless disproved

The following Pass44 decisions remain protected as non-regression because later evidence did not invalidate them:

- compact central-Oster hard extent: approximately 960×940 m, never restore the historical 2.4 km battlefield;
- normal local gameplay defaults to zero implicit filler bots unless explicitly requested;
- Museum BASE acceptance must be based on the actual live pawn, not source-only spawnpoint existence;
- tactical-map bounds follow the compact central-Oster reference rather than legacy peripheral component auto-fit;
- grey/BasicShape weapon material repair is forbidden; authored material gaps remain fail-visible;
- the retired Pass37 weapon palette compatibility owner stays physically deleted, not preserved as an inert shell.

Pass44 historical non-regression does **not** authorize resurrection of any owner/repair path that Pass45 physically retired.

### Historical local UE build/import rejection — 2026-08-25

**LOCAL UE BUILD REJECTED.** The factual local UE 5.8.1 / MSVC 14.51 run exposed two independent regressions:

- tactical-map `FVector2D` reference-road data declared `constexpr` failed with compiler error `C2131`;
- HMMWV/M2 GLB Interchange intake attempted the deprecated `auto_detect_mesh_type` editor property and was rejected by current UE 5.8.

The source fixes now use a normal `const` road table and the current explicit static-mesh Interchange policy. Their status remains **CODED_UNTESTED** until a later factual local UE build/import run proves them; source CI alone is not that acceptance.

## 4. Binding reuse-first / non-regression rules

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

Protected rules:

- one runtime responsibility has one mutating owner;
- authored manual-action animation follows the existing replicated mechanical-action cycle, not a second timer;
- retired procedural whole-weapon/arms manual-action fallback stays physically retired;
- primary registered Remington donor is exhausted before any second donor is promoted;
- unverified licensing cannot be promoted;
- direct gameplay evidence outranks source-only READY claims;
- source/docs work never inflates canonical checklist progress.

## 5. Item 16 current source state

Item 16 requires accepted authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus local UE 5.8 acceptance.

### Remington 870

Registered donor remains the CC-BY-4.0 8sianDude payload:

`SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb`

SHA-256 / LFS OID:

`147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`

The deterministic derivative isolates the physical fore-end into `PASS45_PumpForeEnd` and provides standalone `PASS45_Remington870_PumpCycle` near 0.55 s. The separate local `Remington_870_FREE.glb` remains quarantined and is not the production donor.

Current guarded production source now exists:

- importer: `PASS45_REMINGTON870_PRODUCTION_UE58_IMPORT.py`;
- wrapper: `OsterConflict/PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd`;
- fresh-load verifier: `OsterConflict/Scripts/verify_remington870_production_fresh_load.py`;
- skeletal visual: `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- pump sequence: `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

Runtime source now:

- loads Remington through the skeletal production visual path;
- maps `OC_SG1` / PumpAction to the exact production PumpCycle;
- reuses the existing `bActionCycling` transition and authored animation bridge;
- keeps `runtime_acceptance=0` until factual runtime observation.

The old ledger statements that the production Remington package is absent, the runtime visual is still generic/static, or the `OC_SG1` authored action path is empty are now **superseded source history** and must not drive future implementation.

### Strict runtime gate

The existing canonical runtime evidence verifier now requires a factual gameplay activation:

`PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY weapon=OC_SG1`

with:

- PumpAction identity;
- exact production PumpCycle path;
- replicated gate ownership;
- no second gameplay timer;
- explicit `runtime_acceptance=0` until manual acceptance.

It fails closed on Remington authored bridge failure, authored content gap, or manual-action audio content gap. The strict harness contract-checks that these Remington requirements cannot silently disappear.

Implementation heads:

- `768c4a379438e976c7d0f9365b6ec7a0f89dcd81` — Remington pump runtime evidence gate;
- `43588a0adce133608901fc17778d35928c87ea86` — strict harness binding.

### Remaining item-16 gaps

- Remington current-head direct visible pump acceptance: **PENDING UE 5.8 RUNTIME**.
- Remington current-head direct mechanical-audio acceptance: **PENDING UE 5.8 RUNTIME**.
- M700 exact authored bolt animation: **CONTENT GAP**.
- Lever Action exact authored lever animation: **CONTENT GAP**.
- Item 16 remains **UNCHECKED**.

## 6. Current CI truth

Exact-head GitHub Actions for the `43588a0...` implementation head started and the large matrix was partially complete at the last check. Some jobs were already successful while others were still running. No in-progress CI state is treated as final PASS.

The documentation checkpoint creates a newer head, so final exact-head CI status must always be read from the latest PR head before any merge or acceptance claim.

## 7. Next factual operation

Run the canonical **current-head** full acceptance route:

`START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ`

During gameplay, fire/cycle the `OC_SG1` Remington 870. A Remington slice PASS requires the same current head to prove:

1. guarded import/build/gameplay/evidence gates complete;
2. actual `OC_SG1` authored bridge READY reaches the exact production PumpCycle;
3. no Remington animation/content/audio gap marker appears;
4. the first-person fore-end visibly performs the post-shot pump cycle;
5. pump mechanical audio is audibly synchronized to that cycle;
6. direct screenshot/observation evidence is retained.

After that, item 16 still requires the missing M700 bolt and Lever Action lever authored animation slices. Do not skip them.

## 8. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Do not merge without current-head UE 5.8 runtime acceptance.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- Local user `Changes`: **DO NOT TOUCH**.