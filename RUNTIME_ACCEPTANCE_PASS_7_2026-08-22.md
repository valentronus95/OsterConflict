# OSTER CONFLICT — RUNTIME ACCEPTANCE PASS 7 — 2026-08-22

## Scope

Branch: `fix/runtime-acceptance-pass-7-20260822`
Base: `main` at `d9c0a859a5273cec9499e794e5efaac531ddbfb5`
PR: #38 (Draft)

This pass exists because repeated runtime defects must not be marked done from source inspection alone.
`SOURCE VERIFIED` is not `VERIFIED RUNTIME`.

## Source changes in Pass 7

### P7-UI-001 — settings world bleed
- pre-game Settings keeps the frontend backdrop instead of exposing the 3D world;
- SettingsPanel is forced fully opaque while visible;
- the presentation backing layer is selected before `UIOpenSettings()` changes visibility state, preventing a one-frame world flash;
- pause-menu Settings may still dim live gameplay intentionally.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### P7-START-001 — double START semantics
- main-menu action remains `СТАРТ`;
- final deployment action is now `У БІЙ`;
- the deployment presentation must not rewrite the final action back to a second `СТАРТ`.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### P7-LOAD-001 — deployment loading transition
- deployment loading is a full-screen blocking presentation;
- underlying deployment layout must not be visible while possession changes;
- visible progress starts at `0%` and reaches `100%`;
- completion remains gated by a possessed pawn plus closed deployment UI;
- the loading title is distinct from START/deployment actions.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### P7-SPAWN-001 — Museum BASE guarantee
- the canonical `AOCTeamSpawnPoint` path remains Museum-relative and still owns ground snap plus the 11-weapon rack;
- a server-side gameplay-world guard repairs a stale map-edge BASE through the same canonical `ConfigureServer()` path;
- if either team BASE is missing, the guard creates it at the Museum anchor and lets `ConfigureServer()` apply the canonical team offset;
- successful authoritative BASE readiness emits `PASS7_MUSEUM_BASES_READY`;
- the old GameMode map-origin fallback remains tracked as emergency-only code and must not be the accepted player route.

Status: `SOURCE VERIFIED / RUNTIME NOT VERIFIED`.

### P7-VEHICLE-001 — production HMMWV + M2 + BTR4 must fail closed
Normal gameplay fleet slots are confirmed to create `AOCHMMWVGunTruck` and `AOCBTR` actors. A zero-count runtime check is therefore not valid evidence.

Pass 7 vehicle gate now requires:
- at least one runtime HMMWV gun truck;
- at least one runtime BTR;
- exact production HMMWV body on every HMMWV actor;
- exact tagged production M2 Browning on every runtime gun truck;
- exact production BTR4 shell on every BTR actor;
- `PASS7_PRODUCTION_VEHICLES_READY` only when those conditions pass;
- `PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL` on mismatch or missing expected fleet;
- an invalid proxy actor is hidden and collision-disabled instead of remaining in the world as a fake final vehicle.

The normal fleet does not currently require the optional production pickup actor to exist. If one exists, it must still use its expected body plus production M2.

Status: `SOURCE VERIFIED / UE COMPILE NOT VERIFIED / RUNTIME NOT VERIFIED`.

### P7-LAUNCH-001 — test the branch that actually contains the fix
Previous launcher behavior allowed normal frontend playtest only from `main`. That made the intended workflow impossible: a correction branch could not be runtime-tested before merge, so an operator could unknowingly test stale `main` again.

Pass 7 changes the normal gameplay launcher contract:
- `main` still requires local HEAD == `origin/main`;
- `fix/runtime-acceptance-*` branches may run the same normal frontend flow before merge;
- an acceptance branch must match its own `origin/<branch>` HEAD;
- unrelated feature/backup branches remain blocked;
- Pass 7 source verifier runs before the UE build when present;
- after an acceptance-branch playtest, the launcher rejects any `PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL` marker;
- after an acceptance-branch playtest, the launcher requires both `PASS7_PRODUCTION_VEHICLES_READY` and `PASS7_MUSEUM_BASES_READY` before the automated runtime evidence gate can pass.

Status: `SOURCE VERIFIED / LOCAL WINDOWS EXECUTION NOT YET VERIFIED`.

## Exact UE 5.8 acceptance sequence

Run from `fix/runtime-acceptance-pass-7-20260822` using the existing user-facing launcher:

`START_HERE.cmd` → `1. ЗВИЧАЙНА ГРА`

Acceptance order:

1. Launcher builds the exact current acceptance-branch HEAD successfully in UE 5.8.
2. Main menu appears with the approved Oster background and no 3D-world bleed.
3. Open `НАЛАШТУВАННЯ` from the main menu.
   - no transparent world behind the settings panel;
   - no one-frame flash of the 3D world;
   - close Settings and return to the same frontend state.
4. Press the main-menu `СТАРТ` once.
   - no gray intermediate shell;
   - deployment flow appears normally.
5. Complete Team → Squad → Role → Spawn.
   - final action reads `У БІЙ`, not a second `СТАРТ`.
6. Press `У БІЙ`.
   - loading presentation covers the full frame immediately;
   - deployment panel does not visibly shift underneath;
   - percentage is visible from the start and reaches `100%`;
   - overlay disappears only after gameplay possession/deployment release.
7. Confirm actual spawn is near the canonical Museum site, not the empty field.
8. Confirm the 11-weapon test rack is reachable from the actual spawn.
9. Confirm HMMWV uses the real production HMMWV body and real M2 Browning, with no civilian pickup/proxy turret substitute.
10. Confirm BTR uses the real production BTR4 shell, with no primitive/proxy body substitute.
11. Remain in gameplay long enough for runtime validation to execute; no vehicle `FAIL` marker may exist and the launcher must later find `PASS7_PRODUCTION_VEHICLES_READY`.
12. Confirm HUD minimap is present after gameplay entry.
13. Exit the game normally and let the acceptance launcher inspect the log.
   - `PASS7_PRODUCTION_VEHICLES_READY` required;
   - `PASS7_MUSEUM_BASES_READY` required;
   - any `PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL` rejects the run.

## Merge gate

Do not merge PR #38 only because GitHub source CI passes.
Merge gate remains a successful UE 5.8 compile plus fresh runtime evidence for steps 2–13 above.

Current status: `IN PROGRESS · SOURCE VERIFIED · UE COMPILE NOT VERIFIED · RUNTIME NOT VERIFIED · MAIN NOT MERGED`.
