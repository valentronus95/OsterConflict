# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

The immediately preceding complete history was preserved byte-for-byte before this rotation at:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_REMINGTON_MOTION_2026-09-01.md`

Preserved Git blob: `99969c3221d26a2d4a9bb372243e9fe68681956a`.

The earlier pre-2026-09-01 detailed archive remains preserved separately at:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_2026-09-01.md`

Git history remains the raw source of truth. This file is intentionally compact so future PASS45 sessions can continue from the latest factual checkpoint without replaying hundreds of commits.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active integration PR: **#94 OPEN / UNMERGED**.
- Target branch: `main`.
- `main`: `bca00f4046700f383af9f1742cc24b6a62401b1a` at this checkpoint.
- Latest factual local verdict: **RUNTIME REJECTED 2026-08-31**.
- Latest substantive item-16 audit head before this history rotation: `20581d92206b068e75a7f654dca0f7b06c9013b6`.
- Current branch HEAD must always be re-read from GitHub before another write because guarded content workflows may advance it.
- Merge rule: PR #94 remains OPEN / UNMERGED until a current-head local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot/audio acceptance.
- Official canonical checklist accounting remains **22/36 = 61.1% complete, 38.9% remaining**. Source-only evidence on runtime-dependent work does not increase the percentage.

## First factual open item

The first canonical unchecked checklist item remains **item 16**:

accepted authored M700 / Remington 870 / Lever Action moving-part or skeletal manual-action presentation, factual bolt/pump/lever mechanical audio, and local UE 5.8 acceptance.

Do not skip to later checklist items merely because item 16 contains external-content/runtime blockers.

## Current item-16 factual state

- M700 exact authored manual-action animation remains a content gap.
- Lever Action exact authored manual-action animation remains a content gap.
- Remington 870 exact production `.uasset` remains absent and unaccepted.
- Remington donor source is the registered CC-BY-4.0 `8sianDude` GLB at exact SHA-256 / Git LFS OID `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`.
- That donor remains `APPROVED_FOR_UE_IMPORT` only: `runtime_ready=false`, `ue58_import_pending=true`, `item16_checked=false`.
- Pump audio source fallback remains project-owned `/Game/R13/Audio/shotguncock`.
- Repository-owned CC0 bolt and lever mechanical donor WAV derivatives remain source payloads with UE import/runtime wiring still pending.
- The rejected procedural whole-weapon/arms manual-action fallback remains physically retired and must not be restored.

## Work cycle — 2026-09-01 item 16 Remington donor articulated-motion truth

Start checkpoint for this continuation: `e635392ef616c8ca6dacc8e27ae690d2294e6017`.

Live pre-work audit reconfirmed:

- canonical branch `fix/pass45-runtime-rejection-material-closure-20260826`;
- `main@bca00f4046700f383af9f1742cc24b6a62401b1a`;
- PR #94 OPEN / UNMERGED / mergeable;
- latest runtime verdict `RUNTIME REJECTED 2026-08-31`;
- reuse-first replacement spec, deep audit and third-party register remain binding;
- item 16 is still the first factual unchecked point;
- no production merge or local UE acceptance was permitted or claimed.

### Donor target fingerprint slice

`9effd0900a68f90d9a550ae00e3a2eaef8edbd53` — extend the guarded Remington source acquisition probe to record deterministic action-target evidence for donor fire index 2, easy reload index 3 and full reload index 4.

The first acquisition run `33557757809` correctly failed in the existing source-intake verification step. Root cause was not donor/runtime failure: the verifier expected an LFS pointer in the working tree while the acquisition job correctly had the full 20,621,580-byte GLB checked out before commit, even though the Git index already contained the correct LFS pointer.

`cb59b504072b4e05ab95b43073427cafdce86e33` — fix that CI-only mismatch by verifying the exact staged LFS pointer before running the existing source-intake guard. No gameplay, production package or runtime owner changed.

Acquisition run `33558136503` then completed **SUCCESS** and guarded bot commit `d79ab89b5f6d160e65a489a0f5a8783cf40a9336` persisted the new source evidence.

Pinned action-target fingerprints:

- fire index 2: `5446382d16de5fa56ad784bce41e80be3ae70dc76c03a8a6456e660957f9e5e1`;
- easy reload index 3: `4e96b343cc6268a6239d104ae635abf6dbab5609b01dbca37c956e406e26083f`;
- full reload index 4: `a93ebf3318791345771e8f775d718d2f89427f568378249d1077090687f29487`.

Each action clip targets 56 unique nodes. Named weapon-side targets common to the set include `Rif_059`, `Trigger_060`, `PBody_058` and `Pmag_061`.

### Binary weapon-motion measurement slice

`1fe7ac5001192de37611dc3f191bf26542763d9c` — extend the fail-closed source probe to decode the exact pinned GLB animation accessors and measure weapon-node motion rather than inferring it from filenames or channel counts.

Acquisition run `33558367993` completed **SUCCESS** and guarded bot commit `57606702a72aa128d840991d99818514c7520d58` persisted the measured motion.

Factual pinned motion:

- fire / `PBody_058`: translation `0.050905`, rotation `24.037146°`;
- fire / `Pmag_061`: translation `0.050845`, rotation `24.037145°`;
- easy reload / `PBody_058`: translation `0.067139`, rotation `25.570902°`;
- easy reload / `Pmag_061`: translation `0.606719`, rotation `76.062898°`;
- full reload / `PBody_058`: translation `0.070092`, rotation `25.570902°`;
- full reload / `Pmag_061`: translation `0.606719`, rotation `76.062898°`.

This proves material, reload-specific motion on a named weapon-side donor node. It does **not** prove that `Pmag_061` is the physical fore-end/pump, and it does not prove that any donor clip is a standalone post-shot pump cycle.

`20581d92206b068e75a7f654dca0f7b06c9013b6` — add `_DOCS/PASS45_REMINGTON870_DONOR_MOTION_AUDIT_2026-09-01.md`, a dedicated verifier and workflow. The enforced verdict is:

`ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN`.

The gate explicitly prevents a reload clip from being silently re-labelled as the authoritative pump cycle, prevents source motion evidence from setting runtime acceptance, and preserves the reuse-first rule that a second Remington donor must not be stacked unless the primary donor is factually blocked/rejected in isolated UE 5.8 proof.

### Acceptance state after this cycle

- Source donor identity/provenance: accepted for isolated UE import only.
- Articulated weapon-side reload motion: **SOURCE EVIDENCE PASS**.
- Exact pump-node identity: **UNPROVEN**.
- Standalone pump/manual-action sequence: **UNPROVEN**.
- UE 5.8 isolated import execution: still required locally.
- Production Remington cutover: **NOT AUTHORIZED**.
- Item 16: **UNCHECKED**.
- Runtime verdict: **RUNTIME REJECTED 2026-08-31**.
- PR #94: **OPEN / UNMERGED**.
- Local user `Changes`: not touched.
- Official progress: **22/36 = 61.1% complete, 38.9% remaining**.

## Next factual operation

Run the already-prepared isolated local UE 5.8 Remington import pilot on current canonical content and inspect the imported skeletal hierarchy/animation set in Unreal. Promotion requires proof that the required moving-part action survives UE import and is visually the actual pump/manual-action behavior. Until that evidence exists, do not populate production `ManualActionAnimationObjectPath`, do not create `/Game/Production/Weapons/Remington870` as accepted content, do not stack a second Remington donor, and do not merge PR #94.
