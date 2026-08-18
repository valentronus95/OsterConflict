# OSTER CONFLICT — PROJECT CONTEXT

> Persistent source-owned project context for continuation work. Keep this file concise enough to remain useful and update it only when a checkpoint materially changes.

## Current checkpoint — R13 content + gameplay pass

- Active integration branch: `r13-content-gameplay-pass`.
- Draft PR: #2, `R13 whole Oster content + gameplay pass`.
- R13 preserves the source-owned Oster topology and existing multiplayer/gameplay backend while progressively replacing greybox visuals with bundled art.
- Never rebuild the project from scratch and never reconstruct large source files from partial snippets. Prefer isolated subsystems and small commits.

## Validation checkpoint

- GitHub Actions `Source verification` now runs for pushes and pull requests on Windows and executes `RUN_ALL_VERIFY.py`.
- Source verification is GREEN at commit `84e15bc900f5d003748805ff4b68340e9d416570` (workflow run #238).
- This source-only result does **not** claim an Unreal Engine compile or runtime visual pass.
- Full local Windows UE 5.8.x `OsterConflictEditor` + `OsterConflict` compile is still required before the draft PR is merge-ready.

## Main-menu contract

- Use `/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG` as the approved static startup backdrop.
- Do not globally tint or darken the approved main-menu background.
- The main menu uses only a local left-side black feather; full-frame shade is pause-only.
- Keep the menu narrow/transparent and keep the approved top-level actions: `СТАРТ`, `ЛОКАЛЬНА ГРА`, `МЕРЕЖЕВА ГРА`, `НАЛАШТУВАННЯ`, `ВИЙТИ З ГРИ`.
- Legacy frontend/background layers must stay suppressed.
- Standalone frontend-only sessions must not own a gameplay pawn; `OCR13FrontendShellGuardSubsystem` enforces this so the startup menu cannot fall into the live-world pause presentation.
- Escape/resume must restore gameplay input, clear ignored move/look state and flush pressed keys.

## Environment / Oster art bridge

- Whole-Oster roads and sidewalks use bundled Roadside Construction art.
- Current environment bridge uses AdvancedVillage houses/trees and PN foliage where semantically safe.
- `WoodFences`, `LightSheetFences`, Krushelnytska pole replacements and the temporary `MetalFences` visual bridge are transactional per source family: incomplete replacement keeps the original proxy.
- Central-park bench bridge requires all 14 expected bench proxies and rolls back on failure; unrelated mixed `ParkDetails` must remain untouched.
- Landmark windows use framed glass while source massing remains unchanged.
- Museum roof bridge targets exactly 8 pitched museum panels; flat college roofs remain untouched.
- Museum chimney bridge targets exactly 2 museum chimney proxies; other `LandmarkDetails` remain untouched.
- Do not substitute unrelated generic/medieval assets for recognizable Oster landmarks just to remove greybox geometry.

## Characters / weapons / vehicles

- Authored faction character art wins when available; bundled Manny/Quinn + locomotion are the current fallback before primitive third-person proxy art.
- Primitive first-person proxy arms/hands remain structurally defined but hidden. Production FPS arms are shown only when an authored profile mesh is available.
- Existing weapon art connected: AK-47, MP5, M1911, M700, M14, Lever Action, MAC-10, TEC-9. Shotgun/LMG/launcher still use existing fallback art.
- Civilian art connected: hatchback, sports-car sedan, SUV/wagon, pickup and BoxTruck.
- BTR intentionally remains a gameplay proxy until a real APC/BTR asset is installed. Never disguise it with civilian vehicle art.

## Runtime variant spawn hardening

- Extra R13 weapon pickups and BoxTruck spawn points skip frontend-only sessions and clients.
- Both bridges use bounded retry (20 attempts, 0.5 s retry delay) while waiting for `AOCWorldSectorOster`.
- Weapon variant spawn is all-or-nothing: partial pickup sets are destroyed before retry so successful seeds cannot duplicate.
- BoxTruck spawn points use UE 5.8 deferred spawning, are configured as `BoxTruck` before `BeginPlay`, and are finished only after both pending spawn points are prepared.
- `VERIFY_R13_RUNTIME_SPAWN_BRIDGES.py` guards this behavior and is included in `RUN_ALL_VERIFY.py`.

## Known production-art gaps

- faction-specific military character bodies/gear;
- authored first-person arms/hands and weapon animation set;
- real BTR/APC art;
- production metal-fence art;
- final purpose-built park furniture polish;
- exact production museum/college facade/massing art;
- stadium, Silpo, bus station and remaining recognizable Oster landmark art.

## Local Windows validation path

1. Pull `r13-content-gameplay-pass`.
2. Run `START_HERE.cmd`.
3. Option `1` runs source verification first, then compiles `OsterConflictEditor` and `OsterConflict` using installed UE 5.8 `Build.bat`.
4. Treat the UE compiler/runtime output as authoritative for the next fix cycle.
