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
- Latest factual local runtime verdict carried by PR #94: **RUNTIME REJECTED 2026-08-31**.
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

## Work cycle — 2026-08-30 item 27 primary tree-authoring ownership continuation

- Canonical checklist relevance: item 27 vegetation replacement and broader environment acceptance. This source slice eliminates an owner conflict; it does not accept the remaining UE 5.8 visual/material/LOD result.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `acf44946ea6dfd0bc5b12f4c4ebb3be8b7ac07f3`.
- Substantive source head: `05037124dce69542f5352850d38e7c102a99b6f3` — `PASS45: make world trees sole primary owner`.
- Production change: `AOCWorldSectorOster` now directly selects final `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01` assets before gameplay. The obsolete `OCTreeContentUpgradeSubsystem.*` was physically deleted, removing the post-`BeginPlay` instance-transform/material rewrite and leaving one mutating tree author.
- Runtime truth: `PASS45_REGIONAL_TREE_INTAKE_WIRED` records `primary_authoring=1`, `late_mutation=0` and `runtime_acceptance=0`; `UOCFoliageRuntimeGuardSubsystem` continues to fail closed when the three exact runtime family identities do not match. Oak remains an explicit unverified content gap.
- Verifier changes: content-runtime wiring, authored-tree mapping, ledger wiring, runtime-recovery and S16B checks now require the primary world owner and reject reappearance of the deleted late-upgrade subsystem.
- Local verification: `git diff --check`, `VERIFY_PASS45_CONTENT_LEDGER_RUNTIME_WIRING.py`, `VERIFY_RUNTIME_RECOVERY_PASS_45.py`, `VERIFY_S16B.py` and cumulative `RUN_ALL_VERIFY.py` passed.
- Exact GitHub CI for `05037124dce69542f5352850d38e7c102a99b6f3`: all **68/68** returned workflows completed **SUCCESS**, including `Source verification`, `Foliage runtime acceptance pass 10`, `Pass45 content ledger runtime wiring`, `Runtime recovery Pass 45` and `Pass 45 visual fidelity Gate K`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 rendered tree/material/LOD acceptance exists.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. The source head is 527 commits ahead and 0 behind `main@bca00f4046700f383af9f1742cc24b6a62401b1a` before this ledger-only update.
- Official checklist accounting remains **22/36 = 61.1%**. Item 27 stays unchecked until direct UE 5.8 screenshots prove natural regional vegetation, no visible primitive/fantasy family, acceptable material/LOD quality and broader environment acceptance.

## Work cycle — 2026-08-30 item 27 Stadion Oster primary-tree continuation

- Canonical checklist relevance: item 27 vegetation replacement and item 32/Gate K source fidelity. This slice removes the remaining legacy tree family from the authoritative stadium owner; it does not replace the still-visible stadium BasicShape sport/entrance families or accept runtime visual fidelity.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `626dc45e14c1db7a5d1ab7582d3997a57304d4ca`.
- Substantive source head: `9d04baab648fe75ccb0e6903365f438c55230609` — `PASS45: author stadium trees from final families`.
- Production change: `UOCR13StadiumSurfaceSubsystem`, the sole Stadion Oster site author, now directly selects tracked `HillTree_02` and `ScotsPineTall_01` perimeter meshes. It no longer selects legacy `SM_Tree_Var01/04`; `PASS45_STADIUM_TREE_INTAKE_WIRED` records `primary_authoring=1`, `late_mutation=0` and `runtime_acceptance=0`.
- Scope boundary: field turf/lines, goals, sports metal and the exact `СТАДІОН ОСТЕР` entrance construction remain explicit content/Gate K work. They were not hidden, recolored, substituted with a generic sign, or otherwise falsely accepted.
- Verifier changes: `VERIFY_R13_STADION_OSTER.py` and Gate K verification require final direct stadium tree authoring and reject the retired legacy tree paths.
- Local verification: `git diff --check`, `VERIFY_R13_STADION_OSTER.py`, `VERIFY_PASS45_AUTHORED_TREE_ASSET_MAPPING.py`, `VERIFY_PASS45_VISUAL_FIDELITY_GATE_K.py`, `VERIFY_RUNTIME_RECOVERY_PASS_45.py` and cumulative `RUN_ALL_VERIFY.py` passed.
- Exact GitHub CI for `9d04baab648fe75ccb0e6903365f438c55230609`: all **68/68** returned workflows completed **SUCCESS**, including `Source verification`, `Stadion Oster runtime evidence pass 9`, `Pass 45 visual fidelity Gate K`, `Foliage runtime acceptance pass 10` and `Runtime recovery Pass 45`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 stadium/perimeter-tree visual acceptance exists.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. The source head is 529 commits ahead and 0 behind `main@bca00f4046700f383af9f1742cc24b6a62401b1a` before this ledger-only update.
- Official checklist accounting remains **22/36 = 61.1%**. Item 27 and item 32 remain unchecked until direct UE 5.8 screenshots prove the complete environment and Gate K's remaining stadium/core-world content.

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

## Work cycle — 2026-08-30 item 32 Stadion Oster BasicShape presentation retirement

- Canonical checklist relevance: item 32 / Gate K world visual-fidelity closure, with item 27 environment presentation as a subordinate visual concern. This slice closes the authoritative Stadion Oster **source-level** BasicShape presentation family; it does not close Gate K globally or accept the runtime image.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `55ae707b3247852e7cd1abeaa6c69ee1d078f658`.
- Substantive source head: `ba34e7dffda7d36696e4562ebffac97ec8b27e08` — `PASS45: retire stadium BasicShape presentation`.
- Production change: `UOCR13StadiumSurfaceSubsystem` no longer loads or owns `/Engine/BasicShapes/` for visible stadium presentation. The pitch uses tracked `SM_Plane_1x1`; pitch markings, goals and sports-metal placement use tracked `SM_Curb_1`; entrance presentation uses tracked `SM_Sign_1`; grass/color/metal presentation uses tracked repository material instances. Existing track, footpath, fence, building and final KiteDemo tree families remain intact.
- Geometry safety: the former cube-only `AddBoxLocal` scale assumption is now bounds-aware against the component's actual static-mesh bounds, including long-axis handling and bounds-origin compensation. This preserves placement intent while allowing non-cube authored meshes.
- Fail-closed content rule: required stadium pitch/bar/sign/material assets are explicitly load-checked. Missing content emits `PASS45_STADIUM_AUTHORED_SURFACE_CONTENT_GAP ... gate_k_complete=0` and aborts site construction rather than restoring a BasicShape fallback.
- Source truth marker: `PASS45_STADIUM_SURFACE_PARTIAL_AUTHORED_READY ... authored_surface_families=7 remaining_basicshape_families=0 bounds_aware_box_fit=1 gate_k_complete=0 runtime_acceptance=0`.
- Verifier change: `VERIFY_PASS45_VISUAL_FIDELITY_GATE_K.py` now forbids `/Engine/BasicShapes/` in the authoritative stadium owner, requires the tracked pitch/bar/sign/material assets and verifies their repository files exist. It deliberately keeps world/Gate K completion open.
- Exact GitHub CI for `ba34e7dffda7d36696e4562ebffac97ec8b27e08`: all **68/68** returned workflows completed **SUCCESS**, including `Source verification`, `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `Stadion Oster runtime evidence pass 9`, `World geometry stability pass 12` and `Runtime recovery Pass 45`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 screenshot proves the selected meshes/materials visually fit the stadium; source CI cannot validate donor appearance, proportions or material parameter behavior in-engine.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. No protected merge is authorized.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked because ParkGeometry, the four semantic park-detail proxy families, remaining non-stadium core BasicShape presentation and direct UE 5.8 Gate K evidence remain open.
- Next source-landable Gate K target: retire/replace the remaining `ParkGeometry` and semantic Central Park BasicShape detail families with exact semantic authored ownership, without blanket-remapping unrelated geometry or hiding rejected components.

## Work cycle — 2026-08-30 item 32 Central Park authored benches continuation

- Canonical checklist relevance: item 32 / Gate K semantic Central Park fidelity. This slice retires only the homogeneous `ParkBenches` BasicShape presentation family; it does not close Gate K globally or accept the rendered park.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `c987ad74d1d246c4f509970630f8d6d13f836bb4`.
- Substantive source commit: `aaa4633bbb81fa6247fd103342cba4d09eafedf8` — `PASS45: author Central Park benches`.
- Verifier-fix head: `4c73561c784479b37c4af6b32cf64f31e2275fde` — `PASS45: fix park semantic source assertions`.
- Production change: new `UOCParkSemanticAuthoredUpgradeSubsystem` replaces exactly the 14 canonical `ParkBenches` Cube proxies with tracked `/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Bench_1.SM_Bench_1`. It preserves the authored bench's native proportions through uniform scaling and preserves source ground-bottom contact instead of deforming the bench into the placeholder box.
- Scope safety: the upgrader owns only `ParkBenches`. It does not mutate `ParkDetails`, `ParkMemorialPlaza`, `ParkMemorialApproach` or `ParkSkateFitness`; missing/unexpected content fails closed rather than silently broadening the family scope.
- Verifier/workflow: added `VERIFY_PASS45_PARK_SEMANTIC_AUTHORED.py` and `.github/workflows/pass45-park-semantic-authored.yml`. The first verifier run exposed an over-literal test assumption about `ExpectedSemanticDetails = 23`; production source correctly computed the value and guarded it with `static_assert`, so commit `4c73561c...` corrected the verifier rather than weakening production truth.
- Exact GitHub CI for `4c73561c784479b37c4af6b32cf64f31e2275fde`: every workflow returned by the exact-head audit completed **SUCCESS**, including `Pass 45 Park Semantic Authored`, `Source verification`, `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `World geometry stability pass 12`, `Pass 45 strict runtime acceptance harness` and `Runtime recovery Pass 45`.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 bench scale/material/site visual acceptance exists.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Remaining park source families after this slice were `ParkGeometry`, mixed `ParkMemorialPlaza`, homogeneous `ParkMemorialApproach` and mixed `ParkSkateFitness`; blanket remapping of mixed semantic groups remains forbidden.

## Work cycle — 2026-08-30 item 32 Central Park memorial-approach authored continuation

- Canonical checklist relevance: item 32 / Gate K semantic Central Park fidelity. This slice retires the homogeneous four-step `ParkMemorialApproach` BasicShape family only; it does not close the mixed plaza/skate families, global Gate K or runtime visual acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `4c73561c784479b37c4af6b32cf64f31e2275fde`.
- Substantive source commit: `e9b4f526be2de9794862b8b5c891b358cfaf7ff4` — `PASS45: author Central Park memorial approach`.
- Workflow-only follow-up head before this ledger write: `1fdfab029e6618eff4127a676baeb5560aca6203` — `PASS45: keep park memorial verifier lightweight`.
- Production change: added `UOCParkMemorialApproachAuthoredUpgradeSubsystem`, which targets only `ParkMemorialApproach`, requires exactly four source instances and replaces their Engine Cube mesh with tracked `/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1`.
- Geometry safety: the replacement uses authored bounds plus the factual source Cube world dimensions for bounds-aware 3D fitting, compensates a Y-long authored mesh axis when needed, preserves source centerline and bottom/ground contact, clears BasicShape material overrides and validates a non-BasicShape runtime material postcondition.
- Fail-closed/scope safety: unexpected source mesh/count/content emits explicit CONTENT_GAP/FAIL markers. The subsystem does not touch `ParkDetails`, `ParkMemorialPlaza`, `ParkSkateFitness` or `ParkBenches`.
- Verifier/workflow: added `VERIFY_PASS45_PARK_MEMORIAL_APPROACH_AUTHORED.py` and `.github/workflows/pass45-park-memorial-approach-authored.yml`. The workflow was immediately trimmed to default checkout because the verifier needs tracked LFS pointer existence, not a wasteful full repository LFS download.
- Exact-head focused CI on `1fdfab029e6618eff4127a676baeb5560aca6203`: `Pass 45 Park Memorial Approach Authored` completed **SUCCESS**; `Pass 45 Park Semantic Authored`, `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `Runtime recovery Pass 45`, `World geometry stability pass 12`, `Source verification` and `Tactical Map 2.0 source contracts` also completed **SUCCESS** in the same exact-head run set.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. This source result is not local UE 5.8 visual acceptance.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Audit finding for the next Gate K work: `ParkMemorialPlaza` is mixed surface+monument geometry, `ParkSkateFitness` is mixed pad+ramp geometry, and `ParkGeometry` is reused by `BuildCollegeSector()`. Therefore none of those families may be blanket-upgraded. The next safe source step is semantic ownership separation before assigning exact authored replacements.

## Work cycle — 2026-08-30 item 32 Central Park mixed semantic owner normalization

- Canonical checklist relevance: item 32 / Gate K Central Park ownership cleanup. This slice separates the two mixed runtime presentation buckets before any further authored replacement; it does not claim that the primary world author is fully split and does not close Gate K.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `3c2cf86e27fbdf58ddc6f56e4b5f15417132280c`.
- Substantive source commit: `eaed7b4e848bdd04cb8509eb364aa0f73e4873d7` — `PASS45: normalize mixed Central Park semantic owners`.
- Verifier-fix source head: `a6d4f50d6175f6b6fd7cc14505a252989eafb98f` — `PASS45: fix park semantic normalization Gate K assertion`.
- Production change: added `UOCParkSemanticOwnerNormalizationSubsystem`, a deliberately temporary `primary_authoring=0` migration bridge. Before Gate K's first observation it preflights the exact current mixed source contract, reads all five transforms, creates exact runtime owners `ParkMemorialSurface`, `ParkMemorialMonument`, `ParkSkateSurface` and `ParkSkateRamps`, copies the five transforms as `1 + 1 + 1 + 2`, then clears only the legacy `ParkMemorialPlaza` and `ParkSkateFitness` instance buckets.
- Transaction/fail-closed rule: legacy mixed instances are not cleared until all four destination components exist and all five exact instances have been copied. Owner/copy failure destroys newly created destinations and leaves the source instances intact. Unexpected source count or non-Cube legacy mesh fails rather than reclassifying arbitrary geometry.
- Scope safety: the bridge never mutates `ParkDetails`, authored `ParkMemorialApproach` or authored `ParkBenches`, never calls `SetVisibility(false)` / `SetHiddenInGame(true)`, preserves geometry rather than manufacturing a visual pass, and logs `gate_k_complete=0 runtime_acceptance=0`.
- Ordering: normalization delay is 0.35 s; Gate K's first observation remains at 3.0 s. Gate K already ignores zero-instance ISMs, so cleared legacy quarantine buckets do not create false BasicShape failures while the four exact destination owners remain visible and honestly subject to later authored-content replacement.
- Verifier/workflow: added `VERIFY_PASS45_PARK_SEMANTIC_OWNER_NORMALIZATION.py` and `.github/workflows/pass45-park-semantic-owner-normalization.yml`. The first focused run exposed an over-literal verifier assertion for Gate K's zero-instance variable; commit `a6d4f50d...` corrected the verifier to the factual `Instances = ISM->GetInstanceCount(); if (Instances <= 0) continue;` contract without weakening production behavior.
- Exact-head relevant CI for `a6d4f50d6175f6b6fd7cc14505a252989eafb98f`: `Pass 45 Park Semantic Owner Normalization`, `Source verification`, `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `Pass 45 Park Semantic Authored`, `Pass 45 Park Memorial Approach Authored`, `Runtime recovery Pass 45`, `World geometry stability pass 12`, `Pass 45 strict runtime acceptance harness` and `Tactical Map 2.0 source contracts` completed **SUCCESS**. At the time this ledger checkpoint was written, the unrelated `Pass 45 BTR4 authored material` run remained scheduler-`pending` with no job; no failure was reported for it.
- Content audit: the tracked Street Props mesh inventory did not expose an obvious exact `Ramp`, `Skate`, `Statue` or `Monument` candidate by name. The new exact owners therefore remain **CONTENT GAP** for authored replacement rather than receiving arbitrary substitute props.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head UE 5.8 runtime evidence exists for this normalization bridge or its eventual authored replacements.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Remaining Gate K park work: replace `ParkMemorialSurface`, `ParkMemorialMonument`, `ParkSkateSurface` and `ParkSkateRamps` only with verified semantic authored content; split/retire the temporary bridge into direct primary authoring when the coordinated world-source change is made; treat `ParkGeometry` separately because that component is also used by `BuildCollegeSector()`; then obtain current-head UE 5.8 visual acceptance.

## Work cycle — 2026-08-30 item 32 ParkGeometry semantic split + authored ground

- Canonical checklist relevance: item 32 / Gate K core-world visual fidelity. This cycle removes the shared `ParkGeometry` ownership ambiguity and retires its three visible BasicShape ground instances through exact semantic owners and an authored ground material path; it does not close remaining memorial/skate content gaps or runtime acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `3b9c0fee33cf33ead6372909871dd3cea25967db`.
- Semantic-normalization source commit: `d241c2be5969f813974121db2087ef98b2427d14` — `PASS45: normalize ParkGeometry semantic owners`.
- Final substantive source head: `65b442210746ee1ce7ef22fecb42270d5f47c731` — `PASS45: author exact park ground surfaces`.
- Ownership change: the exact legacy 3-instance `ParkGeometry` contract (Central Park footprint, north civic grove, college recreation strip) is transactionally re-homed as `ParkCentralGround=1`, `ParkNorthCivicGround=1`, `CollegeRecreationGround=1`; source transforms/mesh/material/collision are preserved until all destinations are populated, then the legacy bucket is cleared. The bridge is explicitly temporary `primary_authoring=0`.
- Tactical-map safety: `OCTacticalMapSubsystem` derives projection from all relevant `UPrimitiveComponent` bounds and has no `ParkGeometry` name dependency, so the exact re-home preserves map geography.
- Authored ground change: after normalization, exactly those three green-ground semantic owners replace Engine Cube presentation with tracked `SM_Plane_1x1` + `M_Grass_Inst`. Bounds-aware fitting preserves XY footprint, source yaw and source top-surface elevation; flat-plane Z is not distorted to mimic cube thickness.
- Transaction safety: all three authored replacements preflight before mutation; old mesh/material/transform state is retained and applied owners are rolled back if a later replacement write/postcondition fails.
- Verifier/workflow changes: added `VERIFY_PASS45_PARK_GEOMETRY_OWNER_NORMALIZATION.py` + `.github/workflows/pass45-park-geometry-owner-normalization.yml`, and `VERIFY_PASS45_AUTHORED_PARK_GROUND.py` + `.github/workflows/pass45-authored-park-ground.yml`.
- Exact-head CI for `65b442210746ee1ce7ef22fecb42270d5f47c731`: `Pass 45 Authored Park Ground`, `Pass 45 Park Geometry Owner Normalization`, `Source verification` (full structural/regression suite), `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `Runtime recovery Pass 45`, `Pass 45 strict runtime acceptance harness`, `World geometry stability pass 12` and `Tactical Map 2.0 source contracts` completed **SUCCESS**. Other unrelated workflows may still have been queued/in progress when this ledger entry was written and are not used to promote runtime status.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 visual acceptance exists for the new ground plane/material fit.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Remaining Gate K park content gaps: `ParkMemorialMonument` and `ParkSkateRamps` still lack verified authored mesh candidates; `ParkMemorialSurface` and `ParkSkateSurface` now have exact ownership and may next receive a separately verified hardscape surface treatment. The temporary runtime ownership bridges must ultimately be retired into direct primary `AOCWorldSectorOster` authoring, followed by current-head UE 5.8 evidence.

## Work cycle — 2026-08-31 item 32 Central Park authored hardscape surfaces

- Canonical checklist relevance: item 32 / Gate K core-world visual fidelity. This source slice retires only the two exact flat hardscape BasicShape families created by the existing semantic normalization bridge; it does not fabricate monument/ramp content and does not close Gate K or runtime acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `5f09519f7a69bac6744f4e8e74c8b5d7a2d5d00b`.
- Substantive source head: `eb289282d5696628bcf82e0c14bd07712a728d6a` — `PASS45: author Central Park hardscape surfaces`.
- Production change: added `UOCParkHardscapeAuthoredUpgradeSubsystem`, targeting exactly `ParkMemorialSurface=1` and `ParkSkateSurface=1` after the 0.35 s semantic-owner bridge. It replaces their Engine Cube presentation with tracked `/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1` plus `/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.M_Concrete_1_Inst`.
- Geometry safety: bounds-aware fitting preserves source XY footprint, yaw and source top-surface elevation; the authored plane is not vertically distorted to imitate the legacy slab thickness.
- Transaction/scope safety: both surface owners preflight before mutation; old mesh/material/transform state is retained and both owners roll back on a failed write/postcondition. `ParkMemorialMonument=1` and `ParkSkateRamps=2` are snapshotted and required to remain unchanged, leaving exactly three explicit authored-content-gap instances rather than substituting arbitrary props.
- Runtime truth markers: `PASS45_AUTHORED_PARK_HARDSCAPE_CONTENT_GAP`, `PASS45_AUTHORED_PARK_HARDSCAPE_FAIL`, and `PASS45_AUTHORED_PARK_HARDSCAPE_READY ... remaining_content_gap_instances=3 gate_k_complete=0 runtime_acceptance=0`. The subsystem contains no `gate_k_complete=1` or `runtime_acceptance=1` path.
- Verifier/workflow: added `VERIFY_PASS45_AUTHORED_PARK_HARDSCAPE.py`, `.github/workflows/pass45-authored-park-hardscape.yml`, and integrated the verifier into cumulative `RUN_ALL_VERIFY.py`.
- Exact-head relevant CI for `eb289282d5696628bcf82e0c14bd07712a728d6a`: `Pass 45 Authored Park Hardscape`, `Source verification` (full structural/regression suite), `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `World geometry stability pass 12`, `Tactical Map 2.0 source contracts`, `Runtime recovery Pass 45`, `Pass 45 strict runtime acceptance harness`, `Pass 45 Authored Park Ground`, `Pass 45 Park Geometry Owner Normalization` and `Pass 45 Park Semantic Owner Normalization` completed **SUCCESS**.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 screenshot accepts the new concrete hardscape look, scale or relationship to monument/ramps.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Remaining Gate K park content gaps: `ParkMemorialMonument` and `ParkSkateRamps` still lack verified authored mesh candidates. The next source-landable architectural task is to retire both temporary `primary_authoring=0` park normalization bridges by moving all exact ground/memorial/skate owners directly into `AOCWorldSectorOster`, updating the authored ground/hardscape contracts to `primary_authoring=1`, and preserving the three explicit monument/ramp content gaps until factual authored assets exist.

## Work cycle — 2026-08-31 item 32 primary park semantic ownership

- Canonical checklist relevance: item 32 / Gate K core-world visual fidelity. This cycle retires the temporary ownership-normalization architecture itself. It does not close the still-missing monument/ramp authored content or current-head UE 5.8 acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `aab6e14e154ce57a9a4cba02e2a685511d4c56c4`.
- Primary-authoring production commit: `bfbcbdbb92a7419caf41214f2a63afb8d9172c6d` — `PASS45: make park semantic owners primary`.
- Verifier alignment commits: `2b34cf4bef1d72a65b221351465b0370332ec70d` — `PASS45: align Gate K with primary park owners`; `148f3127c7d5100d06cda79c03d5456f17901e45` — `PASS45: align park bench guard with primary owners`.
- Primary ownership change: `AOCWorldSectorOster` now creates `ParkCentralGround`, `ParkNorthCivicGround`, `CollegeRecreationGround`, `ParkMemorialSurface`, `ParkMemorialMonument`, `ParkSkateSurface` and `ParkSkateRamps` directly as default subobjects / UPROPERTY-owned semantic families. The exact existing transforms/counts remain 1/1/1 ground and 1/1/1/2 memorial/skate.
- Legacy quarantine: `ParkGeometry`, `ParkDetails`, `ParkMemorialPlaza` and `ParkSkateFitness` remain component names for compatibility but are required to stay zero-instance. No `AddBox` / `AddBoxRotated` path writes player-facing geometry back into those mixed/shared buckets.
- Bridge retirement: `OCParkGeometryOwnerNormalizationSubsystem.*` and `OCParkSemanticOwnerNormalizationSubsystem.*`, their two dedicated source verifiers and their two workflows were physically deleted in the same atomic production commit. There is no parallel primary+late-normalization ownership window.
- Ground source contract: `BuildCentralPark()` directly authors central and north civic grounds; `BuildCollegeSector()` directly authors the college recreation ground. A fail-closed 0/1/1/1 legacy/exact count contract logs `PASS45_PARK_GROUND_PRIMARY_OWNERS_READY` or rejects/clears the exact ground family on mismatch.
- Memorial/skate source contract: `BuildCentralPark()` directly authors memorial surface=1, monument=1, memorial approach=4, skate surface=1, skate ramps=2 and benches=14; total detail contract remains exactly 23. It logs `PASS45_PARK_PRIMARY_SEMANTIC_OWNERS_READY ... primary_authoring=1 normalization_bridge=0 remaining_content_gap_instances=3` only after exact counts and zero legacy buckets are proven.
- Presentation downstream: `UOCParkGroundAuthoredUpgradeSubsystem` and `UOCParkHardscapeAuthoredUpgradeSubsystem` now require direct primary owners (`primary_source_required=1 normalization_bridge=0`) and emit READY markers with `primary_authoring=1 normalization_bridge=0`. Their bounds-aware fitting/transaction rollback behavior is unchanged. `ParkMemorialMonument=1` and `ParkSkateRamps=2` remain deliberately untouched.
- Verifier/workflow changes: added `VERIFY_PASS45_PARK_PRIMARY_SEMANTIC_OWNERS.py` and `.github/workflows/pass45-park-primary-semantic-owners.yml`; updated authored-ground/hardscape guards and cumulative `RUN_ALL_VERIFY.py`; aligned Gate K and bench guards to the new exact source ownership rather than keeping stale mixed-family assertions.
- CI correction history: the first `bfbcbdbb...` visual Gate K run correctly failed because its verifier still required the retired `ExpectedMemorialPlaza = 2` mixed-source form. Production ownership was not rolled back. `2b34cf4b...` updated Gate K truth to the direct 1/1/4/1/2/14 contract. The bench guard was proactively corrected in `148f3127...` from stale `LegacyCount` to the direct legacy-quarantine counters.
- Exact-head relevant CI for `148f3127c7d5100d06cda79c03d5456f17901e45`: `Source verification` completed **SUCCESS** including the full structural/regression suite; `Pass 45 Park Primary Semantic Owners`, `Pass 45 Park Semantic Authored`, `Pass 45 Park Memorial Approach Authored`, `Pass 45 Authored Park Ground`, `Pass 45 Authored Park Hardscape`, `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `World geometry stability pass 12`, `Tactical Map 2.0 source contracts`, `Runtime recovery Pass 45` and `Pass 45 strict runtime acceptance harness` completed **SUCCESS**. Unrelated workflows still queued/in-progress at ledger-write time are not used to promote runtime state.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. No current-head local UE 5.8 build/play/direct screenshot acceptance exists for the primary-owner migration or the authored ground/hardscape presentation.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. No protected merge is authorized.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked because source ownership cleanup cannot replace direct rendered Gate K acceptance.
- Remaining park Gate K content gaps: `ParkMemorialMonument` plus the two `ParkSkateRamps` instances still lack verified authored mesh candidates. Broader non-stadium world/material/LOD/reference fidelity and current-head UE 5.8 screenshots also remain open; arbitrary donor props remain forbidden.

## Work cycle — 2026-08-31 item 32 Museum legacy world-blockout retirement

- Canonical checklist relevance: item 32 / Gate K world/reference fidelity and the 2026-08-27 runtime rejection of overlapping/blockout landmark composition. This cycle removes a duplicate source owner at Museum; it does not claim direct UE 5.8 visual acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `149474a741df60151c4c90eed694f65f6e824a50`.
- Guard-first commit: `5f79b131712b30ea5b148e87d497357aad610af3` — `PASS45: add museum legacy blockout retirement guard`.
- Production commit: `42ae6313a2e94e4cfd7a69b347003597cc558dd1` — `PASS45: retire legacy museum world blockout`.
- Verification wiring commits: `1724d7c3d7aef33f63da18db5bff5357c530a659` — `PASS45: wire museum blockout retirement verification`; `7d68da90c1ec746e717b6720d582071663cad90a` — `PASS45: include museum blockout retirement in cumulative verification`.
- Stale-verifier alignment: `f22c481f4de30fc5fc7227ccbdc825ed0bb861c0` — `PASS45: align S09 with authored museum ownership`.
- Production ownership change: `AOCWorldSectorOster::BuildMuseumAndStadium()` no longer authors a second Museum shell through `LandmarkBlocks`, `LandmarkRoofs`, `LandmarkWindows` or `LandmarkDetails`. Those shared families remain live for College/other landmarks; only Museum-specific writes were retired.
- Site-scope safety: exactly three existing Museum perimeter `Fences` proxies remain source-owned because no verified authored site/fence replacement was identified. Stadium authoring in the same function is preserved.
- Authoritative presentation boundary: R13.7 remains the sole visible authored Museum exterior. Its `OnWorldBeginPlay` path still keeps `SuppressLegacyMuseum(World)` as a scoped compatibility guard for stale/older world variants, then builds the authored Museum. Missing authored shell content remains fail-closed through `PASS45_MUSEUM_AUTHORED_SHELL_FAIL ... basicshape_fallback=0`; the retired world Cube shell is not restored as fallback.
- Source marker: `PASS45_MUSEUM_LEGACY_BLOCKOUT_SOURCE_RETIRED ... legacy_landmark_blocks=0 legacy_landmark_roofs=0 legacy_landmark_windows=0 legacy_landmark_details=0 perimeter_fence_proxies=3 authoritative_presentation_owner=R137_MuseumPhotoModel runtime_compatibility_suppression=1 runtime_visual_acceptance=pending`.
- Guard/workflow changes: added `VERIFY_PASS45_MUSEUM_LEGACY_BLOCKOUT_RETIREMENT.py`; `Museum source contracts` now triggers on `OCWorldSectorOster.cpp` and the new verifier and executes the retirement guard; cumulative `RUN_ALL_VERIFY.py` now includes the same guard.
- CI correction history: the first cumulative run on `7d68da90...` failed only because legacy `VERIFY_S09.py` still required the old `red-brick single-storey wings` marker inside `OCWorldSectorOster.cpp`. Production code was not rolled back. `f22c481f...` moved Museum fidelity truth to the authoritative R13.7 owner and made S09 explicitly reject resurrection of the retired world-sector Museum shell.
- Exact-head relevant CI for `f22c481f4de30fc5fc7227ccbdc825ed0bb861c0`: `Source verification` completed **SUCCESS** including the full structural/regression suite; `Museum source contracts` completed **SUCCESS** including `Verify legacy Museum world blockout retirement`; `Pass 45 visual fidelity Gate K`, `Pass 45 Gate K global BasicShape scope`, `World geometry stability pass 12`, `Pass 45 landmark identity`, `Visible museum and weapon palette Pass 37`, `Landmark shell ownership Pass 21`, `Tactical Map 2.0 source contracts`, `Runtime recovery Pass 45`, `Pass 45 strict runtime acceptance harness` and `Pass 45 Park Primary Semantic Owners` completed **SUCCESS**.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. Source retirement removes one known overlap/duplicate-owner path but there is no current-head local UE 5.8 evidence proving `MUS-CAM-01..07`, Museum/Stadium/Culture-House composition, materials, scale or ground presentation are visually accepted.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED. No protected merge is authorized.
- Official checklist accounting remains **22/36 = 61.1%**. Item 32 remains unchecked.
- Remaining item-32 blockers: the three explicit Central Park authored-content-gap instances (`ParkMemorialMonument=1`, `ParkSkateRamps=2`), broader ground/material/LOD/reference fidelity including Culture House/plaza composition, and current-head UE 5.8 direct screenshot/runtime acceptance. Arbitrary substitute meshes remain forbidden.

## Work cycle — 2026-08-31 item 32 Culture House legacy owner fail-closed cleanup

- Canonical checklist relevance: item 32 / Gate K landmark ownership and Culture House/plaza fidelity. This slice removes a residual late runtime repair path; it does not invent missing Culture House reference detail or claim current-head UE 5.8 visual acceptance.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `fc42945273c3eeea3fbe443e5874582e82f742ce`.
- Production commit: `00f770345f991fb34a49c9c645f37206a639e855` — `PASS45: fail closed on legacy Culture House owner`.
- Verifier/source head before ledger write: `d4e92ecef55799df67dc9246843025510e5ef9d7` — `PASS45: guard Culture House legacy owner fail-closed contract`.
- Production change: `UOCR146CultureHousePhotoModelSubsystem::BuildCultureHouse()` no longer calls `Destroy()` when an unexpected `R13_CultureHousePhotoModel` actor is present. Startup ownership is already coordinated authoritatively; legacy presence is now treated as a source/startup regression and fails closed with `PASS45_CULTURE_HOUSE_AUTHORED_SHELL_FAIL reason=legacy_owner_present legacy_owner_mutation=0 primary_authoring_fix_required=1 runtime_acceptance=0`.
- Success truth marker now records `legacy_owner_mutation=0 runtime_visual_acceptance=pending`; the authoritative authored R146 shell, separate canonical Culture House geo anchor and six-column facade remain unchanged.
- Regression guard: `VERIFY_PASS45_LANDMARK_IDENTITY.py` now requires the fail-closed legacy-owner detection/markers and explicitly forbids the old `if (Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel"))) Existing->Destroy();` repair path. Existing `.github/workflows/pass45-landmark-identity.yml` covers the verifier and production file.
- Exact-head CI for `d4e92ecef55799df67dc9246843025510e5ef9d7`: `Pass 45 landmark identity` run `33376673153` / #376 **SUCCESS**; `Source verification` `33376673155` / #2531 **SUCCESS**; `Pass 45 visual fidelity Gate K` `33376673135` / #303 **SUCCESS**; `Pass 45 Gate K global BasicShape scope` `33376672982` / #99 **SUCCESS**; `Runtime recovery Pass 45` `33376673070` / #653 **SUCCESS**; `Pass 45 strict runtime acceptance harness` `33376673184` / #546 **SUCCESS**; `World geometry stability pass 12` `33376673362` / #563 **SUCCESS**; `Landmark shell ownership Pass 21` `33376673072` / #564 **SUCCESS**. All workflow runs returned for the exact source head completed SUCCESS.
- Reference/content boundary: the Culture House source-gap audit still leaves exact facade bearing, dimensions/material zones, detailed facade/site relationship, old-park/plaza composition and direct screenshot acceptance unresolved. These remain **CONTENT GAP / RUNTIME PENDING**, not targets for guessed geometry.
- Runtime state: **RUNTIME REJECTED 2026-08-27** remains authoritative. Green source CI does not supersede the rejected local UE 5.8 rendered state.
- `main` merge state: none; PR #94 remains OPEN / UNMERGED / mergeable. No protected merge is authorized.
- Official checklist accounting remains **22/36 = 61.1%**, **38.9% remaining**. Item 32 remains unchecked.
- Remaining item-32 blockers include exact Culture House/plaza/reference composition, Central Park `ParkMemorialMonument=1` plus `ParkSkateRamps=2` authored content gaps, broader ground/material/LOD fidelity and a current-head full local UE 5.8 Gate K/direct screenshot acceptance set.

## Work cycle — 2026-08-31 P0 Quick Normal black-screen / KiteDemo tree startup recovery

- Canonical checklist relevance: P0 UE 5.8 startup/runtime recovery plus item 27 vegetation truth. This cycle fixes the startup ownership/loading defect only; it does not accept the quarantined tree materials or item 27 visuals.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start GitHub head: `eee743320bb9474d59621e8a7580eaecab700bba`. PR remained OPEN / UNMERGED.
- Substantive source head: `57590e061c9a902da69768f554e1fd01d70f829c` — `PASS45: defer KiteDemo tree startup load`.
- Latest factual local runtime evidence: `START_HERE.cmd` -> `1. ЗВИЧАЙНА ГРА` completed the incremental C++ build with `Result: Succeeded`, then the direct game process stayed on a black window and had to be terminated. The log reached `PASS45_RENDER_BUDGET_READY` and `PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY`, emitted KiteDemo PCD3D_SM5 material compile failures including `Node TransformPosition input must be a 3-component vector` and `SpeedTree node not currently supported for Skeletal Meshes`, then entered `Building static mesh HillTree_02` / `Waiting for static meshes to be ready`.
- Root cause confirmed in source: `AOCWorldSectorOster` synchronously loaded `HillTree_02`, `ScotsPine_01` and `ScotsPineTall_01` with native-constructor `ConstructorHelpers::FObjectFinder`. `BuildVegetation()` immediately depended on those meshes/bounds. This made the world-sector CDO pull the rejected KiteDemo material/static-mesh compile chain before a usable first frame. The preceding Stein-prepass isolation commit had independently recorded the same runtime-module/CDO tree-load coupling under NullRHI.
- Production correction: the three KiteDemo paths are removed from the native constructor/CDO. Constructor-time `BuildVegetation()` now creates only lightweight ground-cover zoning and explicitly returns before any tree-bounds access. Normal runtime quarantines the exact KiteDemo tree family by default so first-frame startup does not touch the rejected compile chain.
- Controlled diagnostic route: `-Pass45LoadKiteDemoTrees` is an explicit opt-in only. It resolves the same exact three paths through `FStreamableManager::RequestAsyncLoad` after `BeginPlay`, keeps the same `AOCWorldSectorOster` owner, creates no second mutation subsystem and still records `material_compatibility=pending runtime_acceptance=0`.
- Regression guard/workflow: `VERIFY_PASS45_TREE_STARTUP_DEFERRED.py` + `.github/workflows/pass45-tree-startup-deferred.yml`; cumulative `RUN_ALL_VERIFY.py` includes the guard. The guard rejects any return of the three KiteDemo paths or named tree `FObjectFinder`s inside the constructor/CDO and requires the quarantine/async evidence contract.
- Canonical TZ updated to `RUNTIME REJECTED 2026-08-31` with a dedicated latest-startup-blocker section.
- Runtime state after source correction: **RUNTIME PENDING / latest factual verdict remains RUNTIME REJECTED 2026-08-31**. No new local UE 5.8 Quick Normal launch has yet proven that the black-screen startup is gone.
- Runtime acceptance: **not credited**. PR #94 remains OPEN / UNMERGED and must not merge before a new current-head UE 5.8 runtime acceptance.
- Official checklist accounting remains **22/36 = 61.1%**, **38.9% remaining**. Item 27 remains unchecked because the exact tree family is intentionally quarantined pending UE 5.8 material/static-mesh repair and direct visual acceptance.

## Work cycle — 2026-08-31 P0 pre-tick sector / second START tree-load recovery

- Canonical checklist relevance: P0 startup recovery, Block0 pre-tick ground foundation and item 27 vegetation truth. This cycle removes two source-level startup blockers; it does not claim a successful UE 5.8 runtime.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Start head: `2587504484c5a4d83454dd7b4050f674cec92747`.
- Source commits: `c61d78178ac1dfc9aeb65a10803fc728567586a1` — pre-tick sector recovery; `b3d00e59110bd1f45cbff8443f6ef34efb1698b0` — preserve stadium verifier/source contract under quarantine; `6cc2158d31f10ee50a1c5d3ca096b86b4b49022b` — distinguish legitimate pre-tick BeginPlay lifecycle from the rejected Fast Preview bootstrap path.
- Root cause 1: Block0 validates the world in `UWorldSubsystem::OnWorldBeginPlay`, while the legacy sector was only created later from GameMode `BeginPlay`; therefore the validator factually saw `oster_sector_count_0` before gameplay tick.
- Production correction 1: `AOCGameModeRuntimeSafe::InitGame()` now creates the lightweight canonical `AOCWorldSectorOster` before world-subsystem BeginPlay, tags it as the pre-tick owner, and requires exactly one. After the legacy base `BeginPlay` path runs, the temporary duplicate is retired before first gameplay tick so the final live sector count is exactly one.
- Root cause 2: `UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay()` independently synchronously loaded `HillTree_02` and `ScotsPineTall_01`, recreating the same heavy KiteDemo START dependency after the world-sector constructor path had already been deferred.
- Production correction 2: `UOCR13StadiumSurfaceSubsystem` is quarantined from automatic world-subsystem instantiation with `UCLASS(Abstract)`. Its source implementation remains for a future safe delayed/content-repaired path, but automatic START no longer pulls those rejected tree/material dependencies.
- Explicit tradeoff: automatic Stadion Oster presentation is temporarily a **CONTENT GAP / RUNTIME PENDING** path while quarantined. This is intentional P0 startup recovery, not visual acceptance.
- Regression protection: added `VERIFY_PASS45_PRETICK_SECTOR_STARTUP_GUARD.py`, `.github/workflows/pass45-pretick-sector-startup-guard.yml`, and cumulative `RUN_ALL_VERIFY.py` wiring. The guard requires the pre-tick sector contract, the Block0 fail-closed check, stadium quarantine, and absence of the diagnostic `Pass45LoadKiteDemoTrees` opt-in from the normal gameplay launcher.
- Stale-verifier correction: the first cumulative source run correctly exposed that `VERIFY_PASS45_FAST_PREVIEW_PROGRESS.py` still rejected any GameMode `BeginPlay` override because an older failed experiment had used BeginPlay for a viewport bootstrap overlay. The verifier was narrowed to forbid the actual rejected overlay/bootstrap symbols while allowing the new sector-lifecycle override.
- Exact source CI for `6cc2158d31f10ee50a1c5d3ca096b86b4b49022b`: `Source verification` run `33421757302` completed **SUCCESS** including the full structural/regression suite. `Pass 45 pre-tick sector startup guard`, `Pass 45 Block0 ground and grass foundation`, `Pass 45 tree startup deferred`, `Runtime recovery Pass 45`, `Pass 45 strict runtime acceptance harness`, Stadion Oster, Gate K and the other returned exact-head workflows completed **SUCCESS**.
- Runtime state: **RUNTIME REJECTED 2026-08-31** remains the latest factual local verdict. No new current-head local UE 5.8 launch has been run after these commits, so source green cannot promote runtime state.
- `main` remains `bca00f4046700f383af9f1742cc24b6a62401b1a`; PR #94 remains OPEN / UNMERGED. No merge is authorized.
- Official checklist accounting remains **22/36 = 61.1%**, **38.9% remaining**. No runtime-dependent item is newly credited.
- Local uncommitted user work was not touched by these remote GitHub commits.
- Next factual blocker: run the current PASS45 branch in UE 5.8 and verify Quick Normal reaches a usable first frame without `PASS45_BLOCK0_PRETICK_GROUND_FAIL`, `oster_sector_count_0`, or the quarantined KiteDemo tree START chain. Only that runtime evidence can advance acceptance.