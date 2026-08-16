# Oster Conflict R11 — Visual Foundation QA

## Scope
R11 keeps the existing R10 gameplay/network code and replaces the most disruptive source-only presentation failures with deterministic engine-owned runtime visuals.

## R11 changes
- Runtime replicated daylight/atmosphere actor: directional sun, SkyAtmosphere, SkyLight, height fog.
- Semantic runtime material tint pass for roads, ground, buildings, roofs, windows, fences, vegetation, water and landmarks.
- Added compact base-area geometry so both team spawn zones have visible ground reference, cover, shelters, fence and approach road.
- Hidden authoring/reference marker meshes and labels during play.
- Composite source-only weapon silhouettes, with a separate root so first-person placement is not distorted by mesh scale.
- Transient local combat FX actor for muzzle flash, partial tracer and impact flash; weapon combat presentation no longer uses DrawDebug primitives.
- Less box-like first-person arms/hands and third-person proxy legs.
- Deterministic vehicle body/glass/wheel palette.
- Launcher UE 5.8 validation path now uses Build.bat; dedicated Client/Server targets are compiled only on source UE builds.
- R11 quick listen-server launch creates the generated runtime map automatically on a fresh archive.

## Verification boundary
`RUN_ALL_VERIFY.py` is the source/static regression gate bundled with this archive. The final authoritative UE C++ compile must run on Windows with UE 5.8 via `START_HERE.cmd -> 1` because Unreal Engine is not installed in the packaging environment used to produce this archive.

## First test
1. Extract R11 to a new folder.
2. Run `START_HERE.cmd` -> `1`.
3. After compile success, run `START_HERE.cmd` -> `4`.
4. Do not copy R10 `Binaries`, `Intermediate`, `Saved` or `DerivedDataCache` into R11.
