# Root-cause evidence

This pass is deliberately root-cause based rather than another cosmetic patch.

- `OCR13FrontendMenuSubsystem` owns menu geometry at `(112,92)` / `440x760`; `OCR13UIViewportStabilizerSubsystem` previously overwrote it at runtime. That duplicate owner is removed.
- `AOCTeamSpawnPoint::ConfigureServer` held Museum relocation logic, but serialized base actors were not guaranteed to execute it. `BeginPlay` now reasserts the contract.
- Dense foliage returned permanently from `OnWorldBeginPlay` while frontend-only. Because frontend/gameplay share `OsterConflict_Runtime`, no second begin-play event existed. Population now waits for gameplay readiness.
- Fire hit validation originates at the camera; visual FX used the same start. Local tracer/muzzle presentation now resolves against visible current-weapon geometry while keeping ballistic validation unchanged.
- The normal launcher previously did not guarantee HMMWV production import and dedicated M2/BTR helpers allowed generated approximations. Acceptance now requires the full production vehicle importer and real local source payloads.
