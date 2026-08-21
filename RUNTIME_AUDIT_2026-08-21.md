# OsterConflict persistent runtime audit — 2026-08-21

## Purpose

This file is the persistent runtime truth for the 2026-08-21 batch of 14 gameplay screenshots and the immediate source audit that follows it.

It exists separately from `BRANCH_AUDIT_2026-08-21.md` because branch/source history and runtime evidence answer different questions. A commit, branch comparison, structural verifier or code comment can prove that code exists; it cannot prove that the player actually receives the intended result in UE runtime.

## Authority rule

For the defects listed here, observed runtime state overrides older optimistic ledger/report wording.

Until a fresh UE 5.8 current-`main` playtest proves the corresponding behavior:

- do not call the item `DONE`, `VERIFIED`, fixed or ready for acceptance;
- use `IN_PROGRESS` when runtime already shows the defect;
- use `CODED_UNTESTED` only for code that has not yet been contradicted by runtime;
- one landmark/site must have one authoritative placement owner;
- cleanup after the fact is not a substitute for ownership/exclusion rules.

## Runtime findings from the 14-screenshot batch

### RT-01 — weapon presentation still exposes proxies

Observed runtime evidence contains primitive/proxy presentation for at least:

- M249;
- M1911;
- MAC-10.

Previous code-only claims that fallback/debug geometry was hidden are therefore not accepted as current runtime truth.

Acceptance gate:

- each implemented weapon resolves its intended production presentation or a non-visible safe fallback;
- no sphere/box/rectangular debug/proxy geometry is visible in normal gameplay;
- all weapon classes are checked in the same current-main playtest.

### RT-02 — BTR production asset is not reliably active

Observed runtime shows a visible green box/proxy where the BTR production vehicle should be.

Likely investigation scope, not yet a diagnosis:

- production asset path resolution;
- runtime validation/fallback path;
- mesh assignment timing;
- actor/component replacement ordering.

Acceptance gate:

- the BTR is represented by the intended production asset in normal gameplay;
- no visible box proxy remains outside an explicit diagnostic mode.

### RT-03 — production character/skin pipeline is not runtime-proven

The current runtime evidence does not prove stable production character/skin binding. Existing character/skin assets must be inventoried and canonical runtime profiles must resolve them consistently.

Acceptance gate:

- player and relevant bots resolve production character/skin assets;
- missing asset paths are reported explicitly;
- normal gameplay does not silently substitute visible primitive/debug bodies.

### RT-04 — `LocationTest=1` / weapon-spawn contract is not authoritative yet

The screenshot batch still exposes a legacy weapon placement/rack around the old world-space area near `930000,-500000`, while the required contract is a test rack next to the actually possessed/deployed gameplay pawn.

This means the existence of spawn-relative code in history is not enough. The active `LocationTest=1` route and runtime spawn/rack path must be traced from launch through possession.

Acceptance gate:

- `LocationTest=1` opens the intended current-main runtime map/path;
- one rack is created from the actual possessed/deployed pawn transform;
- all 11 implemented pickup classes are present and individually usable;
- no legacy fixed-coordinate test rack is visible or authoritative.

### RT-05 — legacy blockout still conflicts with landmark areas

Runtime evidence still shows legacy/procedural blockout capable of overlapping or contaminating the Museum / Silpo / Culture House scope.

Source-level separation systems may exist, but runtime evidence does not yet prove that late placement systems respect them.

Acceptance gate for each landmark:

1. exactly one authoritative placement owner;
2. a protected/exclusion zone that other placement systems must respect;
3. no late subsystem may recreate foreign geometry in that zone;
4. validation runs after startup placement passes and detects conflicting actors/components;
5. the result survives current-main UE playtest without flicker/rebuild after reveal.

### RT-06 — one-shot cleanup can run before a late spawn

A cleanup pass that removes legacy geometry once is insufficient if another subsystem can create that geometry later.

Required architecture:

- ownership prevents invalid creation;
- exclusion checks are applied at placement time;
- post-startup validation verifies the final world;
- cleanup remains a recovery tool, not the main correctness mechanism.

### RT-07 — vehicle exit does not restore the complete player input state

Runtime evidence shows that after leaving a vehicle the character does not reliably regain the complete gameplay control state.

The exit path must restore, as one coherent state transition:

- game input mode (`GameOnly` or the project-equivalent normal gameplay state);
- keyboard movement/WASD;
- sprint;
- mouse look;
- camera/control rotation;
- correct possession/focus/cursor state.

Acceptance gate:

- enter vehicle → drive/use vehicle → exit → immediately walk, sprint and mouse-look normally;
- repeating the cycle does not degrade input state.

### RT-08 — `M` does not satisfy the tactical-map contract

Runtime evidence shows that `M` is either bound to another action, blocked by another input layer, or does not open the intended tactical map.

Acceptance gate:

- `M` opens the tactical map from normal gameplay;
- `M` or the intended close action closes it;
- input focus/cursor state is correct while map is open and after close;
- no duplicate binding steals the key.

## Source fixes already identified elsewhere, but not sufficient to close this audit

`BRANCH_AUDIT_2026-08-21.md` separately identified three source/history regressions:

- normal frontend → TEAM gameplay path vs Sandbox diagnostic launch;
- neutral daylight baseline vs the regressed extreme sun/fog setup;
- AK first-person production-mesh axis/yaw correction.

Those fixes remain useful. They do not close RT-01 through RT-08 without a fresh runtime pass.

## Mandatory execution order

Do not reorder this backlog to add new decorative work.

1. `OSTER_CONFLICT_WORK_LEDGER.md` — runtime-correct statuses.
2. `RUNTIME_AUDIT_2026-08-21.md` — this persistent evidence file.
3. `LocationTest=1` — trace and correct the current-main direct runtime-map contract.
4. Spawn/weapon rack — actual possessed/deployed pawn + all 11 pickups.
5. Production assets — weapons, BTR/vehicles, character/skins.
6. Vehicle input — symmetric enter/exit state restoration.
7. Tactical map — `M` binding, focus and open/close behavior.
8. Museum / Silpo / Culture ownership and exclusion zones.
9. Audit the remaining legacy blockout and late runtime placement systems.

## Current test gate

The project is **not** considered runtime-fixed or acceptance-ready at this point.

The next meaningful proof is a UE 5.8 current-`main` playtest after the above code corrections. For location/runtime debugging the first gate is the corrected `LocationTest=1` path. Normal frontend → TEAM gameplay must then be checked before any affected item becomes `VERIFIED`.

## Explicit non-goal

No new decorative R15/R16 subsystem/layer is to be introduced while RT-01 through RT-08 and the landmark ownership cleanup remain open.
