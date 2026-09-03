# Oster Conflict — persistent work context

This repository is the canonical working project for the user.

## Repository
- GitHub: `valentronus95/OsterConflict`
- Primary integration branch: `main`
- Dedicated location branches may be used for isolated work, but completed, explicitly approved integrations land in `main` through a controlled forward-port.
- Unreal Engine target: UE 5.8.x on Windows
- Project: `OsterConflict/OsterConflict.uproject`

## Mandatory context files
Before editing the project, read these files in this order:
1. `AGENTS.md`
2. `OSTER_CONFLICT_WORK_LEDGER.md` — active issues, repeat counts, real completion state, commits and test status.
3. The dedicated location TZ for the work being changed; for Stadion Oster use `STADION_OSTER_TZ.md`.
4. The matching implementation-status file when present; for Stadion Oster use `STADION_OSTER_IMPLEMENTATION_STATUS.md`.

The WORK LEDGER and dedicated location TZ/status files are persistent living documents. Update them whenever the user adds/changes a requirement, repeats an unresolved requirement, a meaningful code block is committed, or build/playtest changes the status of a task.

## Authority / conflict resolution
The project has accumulated historical passes, verifiers and reports. They are evidence, not equal-priority rules.

Current authority order is:
1. **Latest explicit user requirement and latest user-observed runtime evidence.**
2. **Current `AGENTS.md`.**
3. **Current `OSTER_CONFLICT_WORK_LEDGER.md`.**
4. **Current dedicated TZ / implementation-status file for the location or feature.**
5. Source implementation and active acceptance scripts.
6. Historical verifier scripts, old reports, old pass notes and old implementation assumptions.

When two instructions conflict, the higher item wins. A lower historical rule must never silently undo a newer accepted change.

**Mandatory stale-rule retirement:** whenever a newer user requirement/runtime result disproves an older rule, invariant, verifier expectation, fallback or compatibility path, update or delete the conflicting old requirement in the same work pass. Do not keep a stale behavior merely so an old verifier remains green. Update/retire the verifier instead. Historical reports may remain as chronology, but must be clearly non-authoritative for current behavior.

**No compatibility resurrection:** a deprecated runtime mutation/fallback that caused a confirmed regression must not survive in a second subsystem under a different marker/name. Compatibility shells may remain only when they perform no conflicting mutation/work.

## Mandatory workflow
1. Work directly in this GitHub repository. Do not create replacement ZIP archives as the default delivery method.
2. Before editing, verify the current branch and read all mandatory context files listed above.
3. Preserve existing project history and avoid destructive rewrites or force-updating branches unless explicitly required.
4. Keep fixes small and reviewable. Do not mix unrelated regressions into one change.
5. Do not commit generated UE folders: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`.
6. For user test feedback, fix the reported regression first, then update `OSTER_CONFLICT_WORK_LEDGER.md`. Add a short report in `OsterConflict/Docs/WorkReports/` only when a meaningful milestone or test pass is completed.
7. Reports must stay lightweight Markdown. Do not store copied build logs, screenshots, binaries, archives, or generated assets in the reports folder.
8. Never call an item `VERIFIED` or "done" merely because code was committed. Until UE build/runtime/user playtest confirms it, use `CODED_UNTESTED` in the ledger.
9. If the user repeats an unresolved request, increment its `Repeat` counter in the ledger instead of silently creating another duplicate task.
10. Before creating primitive/blockout geometry, inventory the already imported assets under `OsterConflict/Content` and prefer a suitable real asset when one exists.
11. For geography and landmark placement, user-confirmed local knowledge and verified photo/map evidence override old provisional code coordinates.
12. Do not let independent late runtime subsystems silently overwrite an already visible landmark. One site/landmark should have a clear placement owner.
13. Reference-photo retention is mandatory: when the user supplies images specifically to create a 3D model or location, preserve them in a named `REFERENCE_PHOTOS/<location>/` pack even if the user did not separately ask to save them. Ordinary bug/test screenshots are not automatically archived unless requested.
14. Every preserved photo pack needs an index/manifest linking it to its TZ and explaining chronology/role. Do not discard older references after a model is built; they are regression evidence.
15. **Playable-map size is user-authoritative.** Do not expand Oster to a full-town/procedural 2.4 km square simply because old blockout code can generate it. The hard current playable-area reference is `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg` and its manifest. The intended battlefield is the compact central Oster area shown there, covering the user-visible central street network and landmarks around Silpo / central park / Culture House through Stadium Oster and the Oster Local History Museum. Legacy peripheral BASE geometry, hydrography, roads or residential seeds outside that reference must not inflate gameplay/tactical-map bounds or consume runtime budget.
16. **Museum BASE means actual pawn placement, not a source-only marker.** For a normal local deployment the live player pawn must spawn on the Museum BASE approach and the runtime must prove its 2D distance to the Museum anchor. A verifier that only proves an `AOCTeamSpawnPoint` exists near the Museum is insufficient.
17. **Runtime content truth is fail-visible.** A missing HMMWV/M2/BTR source, missing weapon authored material, or fallback BasicShape material must never be printed/accepted as production-ready. Source/fresh-load checks must distinguish `mesh loads` from `authored materials ready`.
18. **Normal local game must not silently auto-fill a heavy bot population.** Bots/population are opt-in through explicit server/test options. Local visual/playtest launch should start without background filler bots so performance evidence measures the map/content rather than an unrequested AI load.
19. **Verifier truth follows current behavior, not history.** A verifier that requires a superseded constant, fallback, map extent, bot default, palette mutation, spawn proxy or old timing must be updated/retired before merge. Green CI is not allowed to depend on restoring a known regression.
20. **One runtime responsibility has one current owner.** If a newer subsystem takes ownership of spawn, material truth, map bounds or landmark placement, older subsystems may only observe or become inert; they must not mutate the same state later.
21. **Compact playable bounds apply at primary authoring time.** Do not create the old 2.4 km ground, far BASE compounds, peripheral hydrography, roads, residential grids or vegetation instances and rely on a later subsystem to delete them. Primary world generation must create/filter geometry against the current compact reference before `BeginPlay`; `OCCentralPlayableAreaSubsystem` is only a safety net for late/legacy instances.
22. **Tactical-map projection is bounded by the current playable-area reference.** Do not auto-fit the `M` map from arbitrary procedural component extents and do not restore historical minimum-size clamps that expand the map beyond the authoritative playable area. A component outside the hard reference must not make the tactical map zoom farther out.
23. **Runtime actor seeds must respect compact playable bounds.** BASE spawn actors, firing/destruction test lanes, civilian vehicles, combat-vehicle spawn points, pickups, props and other source-authored runtime actors must be created inside the authoritative compact Oster area from the start. Do not keep old edge coordinates and rely on a later relocation/trim pass. Primary/secondary BASE identity must not depend on retired edge-coordinate thresholds.
24. **Physical retirement beats inert resurrection.** When runtime evidence rejects a legacy `UWorldSubsystem`, visual owner, mutation layer or compatibility path and it no longer carries required data/collision responsibility, delete its `.h/.cpp` and retire the verifier/workflow that required it. Do not merely leave it compiled with `ShouldCreateSubsystem=false` “just in case”; Git history already preserves it.
25. **No historical verifier may require a runtime-rejected owner.** If a newer runtime result invalidates an older READY marker, asset family, mutation order or owner class, the historical verifier must be forward-ported or removed in the same corrective pass. CI must never force stale code back into production source.
26. **Mutating-owner exclusivity is stronger than pass chronology.** For one runtime responsibility there may be one mutating owner only. Legacy layers retained for data/collision may not call `SetMaterial`, `SetVisibility`, `SetHiddenInGame`, `Destroy`, `SpawnActor`, `SetActorLocation`, `SetActorTransform`, or equivalent mutation APIs on state owned by the current layer. If they do, consolidate or delete them.
27. **Legacy owner deletion is a tracked migration, not silent cleanup.** Every legacy owner deletion must be recorded in the current TZ and `OSTER_CONFLICT_WORK_LEDGER.md` with the rejected behavior, the replacement/current owner, and runtime status. Deletion alone is never proof of visual correctness; until local UE runtime confirms the replacement, status remains `CODED_UNTESTED`.
28. **Assistant-owned Git operations are mandatory whenever tooling permits.** The user must not be sent to perform branch creation, remote commit/push, PR updates, CI inspection, merge preparation, or other repository-side work that the assistant can execute through the connected GitHub tools. Perform those operations directly. Ask the user to touch GitHub Desktop/CMD only for genuinely local-only state that the connected tools cannot access, such as uncommitted files that exist only on the user's PC, switching the checked-out local working tree, or launching UE. In those local-only cases, explain the limitation explicitly and reduce the user's action to the smallest safe UI step. Never offload a multi-command Git procedure merely for convenience.
29. **Do not merge an unaccepted runtime branch into `main` merely to make local testing easier.** A runtime-recovery PR may be tested from its branch. Preserve uncommitted local work before switching branches; if those files exist only on the user's machine, remote GitHub tools cannot commit or move them and the user should be asked only for the minimal local preservation/switch action. The branch remains unmerged until factual runtime acceptance permits integration.
30. **Continue long-running TZ work from the latest factual checkpoint, not from zero.** When the user asks to continue a named TZ/pass from its latest factual/current checkpoint, first reconcile only the current branch, HEAD, active PR, recent commits, CI and the pass history/checkpoint. Consume newer parallel-chat commits, then resume the first factual open item and its direct dependencies. Do not replay already accepted/closed analysis. A broad re-audit is allowed only when ownership/architecture, engine/license, merge history, direct runtime evidence, or checkpoint integrity materially invalidates the previous checkpoint. For `PASS45_RUNTIME_RECOVERY_TZ.md`, `_DOCS/PASS45_CHECKPOINT_CONTINUATION_PROTOCOL.md` is binding.

### Hard playable-area map reference — 2026-08-24

![User-approved compact central Oster playable area](REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg)

This image is a boundary/topology reference, not decorative documentation. When an old coordinate, generated road, base, water proxy, residential seed, tactical-map auto-fit or other blockout feature expands the visible/playable map beyond this compact central area, the old blockout loses. Do not “fix” this by zooming the tactical UI out farther.

## Current priority
R13 location repair, visual/gameplay stabilization, and replacement of placeholders with existing real assets. Highest-priority tracked work is maintained in `OSTER_CONFLICT_WORK_LEDGER.md`, currently centered on:
- forcing the **actual live player pawn** to the Museum BASE and proving the runtime distance;
- shrinking gameplay/tactical-map bounds to the user-approved compact central Oster reference above;
- stopping the `120 FPS → ~4 FPS` startup collapse before adding more scenery;
- separating and correctly placing Museum / Silpo / Culture House / water tower;
- museum stadium-left + rear-slope + lower-residential topology;
- photo-driven location fidelity;
- eliminating late runtime rebuild/flicker;
- using existing imported houses/fences/lights/weapons/vehicles instead of visible primitive placeholders;
- refusing to call grey BasicShape weapon fallback or missing HMMWV/M2/BTR sources production-ready;
- validating the complete spawn-relative weapon test rack.

For the current Stadion Oster integration in `main`, the dedicated priority is the hard-georeferenced site defined by `STADION_OSTER_TZ.md`, `STADION_OSTER_IMPLEMENTATION_STATUS.md`, and `REFERENCE_PHOTOS/stadion_oster/`.

The detailed Stadion Oster acceptance requirements live in its dedicated TZ.
