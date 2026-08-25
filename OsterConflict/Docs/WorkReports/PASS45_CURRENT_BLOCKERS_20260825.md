# Pass45 current blockers — 2026-08-25

Status: **SOURCE CORRECTIONS ADVANCED / FINAL RUNTIME ACCEPTANCE BLOCKED**

Current `main` after PR #89 contains the strict Pass45 runtime acceptance harness. Source CI success does not promote Pass45 to runtime-ready.

## Source milestones already merged

- PR #86: rejected generic AdvancedVillagePack runtime decorator/recovered owners physically retired; stale CI forward-ported.
- PR #87: repository-safe BTR-4 fallback GLB now carries an explicit authored PBR material and primitive material binding.
- PR #88: production weapon runtime validation now reports exact mesh -> material slot -> authored material -> runtime material -> used texture dependencies and rejects default/placeholder materials.
- PR #89: strict acceptance now requires authored material/dependency evidence plus actual driver enter/exit and M2 gunner interaction evidence; automated logs cannot mark visual acceptance complete.

## Current hard content blockers

Repository-wide current-tree audit confirms that these canonical production weapon assets are absent:

1. Remington 870
   - UE path: `/Game/Production/Weapons/Remington870/SM_Remington870`
   - required file: `OsterConflict/Content/Production/Weapons/Remington870/SM_Remington870.uasset`
   - current status: **CONTENT GAP / NOT READY**

2. M249
   - UE path: `/Game/Production/Weapons/M249/SM_M249`
   - required file: `OsterConflict/Content/Production/Weapons/M249/SM_M249.uasset`
   - current status: **CONTENT GAP / NOT READY**

`AOCWeapon_Shotgun` and `AOCWeapon_LMG` already request those canonical paths. When load fails they retain the gameplay fallback. That fallback is not production acceptance.

No generated grey/default material or invented production replacement is accepted to close either gap.

## Final-acceptance behavior

`VERIFY_PASS45_REQUIRED_LOCAL_CONTENT.py` is a fail-fast preflight for `RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd`.

Before the long final playtest it checks the two required local production `.uasset` files. If either is missing, strict acceptance exits with code `21` and writes:

`Logs/PASS45_REQUIRED_CONTENT_PREFLIGHT.txt`

This does not block ordinary gameplay/testing through `RUN_R14_CURRENT_GAMEPLAY.cmd`; it blocks only the final Pass45 acceptance claim.

If both assets later exist locally, strict acceptance continues to the existing gates:

- normal current-source gameplay route;
- headless production material validation;
- 11/11 weapon runtime validation;
- zero material gaps and unexpected overrides;
- driver enter/exit transform proof;
- M2 gunner pitch/exit proof;
- visual acceptance remains manual/pending until direct screenshots are reviewed.

## Current Pass45 execution truth

- generic rejected houses/fences/tower-shack owner path: source-retired; runtime screenshot still required;
- BTR white/default fallback source cause: corrected; local UE import/runtime screenshot still required;
- weapon dependency audit: implemented;
- Remington 870: **CONTENT GAP**;
- M249: **CONTENT GAP**;
- current-head source CI for merged corrective PRs: green before each merge;
- factual local UE 5.8 final acceptance: **NOT ACHIEVED**;
- Pass45 overall status remains **RUNTIME REJECTED / CORRECTIVE WORK IN PROGRESS** until the missing content and runtime/visual gates are satisfied.
