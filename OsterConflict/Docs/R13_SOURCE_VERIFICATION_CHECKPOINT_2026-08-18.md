# R13 verification checkpoint — 2026-08-18

## Current branch state

Branch: `r13-content-gameplay-pass`

Current checked head: `9d2dc208b523d13cda16c6996cd000faf8bf70d3` (`Include tickable subsystem linkage verification`).

GitHub Actions `Source verification` completed successfully for this head in run #286. The source gate therefore reflects the current R13 head, not an older checkpoint commit.

The Windows source suite executes `RUN_ALL_VERIFY.py` and currently covers, among the existing S04–S19/R6–R11 gates:

- approved R13 frontend backdrop, neutral background tint and legacy-layer suppression;
- frontend-only pawn-leak protection and Escape/input regressions;
- transactional Oster fence/prop bridges;
- 14-bench central-park replacement and rollback;
- landmark windows, museum roof and museum chimney bridges;
- neutral outdoor lighting and pickup first-person camera regressions;
- bounded runtime retry/rollback for R13 weapon pickups and BoxTruck spawn points;
- tickable world-subsystem linkage verification;
- gameplay-polish expectations for menu sizing, pause dimming, hidden primitive FPS hands and BTR proxy handling.

## Generated-folder regression status

The archived Windows compile evidence from `PC_TEST/TEST_RESULTS/20260816_235115` recorded an old static-verifier failure because a local generated `Binaries` directory existed during S05 source verification.

That failure is no longer representative of the current source gate. `RUN_ALL_VERIFY.py` now:

- rejects generated UE output if it is tracked by Git;
- temporarily isolates local `Binaries`, `Intermediate`, `Saved` and `DerivedDataCache` directories before source-only checks;
- restores those directories in `finally` after verification.

This is why the current GitHub source-verification run can pass without confusing legitimate local Unreal build output with committed source artifacts.

## Windows compile evidence

The archived UE 5.8.1 compile run at `PC_TEST/TEST_RESULTS/20260816_235115` records:

- UE 5.8.1: PASS;
- R8 prelaunch: PASS;
- Visual Studio C++ detection: PASS;
- S18C toolchain preflight: PASS;
- Compile Editor: PASS;
- Compile Game: PASS;
- dedicated-server compile: skipped because the machine used an installed/Launcher Unreal Engine build.

This evidence proves that the project compiled on that archived branch state. It does **not** prove that the current R13 head has completed a fresh local UE compile after all later commits.

## Runtime spawn hardening

- Weapon and BoxTruck variant bridges retry up to 20 times with a 0.5 s retry delay while waiting for `AOCWorldSectorOster`.
- Both skip clients and standalone frontend-only sessions.
- Partial weapon sets are destroyed before retry to prevent duplicates.
- BoxTruck spawn points use UE 5.8 deferred spawning so `BoxTruck` style is configured before `BeginPlay`; both pending spawn points must be prepared before finalization.

## Remaining merge gate

PR #2 remains draft until the current head passes fresh local Unreal Engine 5.8.x validation. Required checks:

1. Run `START_HERE.cmd` option 2 (`Full validation`) or option 3 (`Clean full validation`).
2. Confirm current-head Editor/Game compile succeeds.
3. Run option 4 and visually validate the R13 listen-server gameplay session.
4. Check startup menu backdrop and Escape/resume input.
5. Check character and weapon presentation, including first-person camera clearance.
6. Check BoxTruck and weapon variant spawns for duplication or missing actors.
7. Check fences, park benches and museum/college partial art bridges.
8. Check collision and interaction behavior around replaced proxies.

Only after those current-head runtime checks pass should PR #2 be considered merge-ready.
