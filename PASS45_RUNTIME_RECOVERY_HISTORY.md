# PASS45 Runtime Recovery — Persistent Work History

This file is the durable human-readable work ledger for `PASS45_RUNTIME_RECOVERY_TZ.md`.

It complements Git history; it does not replace it. The purpose is to make it possible to answer, without reconstructing hundreds of commits, **what was changed, on which branch/PR, what reached `main`, what remains source-only, what requires UE 5.8 runtime acceptance, and what auxiliary branches/PRs still exist**.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- Active integration PR: **#94**
- Target branch: `main`
- Recovery audit snapshot before this ledger was created: `cfb452a0a7d24ad8daebb737864b4358ef624b9d`
- `main` at that snapshot: `bca00f4046700f383af9f1742cc24b6a62401b1a`
- Integration branch relation to that `main`: **ahead 509 / behind 0** at the snapshot.
- Latest factual local runtime verdict carried by PR #94: **RUNTIME REJECTED 2026-08-27**.
- Merge rule: **PR #94 must remain OPEN / UNMERGED until a current-head local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/visual acceptance.** Green GitHub source CI is not runtime acceptance.

## Recording rule from 2026-08-30 onward

Every substantive PASS45 work cycle must append or update this ledger before being reported as finished. Each entry should record:

1. date;
2. canonical checklist item(s);
3. branch and PR;
4. start head and end/source head;
5. commit SHA(s) and short purpose;
6. production/source files materially changed;
7. verifier/workflow changes;
8. exact CI state for the indexed head;
9. runtime evidence state separately from CI;
10. whether anything was merged to `main`;
11. blockers/content gaps left open;
12. any auxiliary branch/PR that must later be merged, superseded, or closed.

A source improvement may be recorded as `SOURCE-CODED` or `SOURCE-VERIFIED`; it may never be recorded as runtime accepted without factual UE 5.8 evidence.

## Current integration state — recovery audit 2026-08-30

### PR #94

- Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- Snapshot head: `cfb452a0a7d24ad8daebb737864b4358ef624b9d`
- State: OPEN / UNMERGED
- Snapshot compare to `main@bca00f4046700f383af9f1742cc24b6a62401b1a`: ahead 509 / behind 0.
- This means the active PASS45 integration line contains all 509 commits in its own history relative to that main snapshot, but does **not** mean every older divergent PR is safely disposable; those require content-equivalence audit first.
- Latest substantive source head after the legacy-branch/content-truth cycle below: `e90a6b57925da5e7cc8c09a198a65e7de989025b`.
- Pre-ledger head for the completed PR #85 equivalence audit: `80805539c97c2bb8bff1028b98843dd373f0bf7f`; that audit introduced no gameplay/runtime source change.

### Known recovered milestones in the active integration line

| Area | Commit / milestone | Recorded result | Runtime status |
|---|---|---|---|
| ParkPaths / Gate K | `0c5d0e769f93c6991ae1065f247b52d9136e13ec` and later guards | exactly five central-park path proxies separated from `Sidewalks`; authored `SM_Stonepath_Var01` upgrade path guarded | pending UE visual acceptance |
| Block0 regional ground detail | milestone around `a481bda13c30a67867a66751f8bb64b3e9993fdd` | blocked water/road/roof/building/concrete/path/bridge placement; bounded deterministic detail pass; dedicated verifier/workflow | source verified; runtime visual acceptance pending |
| Manual bolt/pump/lever truth | `806519498724426578b2b951eb39845a8c553f20`, `a707705de70f0a45ad5cc375429e04c834ccca22` | removed false `PASS45_MANUAL_ACTION_PRESENTATION_READY`; procedural whole-transform cue now reports fallback/content gap honestly | authored moving-part animation/audio still open |
| Museum approach / conifers | `7ebc1cb03d2aa445bdaaad7fd26b50a95b92c194`, `088766de8f70d1538a3fdf06560f52869462fd68` | authored pale approach and conifer corridor bound to Museum owner; no ParkPaths stone path reuse; no exact-species/coordinate claim | `MUS-CAM-01..07` still required |
| Museum deciduous/litter continuation | `736d7993...`, `1e2e05f0...` | mixed deciduous periphery and ground litter source slice + regression guard | broader Gate K/runtime visual acceptance still open |
| Smoke gameplay volume | `fe613ea2...`, `27e718e8...`, `e44c7b29...` | replaced XY-only infinite-height smoke containment semantics with bounded 3D gameplay volume + regression guard | smoke look/scale/performance still requires runtime acceptance |
| Character BasicShape proxy retirement | `241cecc8...`, `09b207e5...`, `cc10a3c96e42f7ee228f35b80419780a5f578237` | production-default source-only BasicShape character proxy disabled fail-closed; dedicated verifier/workflow added | production character visual acceptance still separate |
| Vegetation continuation | `492917e005c81f836b2a6fbcd02b696ab3b629b5`, `520bb207c5802942273eb376dc75854ebc5cc157`, `e276df6588761579e02909f187cec193642d1a45` | authored pine-family verifier/runtime-tree intake mapping and foliage workflow trigger aligned | item remains runtime/visual dependent |
| Manual-action/content-gap guard continuation | `cfb452a0a7d24ad8daebb737864b4358ef624b9d` | guards manual-action and weapon catalog content gaps so missing authored content cannot false-pass | content/runtime gaps remain factual |
| Weapon exact-content truth / legacy PR #90 triage | `e90a6b57925da5e7cc8c09a198a65e7de989025b` | current R14 registry now explicitly marks Remington 870 and M249 exact-production payloads `CONTENT GAP / NOT READY` while preserving the newer `exact production OR explicit real fallback` acceptance architecture | exact payload gaps remain; runtime acceptance unchanged |

Abbreviated SHAs above are indexing aids where the full SHA was not recovered in this ledger pass; Git remains the authoritative raw commit record.

## Legacy / auxiliary PR audit

These PRs were found during the 2026-08-30 audit. Current disposition is recorded below; no divergent branch is to be merged or discarded merely because its commits are old.

### PR #84 — `chore/pass45-postmerge-state-sync-2-20260825`

- Head: `c35c868c2014aacd52fe1cd555c5ed31e5636181`
- **CLOSED / UNMERGED on 2026-08-30.**
- Docs-only scope: `OSTER_CONFLICT_WORK_LEDGER.md` and `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Its two unique commits only synchronized the 2026-08-25 post-PR-#83 source state. They contain no production/runtime implementation change.
- Current PR #94 carries later canonical status, later factual runtime evidence (`RUNTIME REJECTED 2026-08-27`), later source work and this persistent ledger.
- Disposition: **SUPERSEDED / CLOSED UNMERGED / NO PORT REQUIRED**. Merging #84 would restore stale 2026-08-25 wording rather than recover unique required work.

### PR #85 — `fix/pass45-postmerge-content-closure-20260825`

- Head: `24b8f5a8b869ede9bd64be1faddec8e9e52dd414`
- **CLOSED / UNMERGED on 2026-08-30.**
- Scope: 22 commits / 13 changed files covering generic residential decorator retirement, an `OCEnterableHouse` yard/fence/shed slice, BTR material dependency logic, runner/verifier/workflow updates and historical evidence bookkeeping.
- Commit ancestry against current PR #94 is diverged, so the branch was audited semantically before closure rather than discarded by commit count.
- The old generic residential decorator edits are superseded more completely: current #94 physically no longer carries the old `OCAssetModelDecorator` owner, while `VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` forbids normal-game `AOCEnterableHouse` spawning, removes procedural residential owners and observation-only Gate E rejects any resurrection of `AdvancedVillagePack`, `OCEnterableHouse`, steep-roof/shack/tower families and generic private-fence instance families.
- The old `OCEnterableHouse` `RealYardFence` / `RealSideShed` removal was a valid intermediate fix but is not ported into a normal-game owner that the current architecture has retired entirely. Porting it would maintain a dead presentation path rather than restore required runtime behavior.
- The old BTR `Material -> Texture2D` dependency gate is intentionally not ported. Current canonical R3 BTR is a repository-safe authored glTF with explicit PBR material `M_BTR4_OC_Authored` and authored `COLOR_0` vertex palette. PASS45 item 14 / Gate G requires no white/default material, stable ownership, +X-forward/Y-up orientation, proportional presentation and remote optic; the strict `real texture dependencies / zero texture dependencies = FAIL` chain is the weapon-content rule in item 18, not a BTR requirement.
- The BTR white/default failure mode remains guarded on #94: `ValidateProductionBTR4MaterialState()` revalidates around possession lifecycle, rejects null/DefaultMaterial/BasicShape material slots, requires BTR-authored production material paths and fails closed by hiding an invalid production chassis. The separate R3 authored-material and axis/remote-optic gates remain source-verified architecture, not runtime acceptance.
- Older runner/world/stale-runtime verifier wiring on #85 is superseded by the current strict runtime harness plus reference-driven residential retirement and current evidence/TZ contracts.
- Disposition: **FULLY SUPERSEDED / CLOSED UNMERGED / NO PORT REQUIRED**.

### PR #90 — `fix/pass45-content-gap-truth-20260825`

- Head: `4d6817b4f929c345701c6230a23816d10ad6f554`
- **CLOSED / UNMERGED on 2026-08-30.**
- Its still-valid semantic requirement was partially missing from the current human-readable asset registry: Remington 870 and M249 exact-production payloads must remain explicit `CONTENT GAP / NOT READY` items.
- That truth was ported to current `R14_MODEL_REGISTRY.md` on PR #94 in commit `e90a6b57925da5e7cc8c09a198a65e7de989025b`.
- The old #90 exact-path preflight and verifier were intentionally **not** ported: they hard-failed final acceptance solely on absence of the canonical Remington/M249 `.uasset` and required an all-exact `11/11 production weapon classes PASS` sentinel.
- Current Pass45 architecture is newer: `exact production OR explicit real fallback`; exact payload absence remains an explicit content gap, while a real fallback must satisfy the same authored material/texture/fresh-load/runtime visual gates. The current strict harness explicitly rejects resurrection of the obsolete all-exact sentinel.
- Disposition: **PARTIALLY PORTED (registry truth) / REMAINDER SUPERSEDED / CLOSED UNMERGED**.

### PR #95 — `content/free-gameplay-assets-intake-20260828`

- Head: `ffcb6294665942cd755d3ce8f5a16a3c1b2e0799`
- DRAFT / OPEN / UNMERGED.
- Base is the PASS45 integration branch, not `main`.
- Scope is intentionally separate content acquisition/provenance tooling: arms/audio/ambience/vehicle sound candidates; it does not itself change active runtime implementation.
- Commit ancestry comparison against active #94 snapshot is **diverged**; #95 has 4 unique commits relative to the active line because the integration branch continued moving after the content branch split.
- Required disposition: keep isolated until provenance/license/local acquisition/import validation is accepted; then deliberately rebase/port/merge the desired intake work. Never lose it merely because #94 advances.

## Work cycle — 2026-08-30 legacy-branch triage + weapon content truth

- Canonical checklist relevance: item 18 weapon material/texture/content-gap truth plus final branch-hygiene debt.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `9aae675efa13eecc062f3171c7edfaad142c1d1c`.
- Substantive end/source head before this ledger-only bookkeeping write: `e90a6b57925da5e7cc8c09a198a65e7de989025b`.
- Substantive commit: `e90a6b57925da5e7cc8c09a198a65e7de989025b` — reconcile `R14_MODEL_REGISTRY.md` with current exact-production `CONTENT GAP` truth for Remington 870/M249 without resurrecting obsolete exact-path-only acceptance semantics.
- Material source/document file changed: `R14_MODEL_REGISTRY.md`; no gameplay/runtime implementation file was changed in this slice.
- Verifier/workflow changes: none committed in this cycle. Existing current strict harness, material/dependency gates, residential retirement guard and BTR authored-material/axis guards were audited against legacy PR intent.
- Exact CI for `e90a6b57925da5e7cc8c09a198a65e7de989025b`: every workflow run returned by the commit audit completed **SUCCESS**, including `Source verification`, `Pass 45 strict runtime acceptance harness`, `Pass 45 weapon material dependency audit`, `Pass 45 BTR4 authored material`, `Pass 45 BTR4 material state`, `Pass 45 BTR4 axis remote optic`, `Pass 45 visual fidelity Gate K` and `Pass 45 reference-driven residential retirement`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains the latest factual local UE verdict. The new source head has no current-head UE 5.8 runtime acceptance.
- `main` merge state: **none**; PR #94 remains OPEN / UNMERGED.
- Legacy disposition completed at this stage: PR #84 closed superseded with no port; PR #90 closed after porting the still-valid registry truth.
- Branch debt at the end of this specific stage was PR #85 pending BTR/material equivalence audit plus isolated Draft #95; the following cycle resolves #85.
- Remaining technical blockers are unchanged by this bookkeeping/content-truth slice: authored/manual weapon-action and exact audio/ADS closure, item 24 grenade presentation closure, vegetation/world fidelity, HMMWV/M2 runtime acceptance and road speed, BTR material/orientation/remote optic runtime acceptance, fullscreen/native-scale/thermal soak, tactical-map screenshot, full current-head UE 5.8 acceptance and final protected merge.

## Work cycle — 2026-08-30 PR #85 content-equivalence closure

- Canonical checklist relevance: item 14 BTR material/orientation/remote-operator truth, Gate E reference-driven residential retirement and final branch-hygiene debt.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `80805539c97c2bb8bff1028b98843dd373f0bf7f`.
- End/source head before this ledger-only write: `80805539c97c2bb8bff1028b98843dd373f0bf7f`; **no gameplay/runtime source commit was needed in this audit cycle**.
- External PR mutation: PR #85 was closed unmerged after semantic equivalence audit; its historical head remains `24b8f5a8b869ede9bd64be1faddec8e9e52dd414`.
- Old files audited from #85 included `OCAssetModelDecorator.*`, `OCEnterableHouse.*`, `verify_production_vehicle_fresh_load.py`, `VERIFY_PASS45_CONTENT_DEPENDENCIES.py`, stale/world verifiers, runtime runner and workflow wiring.
- Current contracts audited included `PASS45_RUNTIME_RECOVERY_TZ.md`, `VERIFY_PASS45_BTR4_AUTHORED_MATERIAL.py`, `VERIFY_PASS45_BTR4_MATERIAL_STATE.py`, `OCBTR.cpp`, `VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` and `OCGameMode.cpp`; the old `OCAssetModelDecorator` header is absent from the active integration tree.
- Equivalence result: generic residential content is retired more completely by the current architecture; dormant old `OCEnterableHouse` yard presentation does not need a port because normal-game spawning is forbidden; old BTR Texture2D dependency semantics conflict with the current R3 vertex-color PBR material contract and are not part of item 14/Gate G.
- BTR safety retained: current material-state validation rejects null/default/BasicShape/non-BTR material ownership and fails closed across possession lifecycle. This is source architecture only; 2026-08-27 rendered BTR rejection remains authoritative until a new current-head local test.
- Verifier/workflow changes: none committed in this cycle; the audit demonstrated that the current newer contracts supersede #85 rather than requiring another verifier copy.
- CI state: no new substantive source head was created. The latest substantive source commit `e90a6b57925da5e7cc8c09a198a65e7de989025b` remains the exact-head source-verified state recorded in the previous cycle. This ledger-only commit does not authorize any runtime promotion.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains unchanged; no current-head local UE 5.8 acceptance exists.
- `main` merge state: **none**; PR #94 remains OPEN / UNMERGED.
- Legacy branch debt after this cycle: #84 CLOSED superseded, #85 CLOSED fully superseded, #90 CLOSED after partial truth port; only #95 remains intentionally DRAFT/OPEN as isolated content-intake/provenance work.
- Official checklist accounting remains **22/36 = 61.1%** because no runtime-dependent canonical item was factually accepted by this source/branch audit.
- Remaining technical blockers: authored/manual weapon-action and exact audio/ADS closure, grenade presentation closure, vegetation/world fidelity, HMMWV/M2 runtime acceptance and road speed, BTR material/orientation/remote optic runtime acceptance, fullscreen/native-scale/thermal soak, tactical-map screenshot, full current-head UE 5.8 acceptance and final protected merge.

## Work cycle — 2026-08-30 item 24 authored grenade throw audio continuation

- Canonical checklist relevance: item 24 grenade first-person presentation/audio continuation; this slice does not close the still-missing authored hand/pull/throw/recover animation, exact per-type bodies, flash world VFX or direct smoke visual acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `164736c8f693deb739817e577786abba1d0560ac`.
- Substantive source head: `5c8037feda054e435ad734e1194bc69b6fd1fc5a` — `PASS45: wire authored grenade throw audio`.
- Production change: `UOCCharacterVisualComponent` now loads committed `/Game/R13/Audio/snd_throw1`, plays it only from the replicated factual successful-throw presentation event, uses local 2D playback for the owning first-person player and world-location playback for remote presentation, and performs no dedicated-server audio work.
- Authority boundary: input-before-spawn does not play the sound; failed/blocked projectile spawn still consumes zero inventory and emits no throw presentation. Audio remains cosmetic (`gameplay_authority=0`) and owns no fuse, inventory or projectile timing.
- Runtime truth markers: success emits `PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY`; missing/unloadable authored content emits `PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP`, which the dedicated and general strict runtime evidence paths reject.
- Verifier/workflow changes: the existing grenade throw animation gate now also guards the tracked sound asset, native local/remote playback routes, runtime READY/CONTENT GAP contract and general strict evidence wiring. No duplicate workflow was added.
- Local verification: `VERIFY_PASS45_GRENADE_THROW_ANIMATION_GATE.py`, `VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` and cumulative `RUN_ALL_VERIFY.py` all passed; `git diff --check` passed.
- Exact GitHub CI for `5c8037feda054e435ad734e1194bc69b6fd1fc5a`: all **68/68** returned workflows completed **SUCCESS**, including `Source verification`, `Pass 45 grenade throw animation gate`, `Pass 45 strict runtime acceptance harness`, `Pass 45 grenade smoke primitive retirement`, `Pass 45 flash grenade acceptance gate` and `Pass 45 visual fidelity Gate K`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. This source slice has no local UE 5.8 audibility or direct visual acceptance.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. The source head is 525 commits ahead and 0 behind `main@bca00f4046700f383af9f1742cc24b6a62401b1a` before this ledger-only update.
- Official checklist accounting remains **22/36 = 61.1%**. Item 24 stays unchecked because audio source integration cannot substitute for accepted authored first-person animation, distinct flash world VFX, exact/distinct grenade presentation and UE 5.8 smoke/visual acceptance.

## Main-merge history relevant to PASS45

Known historical merged milestones include:

- PR #83 merged to `main` at `f5e883fb69ae8bdd35c754dc895d8b06e4843e08` as a Pass45 runtime-recovery source milestone.
- PR #89 merged to `main` at `a375f52635fbe9fa07c1000aa706e28c53eb42f4` for the strict runtime acceptance harness.
- The current PR #94 is intentionally **not merged** because the latest factual local runtime remains rejected and current-head UE 5.8 acceptance has not yet been produced.

The active `main` snapshot at the 2026-08-30 audit is newer than those historical merge points: `bca00f4046700f383af9f1742cc24b6a62401b1a`.

## Current checklist accounting

At the last canonical checklist count before this recovery audit:

- total checklist items: 36;
- checked: 22;
- open: 14;
- official progress: **61.1%**;
- remaining: **38.9%**.

Source work on runtime-required items does not increase this official percentage until the checklist's factual acceptance condition is met.

Major remaining runtime-heavy blocks include authored/manual weapon-action closure, weapon content/material gaps, vegetation/world visual fidelity, HMMWV M2 hierarchy runtime acceptance, HMMWV road speed, BTR material/orientation/remote optic, fullscreen/native-scale/thermal soak, tactical-map screenshot, current-head full runtime test and protected merge.

## Final closure checklist for branch hygiene

Before PASS45 is declared finished, perform all of the following in addition to the canonical technical acceptance:

- re-read this history ledger and the canonical TZ;
- enumerate all open PRs/branches containing `PASS45` or based on the active integration branch;
- content-diff every divergent legacy PR against the final accepted integration head;
- port any unique still-valid change deliberately;
- explicitly close/supersede obsolete PRs instead of leaving them ambiguous;
- verify the accepted integration head is the exact head merged to `main`;
- record final merge commit in this ledger;
- record post-merge `main` SHA;
- record the exact local UE 5.8 runtime evidence set that authorized the merge;
- leave no unclassified PASS45 branch carrying required production changes.

## Status vocabulary

Use only these meanings:

- `SOURCE-CODED`: implementation exists in source.
- `SOURCE-VERIFIED`: relevant source/CI verification passed for the exact indexed head.
- `CONTENT GAP`: required authored content is absent/unaccepted.
- `RUNTIME PENDING`: no current-head local UE 5.8 acceptance exists.
- `RUNTIME REJECTED`: factual local runtime evidence rejected the state.
- `RUNTIME ACCEPTED`: current-head local UE 5.8 import/build/gameplay/evidence/direct visual acceptance passed.
- `MERGED`: exact accepted head was merged to `main` and merge SHA is recorded.

Never collapse these states into one generic `PASS`.
