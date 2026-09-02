# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

The previous complete ledger is preserved by Git blob:

`f480ba6dc01de2b41e9d3970f7a91b8b4cee1966`

Use Git history for older implementation detail. This live ledger is intentionally compact and current so future work does not restart already accepted or rejected slices.

## 1. Current context — 2026-09-02

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Pass 45 is the active corrective pass.
- Source-contract head for the current Remington assembly-audit slice: `b3467b5e82377670cc06b186c2687114df3e89a8`.
- Persistent history checkpoint immediately after that source slice: `b0c0ea6520d0076be131f4a36b658ebbd1cb41a4`.
- Formal checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**.
- First factual open checklist item remains **item 16**.
- UE target: 5.8.x / Windows.
- Canonical user launcher remains `START_HERE.cmd` for game/full runtime; focused PASS45 evidence launchers may be used only for their named isolated proof.
- PR #94 must not merge until current-head UE 5.8 runtime acceptance for the required open gates.
- Local user `Changes` are not to be reset, cleaned, overwritten or casually committed.

Current status token:

**PASS45 ACTIVE / GAMEPLAY REACHED 2026-09-02 / REMINGTON RECOIL PRESENT / REMINGTON PUMP PRESENTATION MISSING / DERIVED PUMP SOURCE READY / CURRENT DERIVED UE58 ENGINE+ASSEMBLY AUDIT PENDING / ITEM16 OPEN / PR94 UNMERGED**

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure incomplete.
- `CODED_UNTESTED` — source correction exists but factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content absent/unverified; never fake READY.
- `AUDIO CONTENT GAP` — routing exists but accepted authored sound absent/unverified.
- `RUNTIME REJECTED` — factual local gameplay disproved the result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds.
- `VERIFIED RUNTIME` — factual local UE/user playtest proves behavior/appearance.
- Green source CI is structural evidence only, not UE compile/runtime acceptance.
- Historical verifiers may not override newer runtime evidence or resurrect retired owners.
- No profile/material/audio slot becomes READY merely because a field or object path exists.

## 3. Latest authoritative runtime — 2026-09-02

The user's latest full runtime attempt reached actual gameplay. This supersedes the older 2026-08-31 black-window startup rejection as the immediate startup truth.

Observed factually:

- full runtime route completed far enough to enter the playable world;
- M700 and Lever Action were visible/usable in the tested session;
- normal HUD and downed-state presentation were reached;
- Remington 870 fired and showed recoil;
- **Remington 870 did not visibly cycle its pump/fore-end after firing**.

Therefore this is not a full runtime acceptance. The active narrow defect is:

`REMINGTON_MANUAL_ACTION_PRESENTATION_MISSING`.

Source inspection matches the runtime symptom:

- `OC_SG1` already has Pump mechanical action semantics, cycle timing and manual-action audio routing;
- its `ManualActionAnimationObjectPath` remains empty;
- current Remington gameplay therefore falls back to the generic static shotgun visual;
- `UOCFirstPersonWeaponPresentationSubsystem` only starts authored manual-action animation on a compatible `USkeletalMeshComponent + UAnimSequence` sharing the same skeleton;
- simply filling an animation path while keeping a static production visual would not work.

The previous black-window history remains retained evidence, but it is no longer the immediate startup blocker on the latest user run.

## 4. Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass44 as a complete solution: spawn/result framing was wrong, the map was still perceived as excessively large/empty, weapon visuals/materials were not production-ready, production-model claims were unreliable, and FPS could collapse severely. Pass45 supersedes Pass44 as the active corrective pass; this historical rejection may not be erased by later source fixes.

### Pass 44 behavior retained unless disproved

The following Pass44 decisions remain protected as non-regression because later evidence did not invalidate them:

- compact central-Oster hard extent: approximately 960×940 m, never restore the historical 2.4 km battlefield;
- normal local gameplay defaults to zero implicit filler bots unless explicitly requested;
- Museum BASE acceptance must be based on the actual live pawn, not source-only spawnpoint existence;
- tactical-map bounds follow the compact central-Oster reference rather than legacy peripheral component auto-fit;
- grey/BasicShape weapon material repair is forbidden; authored material gaps remain fail-visible;
- the retired Pass37 weapon palette compatibility owner stays physically deleted, not preserved as an inert shell.

Pass44 historical non-regression does **not** authorize resurrection of any owner/repair path that Pass45 physically retired. These retained historical rules do not override the newer 2026-09-02 gameplay evidence above.

## 5. Binding non-regression and reuse-first rules

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

Protected rules include:

- one runtime responsibility has one mutating owner;
- validation may fail but must not become a late repair owner;
- retired procedural whole-weapon/arms manual-action fallback stays physically retired;
- manual-action gameplay timing remains server/mechanical-action-owned, not audio-owned;
- exact authored animation may use the existing bridge, not a second gameplay timer;
- primary registered donor must be exhausted before promoting a second Remington donor;
- unknown/unverified license cannot be promoted;
- user runtime/screenshots outrank source-only READY claims;
- direct gameplay acceptance requires factual UE evidence.

## 6. Item 16 current state

Item 16 requires authored moving-part/manual-action presentation and factual mechanical audio for M700, Remington 870 and Lever Action plus local UE 5.8 acceptance.

Current state:

- M700 exact authored bolt animation: **CONTENT GAP**.
- Lever Action exact authored lever animation: **CONTENT GAP**.
- Remington exact gameplay pump presentation: **RUNTIME FAIL on latest user test**.
- Remington pump audio fallback: project-owned `/Game/R13/Audio/shotguncock` remains valid routing content.
- Repository-owned bolt/lever mechanical source audio remains separate from visual acceptance and does not close missing animation.
- Item 16 stays **UNCHECKED**.

## 7. Registered Remington donor

Primary donor remains:

- `SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb`;
- SHA-256 / LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- 20,621,580 bytes;
- creator 8sianDude;
- CC-BY-4.0;
- register `PASS45-3P-WEAPON-001`;
- still `runtime_ready=false`, `item16_checked=false`.

Exact registered-donor UE 5.8 imported-motion evidence previously passed. It proves articulated imported motion survives UE 5.8, not that a production pump presentation is accepted.

The separate local `Remington_870_FREE.glb` remains quarantined as an unregistered candidate and is not the production route while the primary donor remains viable.

## 8. Remington Pmag topology and derived fore-end

Direct `Pmag_061 = pump` mapping is rejected because source topology proved `Pmag_061 / Object_95` is composite:

- 4411 vertices;
- 4982 triangles;
- 106 disconnected components.

Deterministic spatial/topology partition of the registered primary donor now isolates:

- probable fore-end: **48 components / 1170 vertices**;
- side-saddle/other remainder: **58 components / 3241 vertices**.

`PASS45_REMINGTON870_DERIVED_PUMP_SOURCE.py` creates a deterministic CC-BY derivative of the exact primary donor:

- new joint `PASS45_PumpForeEnd`;
- only the 1170 fore-end vertices move to the new joint;
- 3241 remainder vertices retain original ownership;
- new standalone `PASS45_Remington870_PumpCycle`;
- duration 0.55 s;
- stroke derived from donor measured dominant-Y pump travel;
- unrelated X/Z/rotation omitted;
- `production_cutover=false`, `runtime_acceptance=false`, `item16_checked=false`.

The derived source verifier is PASS.

## 9. Derived pump UE 5.8 engine gate

`PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py` is the current isolated engine gate. It rebuilds the exact derivative, imports it without saving packages and requires UE 5.8 to prove:

- imported SkeletalMesh has `PASS45_PumpForeEnd`;
- pump animation exists;
- sampled pump motion is non-trivial;
- mesh/animation skeleton is compatible;
- one uniquely matching sequence near 0.55 s retains `PASS45_Remington870_PumpCycle` identity.

Static/source contract CI is green. **Actual local UE 5.8 execution of the current derived pilot is still pending.**

Do not claim engine acceptance from its Linux/static CI.

## 10. Imported assembly audit

The donor contains multiple mesh/skin parts, so selecting the first pump-bearing SkeletalMesh without inventory could yield an animated part instead of a complete shotgun visual.

Added fail-closed audit:

- `PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py`;
- `VERIFY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py`;
- `OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`;
- `.github/workflows/pass45-remington870-derived-pump-ue58-assembly-audit.yml`.

The audit runs the existing derived UE 5.8 pilot in the same isolated commandlet process, inventories all unsaved imported assets, and records:

- SkeletalMesh/StaticMesh/AnimSequence counts;
- pump-bearing mesh count;
- skeleton identities;
- unique compatible pump sequence;
- assembly classification:
  - `SINGLE_SKELETAL_IMPORT_CANDIDATE`, or
  - `MULTI_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN`, or
  - `MIXED_STATIC_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN`.

It does **not** save Content packages and always leaves:

`production_visual_completeness=UNPROVEN / production_cutover=0 / runtime_acceptance=0 / item16_checked=0`.

Exact source-contract CI for the implementation head:

- head `b3467b5e82377670cc06b186c2687114df3e89a8`;
- workflow `Pass 45 Remington 870 derived pump UE58 assembly audit`;
- run `33652982669`;
- result **SUCCESS**.

The same audit contract also passed on the later documentation head before this historical-marker correction. The actual assembly classification still needs the local UE 5.8 run.

## 11. Why production cutover is still blocked

Current gameplay production wiring is intentionally unchanged:

- Remington does not yet have an accepted `/Game/Production/Weapons/Remington870` skeletal package;
- `OC_SG1` authored manual-action path stays empty;
- current static/generic fallback remains visible rather than pretending the missing pump is solved;
- runtime production validator still reflects the pre-cutover Remington state.

This is deliberate fail-closed behavior. Production wiring is only legal after the current derived engine proof identifies the imported assembly shape and proves the pump sequence on the compatible skeleton.

## 12. Next factual operation

Run exactly one isolated local engine audit:

`OsterConflict\TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`

It performs both the current derived-pump UE 5.8 motion proof and assembly inventory in one UE commandlet process. It writes only `Saved` log/JSON evidence and does not save `.uasset` production content.

The result determines the next implementation branch:

- one complete skeletal import candidate -> build guarded canonical production package and wire existing authored animation bridge;
- multiple skeletal/mixed assets -> implement explicit production assembly or source combination first, then wire the bridge.

After production wiring, a fresh local gameplay test must visibly prove post-shot pump travel and mechanical audio timing. Only then may the Remington slice be accepted. M700/Lever gaps still remain within item 16.

## 13. Protected merge/accounting state

- PR #94: **OPEN / UNMERGED**.
- Do not merge without current-head UE 5.8 runtime acceptance.
- Item 16: **UNCHECKED**.
- Official checklist: **22/36 = 61.1% complete**.
- Remaining: **38.9%**.
- Local user `Changes`: **DO NOT TOUCH**.
