# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest user runtime evidence overrides source-only claims and historical pass notes.
> Historical Pass 1–43 details remain preserved in Git history and `RUNTIME_*` / `Docs/WorkReports/*` reports; they are chronology, not current rules.

## 1. Current context

- Repository: `valentronus95/OsterConflict`
- Active correction branch: `fix/runtime-map-spawn-fps-assets-pass-44-20260824` → `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **only `START_HERE.cmd`**.
- Current root authority/conflict policy: `AGENTS.md`.
- Current hard playable-map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Do not add new decorative layers until spawn/map/FPS/material/content regressions below are closed.

## 2. Status rules

- `IN_PROGRESS` — latest runtime proves the problem still exists, or implementation is incomplete.
- `CODED_UNTESTED` — source fix exists on Pass 44 but has not been accepted by a new local UE 5.8 runtime.
- `VERIFIED BUILD` — build blocker removed by a later factual build.
- `VERIFIED RUNTIME` — only after a factual user/local UE runtime demonstrates the requested behavior.
- A green source verifier is never runtime acceptance.
- Actual live pawn position outranks spawn-point existence.
- Authored material/texture truth outranks a coloured fallback.
- Missing production source is a content gap, never a READY state.
- Old verifier expectations that contradict current user runtime are retired/updated, not restored.

## 3. Latest authoritative user runtime — 2026-08-24

The newest normal gameplay run reached gameplay, so it supersedes the earlier Pass 43-era "frontend crashes before gameplay" state.

Observed by the user:
- actual player spawn is still far from the Oster Local History Museum; the visible scene is effectively an empty field;
- the user must spawn near the Museum, not on a distant/legacy map location;
- tactical/playable map is still much larger than requested;
- user supplied a compact central-Oster map screenshot and explicitly repeated that the map must be reduced to that area;
- FPS starts around 120 and rapidly collapses to about 4;
- weapon rack still shows grey/placeholder material presentation;
- expected additional production models are still not visible;
- HMMWV / M2 Browning / BTR-4 availability must be judged by actual content/runtime, not optimistic launcher text.

This run is the current truth baseline for Pass 44.

## 4. Active requirements

| ID | Requirement | Repeat | Status | Current factual state / Pass 44 action |
|---|---|---:|---|---|
| GAME-SPAWN-001 | Actual live pawn spawns near Museum BASE | ≥9 | CODED_UNTESTED | Pass 44 adds `OCGameModeRuntimeSafe`: BASE restart explicitly selects nearest Museum team BASE, verifies the spawned pawn itself, corrects any >45 m displacement, emits `PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY/FAIL`. Base actors themselves are now authored beside Museum instead of at legacy ±1060 m edges, and primary/secondary identity no longer depends on ±920 m coordinate thresholds. New UE runtime required. |
| MAP-EXTENT-001 | Playable/tactical map matches compact central Oster user reference | ≥2 | CODED_UNTESTED | Pass 44 no longer relies only on post-start trimming. `OCWorldSectorOster` authors the primary ground directly as 960×940 m at x=-780..180 m / y=-120..820 m, filters out-of-bounds primitive/tree/residential authoring, retires far ±1040 m BASE compounds and peripheral hydrography. `OCGameMode` runtime seeds for BASE/test lanes/civilian/combat vehicles are also authored inside the compact area. `OCTacticalMapVisual` projects directly from the same hard four-corner bounds and removes the old ~1600 m minimum/auto-fit. `OCCentralPlayableAreaSubsystem` remains a safety net for late/legacy instances. Runtime visual acceptance required. |
| PERF-COLLAPSE-001 | Stop rapid ~120 FPS → ~4 FPS collapse | ≥5 | CODED_UNTESTED | Concrete hidden load found: base game defaulted to population 16 + bot autofill. Base `OCGameMode` now defaults target population to 0 unless population/bots are explicitly requested; explicit `BotFill=true` remains an opt-in fill request. `OCGameModeRuntimeSafe` also enforces zero implicit local filler bots. Obsolete weapon palette polling/mutation retired. Pass 44 prevents primary creation of the old 2.4 km ground/peripheral BASE/hydrography and removes old edge actors/vehicle spawn points/test lanes from runtime authoring. Runtime 20 s FPS test required. |
| GAME-WEAPONS-001 | 11 grounded pickup classes near actual Museum spawn | ≥9 | CODED_UNTESTED | Pass 42 12 cm grounding remains. Pass 44 acceptance now also requires the actual pawn Museum marker, not only rack/spawn actors. |
| WEAPON-MATERIAL-001 | Real weapon authored materials, no grey BasicShape disguise | ≥9 | CODED_UNTESTED / CONTENT CHECK | `OCRealWeaponFallbackSubsystem` no longer paints missing slots. Missing/default/BasicShape material becomes `PASS44_WEAPON_AUTHORED_MATERIAL_GAP`; rack gap is a strict failure. `OCWeaponPalettePass37Subsystem` is now inert compatibility shell: zero polling, zero material creation, zero SetMaterial. |
| VEH-PICKUP-001 | Real HMMWV + M2 Browning | ≥5 | CODED_UNTESTED / ASSET CHECK | Import no longer waits for BTR. HMMWV/M2 may import independently when their local sources exist. Fresh-load material checks distinguish mesh presence from authored material readiness. Pickup/HMMWV spawn points are now authored in the Museum core instead of old map edges. |
| ASSET-BTR-001 | Real BTR-4/Bucephalus model | ≥5 | IN_PROGRESS / CONTENT GAP | Local source recovery now searches BTR ZIPs more broadly, including generic FBX names inside BTR archives. If no real FBX is found, BTR remains explicit content gap and must not block HMMWV/M2 import or print fake READY. BTR spawn points are now inside Museum/Stadium core so a valid imported model is visible in the next runtime. |
| ASSET-M16-M4-001 | M16/M4 production weapon visuals | ≥2 | IN_PROGRESS / CONTENT GAP | No verified M16/M4 payload has been found in the checked GitHub repository/tree/history. Do not claim connected until a real source asset is found/imported. |
| LOC-MUSEUM-001 | Museum visibly present at the actual spawn | ≥9 | CODED_UNTESTED | Existing early R13.7/R13.8 scheduling and one-rebuild ceiling remain; Pass 44 fixes live spawn proof first. Runtime must visually show the Museum, not only markers/tags. |
| VIS-GRASS-001 | Natural grass without progressive FPS collapse | ≥5 | CODED_UNTESTED | LowCPU foliage remains bounded 200×200 m / 85 m cull around Museum. Primary old sector vegetation authoring is now clipped to compact bounds. Do not expand density until Pass 44 FPS run is stable. |
| UI-TACTICAL-MAP-001 | `M` map uses compact bounds and visible player marker | ≥3 | CODED_UNTESTED | Existing marker foreground retained. Pass 44 hard-binds `ReframeProjectionForCentralOster()` to x=-780..180 m / y=-120..820 m using the four transformed sector corners. Old component auto-fit, +300/+260 m padding and 800 m half-width minimum are retired. Runtime marker: `PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY`. |
| UI-MENU-001 | Frontend/menu remains stable | ≥8 | CODED_UNTESTED | Latest run reached gameplay, so prior RenderTargetPool crash is no longer the current primary blocker. Keep Pass 43 protections; do not regress them. |
| GAME-VEHICLE-INPUT-001 | WASD/mouse restored after vehicle exit | 1 | CODED_UNTESTED | Pass 41 adaptive one-shot recovery remains; runtime regression acceptance pending. |
| VIS-GRAPHICS-QUALITY-001 | No hidden blurry graphics downgrade | ≥3 | CODED_UNTESTED | Native 100% / Texture 3 automatic clarity target retained; no hidden low-FPS quality mutation. |

## 5. Pass 44 stale-rule retirement

The following older behavior is explicitly **superseded and must not return**:

1. `TargetPopulation = 16` + `bAutoFillBots = true` as implicit normal-local defaults, including the old `OCGameMode::InitGame()` fallback to `MaxPlayerSlots` when no bot/population option was supplied.
2. Treating existence of a near-Museum `AOCTeamSpawnPoint` as proof that the actual human pawn spawned there.
3. Treating the historical 2400×2400 m procedural ground/peripheral blockout as the required playable/tactical-map extent.
4. Creating that 2.4 km sector first and depending on a post-start trimmer as the primary map-size mechanism.
5. Creating BASE actors, firing/destruction lanes, civilian vehicles or combat-vehicle spawn points at retired edge coordinates and relying on later relocation/trim.
6. Determining primary/secondary BASE role from historical ±920 m Y-coordinate thresholds.
7. Tactical-map component auto-fit / historical 800 m half-width minimum that expands `M` beyond the user-approved compact area.
8. Runtime `BasicShapeMaterial` material recovery that paints missing weapon materials grey/dark and then calls the visual repaired.
9. Pass 37/38 palette mutation that creates runtime MIDs or calls `SetMaterial()` on placeholder weapon slots.
10. All-or-nothing vehicle import where missing BTR prevents available HMMWV/M2 from importing.
11. Launcher text that prints all production vehicles imported after the importer actually failed.
12. Old verifier rules whose only purpose is to keep any of the superseded behaviors above alive.

Current `AGENTS.md` requires stale conflicting rules/verifiers to be updated or retired in the same pass.

## 6. Pass 44 implementation state

Already coded on `fix/runtime-map-spawn-fps-assets-pass-44-20260824`:
- hard root rule precedence + stale-rule retirement policy;
- stored map reference + manifest;
- `OCGameModeRuntimeSafe` actual Museum pawn spawn proof;
- safe bot defaults and explicit-only local bot fill in both base `OCGameMode` and runtime-safe enforcement;
- compact primary `OCWorldSectorOster` authoring: 960×940 m ground, bounds-gated primitives/trees/residential seeds, no far legacy BASE compounds, no peripheral hydrography;
- compact `OCGameMode` runtime actor authoring: firing/destruction lanes, BASE seeds, civilian vehicles and HMMWV/BTR spawn points no longer use old edge coordinates;
- coordinate-independent Museum BASE primary/secondary resolution in `OCTeamSpawnPoint`;
- compact central playable-area runtime safety trim + tactical-map cache invalidation;
- hard tactical-map four-corner projection matching the playable bounds; old auto-fit/1600 m minimum retired;
- weapon authored-material fail-visible audit;
- complete retirement of runtime weapon palette mutation/polling;
- independent HMMWV/M2/BTR import path and broader BTR archive search;
- production fresh-load material validation;
- required weapon preflight now distinguishes mesh load from authored material status;
- runtime acceptance requires primary compact-world, compact runtime-seed, coordinate-independent BASE and compact-tactical-map evidence in addition to the existing Pass 44 gates;
- dedicated Pass 44 source verifier forbids old edge BASE/test-lane/combat-vehicle coordinates and old ±920 m BASE discriminator;
- Pass 3/4/19/20/33/36/37/38/41, S16A and launcher/tactical-map stale verifier expectations forward-ported/retired where they conflicted.

Status: **CODED_UNTESTED** until source CI + new local UE 5.8 run.

## 7. Merge gate / next execution order

Before `main`:
1. Run/update full source verifier suite including dedicated Pass 44 verifier.
2. Fix any old verifier that demands a superseded rule. Never reintroduce the regression to satisfy CI.
3. PR #78 is open; inspect Actions/CI on the latest head.
4. Merge only when source checks are green and branch is not behind `main`.

After merge/local pull:
1. `START_HERE.cmd → 1. ЗВИЧАЙНА ГРА`.
2. Confirm `PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY`.
3. Confirm `PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY`.
4. Confirm `PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY`, `PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY` and `PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY`.
5. Confirm `PASS44_COMPACT_PLAYABLE_AREA_READY`.
6. Open `M` and confirm `PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY`; visually verify the map no longer expands beyond the compact central-Oster reference.
7. Confirm `PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY` and visually verify Museum is actually nearby.
8. Check rack authored materials; any `PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP` keeps visual acceptance FAIL.
9. Remain in gameplay ~20 s only while thermals are sane; require >=30 FPS and no progressive collapse.
10. Verify HMMWV/M2/BTR individually. A missing one is a named content gap, not a hidden proxy success.

**Current prohibition:** no new decorative map layers or map expansion until this runtime baseline is stable.