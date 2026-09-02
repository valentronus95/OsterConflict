# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The immediately preceding complete history is preserved at:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_CURRENT_HEAD_PREFLIGHT_2026-09-02.md`

Preserved Git blob: `798b6b3b35e09cc4a4fce600dcaaf6afce7f9bf6`.

Earlier history remains preserved in Git. Git history is the raw source of truth; this live file intentionally records only the newest factual state needed to continue without replaying completed work.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target baseline: `main@bca00f4046700f383af9f1742cc24b6a62401b1a` at this checkpoint.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Latest substantive source-gate head before this ledger update: `43588a0adce133608901fc17778d35928c87ea86`.
- PR #94 must remain unmerged until a current-head local UE 5.8 full runtime run passes the still-open content/runtime gates and direct visual/audio acceptance.
- Official canonical checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source/docs/CI preparation does not increase it.
- Local user `Changes` remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items while item 16 still contains factual content/runtime blockers.

## Binding reuse-first state

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

- Primary Remington donor remains the registered CC-BY-4.0 8sianDude payload; do not silently promote the quarantined `Remington_870_FREE.glb` candidate.
- Authored manual actions stay synchronized to the existing replicated mechanical action cycle. Animation/audio must not become a second gameplay timer.
- The retired procedural whole-weapon/arms manual-action fallback must not return.
- Compile/source green is not runtime acceptance.

## Latest direct user runtime evidence — 2026-09-02

The latest direct user playtest reached gameplay successfully. Observed in that run:

- gameplay world loaded;
- M700 and Lever Action were present/usable;
- Remington 870 fired and showed recoil;
- **Remington 870 did not show a visible pump/fore-end cycle after firing**.

That run therefore remains a factual **runtime rejection for the pre-cutover Remington presentation**:

`GAMEPLAY_REACHED / REMINGTON_RECOIL_PRESENT / REMINGTON_PUMP_PRESENTATION_MISSING`.

Important chronology: that observation predates the later production Remington skeletal/animation cutover now present in source. It proves the old presentation failed. It does **not** prove that the current head visually succeeds or fails. A new current-head local UE 5.8 run is still mandatory.

## Registered Remington donor and derived pump source

Primary source remains:

- `SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb`;
- exact SHA-256 / LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- exact payload size `20,621,580` bytes;
- creator: 8sianDude;
- license: CC-BY-4.0;
- register entry: `PASS45-3P-WEAPON-001`.

Existing exact-donor UE 5.8 evidence proves imported articulated motion and addressable weapon-side moving tracks. Direct `Pmag_061 = pump` mapping remains rejected because that source node is composite geometry.

The deterministic derivative isolates the probable physical fore-end:

- fore-end partition: **48 components / 1170 vertices**;
- side-saddle/other remainder: **58 components / 3241 vertices**;
- derived joint: `PASS45_PumpForeEnd`;
- standalone action: `PASS45_Remington870_PumpCycle`;
- duration: **0.55 s**.

The derivative is still a derivative of the registered primary donor, not a second donor.

## Remington production source state — current head

The old checkpoint saying `production Remington package ABSENT` and `ManualActionAnimationObjectPath NOT AUTHORIZED` is superseded by later guarded source work.

Current source now contains the production path:

- fail-closed UE 5.8 importer: `PASS45_REMINGTON870_PRODUCTION_UE58_IMPORT.py`;
- canonical wrapper: `OsterConflict/PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd`;
- fresh-load verifier: `OsterConflict/Scripts/verify_remington870_production_fresh_load.py`;
- expected production skeletal asset: `/Game/Production/Weapons/Remington870/SKM_Remington870.SKM_Remington870`;
- expected production pump sequence: `/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle.AN_Remington870_PumpCycle`.

Runtime source wiring is also current:

- `AOCWeapon_Shotgun::BeginPlay()` requests the production Remington **SkeletalMesh** rather than the old static/generic shotgun presentation;
- `OC_SG1` / `PumpAction` now points its authored manual-action profile to `AN_Remington870_PumpCycle`;
- the existing `UOCFirstPersonWeaponPresentationSubsystem` consumes the real replicated `bActionCycling` transition and plays the authored sequence on the compatible skeletal production visual;
- the bridge deliberately logs `runtime_acceptance=0`; source activation is not direct visual/audio acceptance.

The full runtime route includes the guarded production intake. Static/source validators were aligned to the skeletal Remington path and PumpCycle contract. Do not resurrect the old static Remington expectations.

## Strict runtime evidence gate — current checkpoint

The existing strict acceptance path now includes Remington instead of leaving this slice outside the main gate.

`VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` now requires an actual gameplay log line for:

`PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY weapon=OC_SG1`

The accepted line must also prove:

- `action=EOCWeaponActionType::PumpAction`;
- exact production `AN_Remington870_PumpCycle` path;
- `replicated_gate=1`;
- `second_gameplay_timer=0`;
- `runtime_acceptance=0`.

The evidence gate fails closed on:

- `PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_FAIL weapon=OC_SG1`;
- `PASS45_MANUAL_ACTION_AUTHORED_CONTENT_GAP weapon=OC_SG1`;
- `PASS45_WEAPON_AUDIO_CONTENT_GAP weapon=OC_SG1 event=manual_action`.

On automated success it emits `REMINGTON870_AUTHORED_PUMP_RUNTIME_BRIDGE=PASS` while preserving `VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION`.

`VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` now contract-checks that all of those Remington requirements remain part of the canonical strict evidence path.

Implementation commits for this gate:

- `768c4a379438e976c7d0f9365b6ec7a0f89dcd81` — `test(pass45): gate Remington pump runtime evidence`;
- `43588a0adce133608901fc17778d35928c87ea86` — `test(pass45): bind Remington pump to strict runtime harness`.

Exact-head GitHub Actions for `43588a0...` started successfully; at the time of this ledger update the large check set was still partially in progress. No CI-in-progress state may be promoted to PASS until its final conclusions exist.

## Current acceptance state

- Registered donor provenance/source intake: **PASS FOR GUARDED USE**.
- Exact registered donor imported-motion evidence: **LOCAL UE 5.8 PASS / IMPORTED-MOTION PROOF ONLY**.
- Direct `Pmag_061 = pump` mapping: **REJECTED**.
- Derived physical fore-end + standalone PumpCycle source: **SOURCE PASS**.
- Production Remington skeletal importer/wrapper/fresh-load proof path: **SOURCE-WIRED / GUARDED**.
- Runtime Remington skeletal visual path: **SOURCE-WIRED**.
- Runtime authored PumpCycle path: **SOURCE-WIRED**.
- Strict automated gameplay evidence requirement for actual PumpCycle activation: **SOURCE-WIRED**.
- Current-head direct Remington visual pump acceptance: **PENDING LOCAL UE 5.8 RUNTIME**.
- Current-head direct Remington mechanical-audio acceptance: **PENDING LOCAL UE 5.8 RUNTIME**.
- Separate `Remington_870_FREE.glb`: **QUARANTINED / NOT PROMOTED**.
- M700 exact authored bolt animation: **CONTENT GAP**.
- Lever Action exact authored lever animation: **CONTENT GAP**.
- Item 16: **UNCHECKED**.
- PR #94: **OPEN / UNMERGED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Run the canonical **current-head** full acceptance route:

`START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ`

During the gameplay portion, fire/cycle `OC_SG1` Remington 870. Acceptance requires all of the following on the same current head:

1. strict importer/build/gameplay/evidence gates complete without Remington production/content/audio gap;
2. `PASS45_MANUAL_ACTION_AUTHORED_SOURCE_BRIDGE_READY weapon=OC_SG1` reaches the exact production PumpCycle path;
3. the fore-end visibly travels through the post-shot pump cycle in first-person gameplay;
4. pump mechanical audio is factually audible and synchronized to that cycle;
5. direct screenshot/observation evidence is retained.

Even a Remington PASS does not close item 16 by itself: M700 and Lever Action authored moving-part animation gaps must still be closed and accepted. PR #94 remains unmerged.