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

### P7-LAUNCH-001 — test the branch that actually contains the fix
Previous launcher behavior allowed normal frontend playtest only from `main`. That made the intended workflow impossible: a correction branch could not be runtime-tested before merge, so an operator could unknowingly test stale `main` again.

Pass 7 changes the normal gameplay launcher contract:
- `main` still requires local HEAD == `origin/main`;
- `fix/runtime-acceptance-*` branches may run the same normal frontend flow before merge;
- an acceptance branch must match its own `origin/<branch>` HEAD;
- unrelated feature/backup branches remain blocked;
- Pass 7 source verifier runs before the UE build when present.

Status: `SOURCE VERIFIED / LOCAL WINDOWS EXECUTION NOT YET VERIFIED`.

## Exact UE 5.8 acceptance sequence

Run from `fix/runtime-acceptance-pass-7-20260822` using the existing user-facing launcher:

`START_HERE.cmd` → `1. ЗВИЧАЙНА ГРА`

Acceptance order:

1. Main menu appears with the approved Oster background and no 3D-world bleed.
2. Open `НАЛАШТУВАННЯ` from the main menu.
   - no transparent world behind the settings panel;
   - no one-frame flash of the 3D world;
   - close Settings and return to the same frontend state.
3. Press the main-menu `СТАРТ` once.
   - no gray intermediate shell;
   - deployment flow appears normally.
4. Complete Team → Squad → Role → Spawn.
   - final action reads `У БІЙ`, not a second `СТАРТ`.
5. Press `У БІЙ`.
   - loading presentation covers the full frame immediately;
   - deployment panel does not visibly shift underneath;
   - percentage is visible from the start and reaches `100%`;
   - overlay disappears only after gameplay possession/deployment release.
6. Confirm actual spawn is near the canonical Museum site, not the empty field.
7. Confirm the 11-weapon test rack is reachable from the actual spawn.
8. Confirm HUD minimap is present after gameplay entry.

## Merge gate

Do not merge PR #38 only because CI/source checks pass.
Merge gate remains fresh UE 5.8 runtime evidence for steps 1–8 above.

Current status: `IN PROGRESS · SOURCE VERIFIED · RUNTIME NOT VERIFIED · MAIN NOT MERGED`.
