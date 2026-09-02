# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The immediately preceding complete history is preserved at:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_CURRENT_HEAD_PREFLIGHT_2026-09-02.md`

Preserved Git blob: `798b6b3b35e09cc4a4fce600dcaaf6afce7f9bf6`.

Earlier archives remain preserved in Git. Git history is the raw source of truth; this file intentionally stays compact so a new session can resume without replaying hundreds of commits.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Target: `main@bca00f4046700f383af9f1742cc24b6a62401b1a` at this checkpoint.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Latest source-contract head before this checkpoint update: `b3467b5e82377670cc06b186c2687114df3e89a8`.
- PR #94 must remain unmerged until a current-head local UE 5.8 runtime acceptance passes the still-open item-16 content/runtime gates.
- Official checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source/docs/CI preparation does not increase it.
- Local user `Changes` remain outside assistant mutation scope.

## First factual open item

The first canonical unchecked checklist item remains **item 16**: accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items while item 16 still contains content/runtime blockers.

## Binding reuse-first state

`_DOCS/PASS45_REUSE_FIRST_REPLACEMENT_SPEC.md`, `_DOCS/PASS45_REUSE_FIRST_DEEP_AUDIT_2026-09-01.md` and `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` remain binding.

- Primary Remington donor remains the registered CC-BY-4.0 8sianDude payload; do not silently promote the quarantined `Remington_870_FREE.glb` candidate.
- Authored manual actions belong in skeletal animation timing; MetaSounds/audio cannot become a second gameplay timer.
- The retired procedural whole-weapon/arms manual-action fallback must not return.
- Compile/source green is not runtime acceptance.

## Latest user runtime evidence — 2026-09-02

The user completed the full runtime launcher and reached gameplay successfully. The latest screenshots/runtime observation supersede the older startup-black-window state as the immediate runtime truth.

Observed in gameplay:

- gameplay world loaded;
- Lever Action and M700 were present/usable in the tested session;
- normal HUD/downed presentation was reached;
- Remington 870 fired and showed recoil;
- **Remington 870 did not show a visible pump/fore-end cycle after firing**.

Therefore the current item-16 runtime verdict is not a full PASS. The immediate Remington defect is now narrowly classified as:

`GAMEPLAY_REACHED / REMINGTON_RECOIL_PRESENT / REMINGTON_PUMP_PRESENTATION_MISSING`.

The runtime source explains that observation: `OC_SG1` has pump action semantics/timing/audio but still has an empty authored manual-action animation path, while the current Remington runtime visual falls back to the generic static shotgun. The authored animation bridge only plays compatible `UAnimSequence` content on a `USkeletalMeshComponent`.

## Registered Remington donor

Primary source:

- `SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb`;
- exact SHA-256 / LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`;
- exact payload size `20,621,580` bytes;
- creator: 8sianDude;
- license: CC-BY-4.0;
- register entry: `PASS45-3P-WEAPON-001`, still not runtime-ready.

Existing exact-donor UE 5.8 evidence already proves imported articulated motion and addressable moving weapon-side tracks. That proof remains valid but does not itself authorize gameplay cutover.

Pump audio fallback remains project-owned `/Game/R13/Audio/shotguncock`.

## Remington physical fore-end isolation

The old direct assumption `Pmag_061 = pump` is rejected. Source topology proved `Pmag_061 / Object_95` is composite geometry: **4411 vertices, 4982 triangles, 106 disconnected components**.

A deterministic source partition now isolates the probable physical fore-end from the side-saddle/other geometry:

- fore-end partition: **48 components / 1170 vertices**;
- remaining side-saddle/other partition: **58 components / 3241 vertices**.

`PASS45_REMINGTON870_DERIVED_PUMP_SOURCE.py` creates a CC-BY derivative of the exact registered primary donor, not a second donor:

- new bind-equivalent joint: `PASS45_PumpForeEnd`;
- only the 1170 fore-end vertices are reassigned to it;
- the remaining 3241 vertices stay on the original `Pmag_061` ownership;
- new standalone animation: `PASS45_Remington870_PumpCycle`;
- duration: **0.55 s**;
- derived stroke uses the donor-measured dominant-Y travel and omits unrelated X/Z/rotation;
- `production_cutover=false`, `runtime_acceptance=false`, `item16_checked=false`.

The derived source verifier is PASS. This replaces the older `PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN` checkpoint with a narrower state: a deterministic derived fore-end and standalone pump source now exist, but UE 5.8 production assembly/runtime acceptance remain pending.

## Derived pump UE 5.8 pilot

`PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py` is the isolated engine proof path. It rebuilds the exact derivative, imports with `save=False`, and requires UE 5.8 to prove:

- `PASS45_PumpForeEnd` exists on an imported SkeletalMesh;
- the standalone pump AnimSequence exists;
- the pump bone has non-trivial sampled motion;
- the animation and pump mesh share a compatible skeleton;
- the accepted sequence is uniquely near 0.55 s and retains the expected pump identity.

It deliberately writes only Saved evidence and does not save `/Game` packages or authorize production/runtime acceptance.

Its static/source contract is CI-green, but **the new derived-pump pilot has not yet been executed locally in UE 5.8 on this current source state**. Do not convert source-contract CI into engine acceptance.

## UE 5.8 imported assembly audit

A further fail-closed audit was added because the donor contains multiple mesh/skin parts and blindly selecting the first SkeletalMesh could create an animated fore-end without a complete shotgun visual.

Current files:

- `PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py`;
- `VERIFY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.py`;
- `OsterConflict/TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`;
- `.github/workflows/pass45-remington870-derived-pump-ue58-assembly-audit.yml`.

The audit reuses and executes the existing derived UE 5.8 pilot in the same isolated commandlet process, then inventories all unsaved imported assets and records:

- SkeletalMesh count;
- StaticMesh count;
- animation count;
- pump-bearing mesh count;
- skeleton identities/shared skeletons;
- the unique compatible pump sequence;
- one of `SINGLE_SKELETAL_IMPORT_CANDIDATE`, `MULTI_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN`, or `MIXED_STATIC_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN`.

It always remains fail-closed with `production_visual_completeness=UNPROVEN`, `saved_packages=0`, `production_cutover=0`, `runtime_acceptance=0`, `item16_checked=0`.

Exact-head source-contract CI on `b3467b5e82377670cc06b186c2687114df3e89a8`:

- `Pass 45 Remington 870 derived pump UE58 assembly audit` run `33652982669`: **SUCCESS**.

That is static/source validation only. The actual UE 5.8 assembly classification remains pending until the local launcher is run.

## Current acceptance state

- Registered donor provenance/source intake: **PASS FOR ISOLATED UE IMPORT**.
- Exact registered donor UE 5.8 imported-motion evidence: **LOCAL PASS / IMPORTED-MOTION PROOF ONLY**.
- Direct `Pmag_061 = pump` mapping: **REJECTED**.
- Derived physical fore-end partition 1170/3241: **SOURCE PASS**.
- Derived `PASS45_PumpForeEnd`: **SOURCE PASS / UE 5.8 CURRENT-DERIVATIVE EXECUTION PENDING**.
- Derived standalone `PASS45_Remington870_PumpCycle` 0.55 s: **SOURCE PASS / UE 5.8 CURRENT-DERIVATIVE EXECUTION PENDING**.
- Imported production visual assembly shape: **UNPROVEN / LOCAL UE 5.8 AUDIT PENDING**.
- Production Remington skeletal package: **ABSENT / NOT AUTHORIZED**.
- Gameplay `ManualActionAnimationObjectPath` cutover: **NOT AUTHORIZED**.
- Latest gameplay: **REACHED; REMINGTON RECOIL PRESENT; REMINGTON PUMP MISSING**.
- Separate `Remington_870_FREE.glb`: **QUARANTINED / NOT PROMOTED**.
- M700 exact authored bolt animation: **CONTENT GAP**.
- Lever Action exact authored lever animation: **CONTENT GAP**.
- Item 16: **UNCHECKED**.
- PR #94: **OPEN / UNMERGED**.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Run the single isolated local launcher:

`OsterConflict\TRY_PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.cmd`

It performs the current derived-pump UE 5.8 engine proof and the imported-assembly inventory in one process without saving Content packages. The resulting log/JSON must determine whether Unreal exposes one complete skeletal visual or a multi-part/mixed assembly.

Only after that factual engine result may the production Remington package and runtime `ManualActionAnimationObjectPath` be wired. No production package save, item-16 closure, or PR #94 merge is authorized before that evidence.
