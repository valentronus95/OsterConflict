# Runtime playtest audit — 2026-08-23 — Pass 35 trigger

Status: **runtime regression evidence / fix CODED_UNTESTED**.

Latest user playtest reached gameplay but rejected the current result:

- player appeared at the BASE weapon rack in an apparently empty field instead of seeing the museum nearby;
- Tactical Map opened, but the local player marker was not visibly distinguishable at the museum/objective cluster;
- most restored rack weapon meshes rendered white/grey while AK retained authored color;
- FPS was briefly around 32 and then fell to roughly 7–4 FPS.

Source localization for Pass 35:

1. `OCR137MuseumPhotoModelSubsystem::BuildMuseum()` aborts the entire R13.7 owner when the optional rural-cabin roof mesh is unavailable (`!Cube || !Basic || !RoofMesh`).
2. `OCR138MuseumInteractiveArchitectureSubsystem::UpgradeMuseum()` refuses to build the authoritative enterable core when that R13.7 owner actor is missing. This can leave a correctly relocated BASE looking at an empty museum site.
3. The Tactical Map player marker is created at Z=20 while dynamic objective markers use Z=22. Because BASE is near Museum/A, objective UI can cover the local-player triangle.
4. Weapon material loss and the progressive FPS collapse remain separate active regressions and are not declared solved by Pass 35.

Pass 35 therefore adds a late idempotent museum owner/core recovery and a presentation-only Tactical Map marker priority guard. It does **not** move the canonical Museum geo anchor and does **not** claim runtime verification.
