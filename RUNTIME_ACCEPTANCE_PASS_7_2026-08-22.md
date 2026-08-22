# OSTER CONFLICT — RUNTIME ACCEPTANCE STATUS — 2026-08-22

## Current source state

The original Pass 7 branch is no longer the active source of truth.

Merged into `main`:

- PR #38 / Pass 7 → merge commit `bd633bfa3ed0b06aae06d1e0cf1e4d08e5c95952`;
- PR #39 / Pass 8 reconciliation → merge commit `d75d646840ad0fd788b7d9b134b5b5703536126a`;
- PR #40 / strict main runtime-acceptance launcher → merge commit `69b375a9ec91f5532b57442b1e1eee3527f2ab46`.

Old conflicting Pass 6 PR #36 was closed as superseded after its still-useful runtime changes were selectively reconciled through PR #39.

`SOURCE VERIFIED` is still not `VERIFIED RUNTIME`.

## Runtime-critical corrections now in main

### Settings world bleed
- pre-game Settings preserves the frontend backdrop;
- visible SettingsPanel is forced opaque;
- opening Settings must not expose the live 3D world for one frame;
- pause-menu Settings may still dim actual gameplay intentionally.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### START / deployment semantics
- main-menu action remains `СТАРТ`;
- final deployment action is `У БІЙ`;
- deployment must not present a second `СТАРТ`.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### Deployment loading
- full-screen opaque loading presentation;
- visible progress starts at `0%` and reaches `100%`;
- underlying deployment layout must not visibly move through the loading layer;
- completion is gated by player possession plus deployment UI release.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### Museum BASE / spawn
- canonical BASE is Museum-relative;
- server-side Museum guard repairs stale map-edge BASE actors through canonical `ConfigureServer()`;
- missing team BASE is created through the same canonical path;
- canonical BASE retains ground snap plus the 11-weapon rack;
- successful authoritative BASE readiness emits `PASS7_MUSEUM_BASES_READY`;
- old GameMode map-origin fallback is emergency-only and must not be the accepted runtime spawn route;
- obsolete generated map-edge BASE presentation is removed by the reconciled compatibility correction.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### 11-weapon Museum rack
The runtime rack contains these required classes:

1. AK-47
2. MP5
3. M1911
4. M700
5. Remington 870
6. M249
7. M14
8. MAC-10
9. TEC-9
10. Lever Action
11. Anti-Armor Launcher

Every rack actor must expose a visible `OC_ProductionWeaponVisual` component. StaticMesh production weapons are now accepted by the first-person presentation path instead of being treated as missing skeletal visuals.

Runtime markers:
- success: `PASS7_PRODUCTION_WEAPONS_READY`;
- failure: `PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL`.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### HMMWV + M2 Browning + BTR4
Normal gameplay requires:
- at least one HMMWV gun-truck actor;
- exact production HMMWV body;
- exact tagged production M2 Browning on every gun truck;
- at least one BTR actor;
- exact production BTR4 shell;
- production visual proxies fully inert when replaced, not merely invisible;
- M2 imported long axis normalized to Unreal forward convention;
- M2 muzzle presentation tied to the actual normalized production gun.

Runtime markers:
- success: `PASS7_PRODUCTION_VEHICLES_READY`;
- failure: `PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL`.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### First-person weapons / muzzle / tracer
Reconciled Pass 8 source now requires:
- StaticMesh and SkeletalMesh production weapon visuals both participate in first-person presentation;
- skeletal-only animation calls remain isolated to skeletal components;
- muzzle/tracer FX resolves the actual firing character's `CurrentWeapon`;
- remote actors are considered, not only the first local pawn;
- socket muzzle is preferred; production component bounds are the fallback;
- target-side network tracer is rebound to the actual weapon muzzle and rendered as a short streak.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### Minimap / chat / foliage / frontend travel
Reconciled Pass 8 source includes:
- compact minimap;
- compact Y team-chat / U global-chat UI;
- denser foliage with bounded incremental generation rather than synchronous whole-map population;
- pawn-less frontend travel isolation using the approved menu blocker/background;
- persistent `GameViewportClient::bDisableWorldRendering` is not used as the travel transition mechanism.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

## GitHub verification completed

Before PR #39 merged, all 9 relevant workflows passed on its exact head:
- Runtime reconcile Pass 8 source contracts;
- Source verification;
- Production model integration contracts;
- R14 weapon model contracts;
- R14 vehicle identity contracts;
- Museum source contracts;
- Frontend and deployment regression guard;
- Runtime acceptance Pass 3 source contracts;
- Runtime acceptance Pass 4 source contracts.

Before PR #40 merged, all relevant launcher/source workflows passed, including:
- Main runtime acceptance launcher contracts;
- Source verification;
- Runtime acceptance Pass 3;
- Runtime acceptance Pass 4;
- Runtime acceptance Pass 7.

The repository currently has no Windows or self-hosted UE 5.8 GitHub runner, so those workflows cannot prove compilation or gameplay runtime.

## Exact strict UE 5.8 acceptance route from main

Use the dedicated root launcher:

`RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd`

It delegates to the normal gameplay launcher but forces strict evidence mode on current `main`.

The launcher:
1. requires local `main` HEAD to equal `origin/main`;
2. hydrates required Git LFS payloads;
3. runs source verifiers, including Pass 7 and Pass 8 when present;
4. builds `OsterConflictEditor` with local UE 5.8;
5. opens every required real weapon asset in a fresh UE process;
6. imports and validates production HMMWV + M2 + BTR4 assets;
7. launches the normal frontend/gameplay route;
8. after the game closes, rejects any vehicle or weapon runtime FAIL marker;
9. requires all three READY markers:
   - `PASS7_PRODUCTION_VEHICLES_READY`;
   - `PASS7_PRODUCTION_WEAPONS_READY`;
   - `PASS7_MUSEUM_BASES_READY`.

## Visual/runtime acceptance sequence

1. Main menu shows the approved Oster background with no 3D-world bleed.
2. Open `НАЛАШТУВАННЯ`:
   - no transparent world behind the panel;
   - no one-frame 3D-world flash;
   - closing Settings returns to the same frontend state.
3. Press main-menu `СТАРТ` once:
   - no gray intermediate shell;
   - deployment flow appears normally.
4. Complete Team → Squad → Role → Spawn:
   - final action reads `У БІЙ`, not a second `СТАРТ`.
5. Press `У БІЙ`:
   - loading covers the full frame immediately;
   - percentage is visible from the start and reaches `100%`;
   - deployment UI is not visible moving underneath;
   - loading disappears only after gameplay possession/release.
6. Actual player spawn is near the canonical Museum site, not the empty field.
7. The 11-weapon rack is reachable and all 11 weapons show real production visuals.
8. First-person weapons are positioned correctly; muzzle flash/tracer originates from the actual firing weapon.
9. HMMWV shows the production HMMWV body and production M2 Browning with no proxy/civilian substitute.
10. BTR shows the production BTR4 shell with no primitive/proxy substitute.
11. Compact minimap is visible in gameplay.
12. Y opens team chat; U opens global chat.
13. Foliage is denser without a deployment freeze.
14. Remain in gameplay long enough for runtime validation to execute.
15. Exit normally and allow `RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd` to inspect the log.

## Current status

`MAIN UPDATED` · `SOURCE VERIFIED` · `STRICT MAIN ACCEPTANCE LAUNCHER READY` · `WINDOWS UE 5.8 COMPILE NOT YET EXECUTED` · `RUNTIME NOT VERIFIED`.
