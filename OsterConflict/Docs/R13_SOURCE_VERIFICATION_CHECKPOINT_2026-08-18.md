# R13 source-verification checkpoint — 2026-08-18

## Verified source state

GitHub Actions `Source verification` completed successfully at commit `84e15bc900f5d003748805ff4b68340e9d416570` (run #238).

The Windows source suite executes `RUN_ALL_VERIFY.py` and currently covers, among other existing S04–S19/R6–R11 gates:

- approved R13 frontend backdrop, neutral background tint and legacy-layer suppression;
- frontend-only pawn-leak protection and Escape/input regressions;
- transactional Oster fence/prop bridges;
- 14-bench central-park replacement and rollback;
- landmark windows, museum roof and museum chimney bridges;
- neutral outdoor lighting and pickup first-person camera regressions;
- bounded runtime retry/rollback for R13 weapon pickups and BoxTruck spawn points;
- current gameplay-polish expectations for menu sizing, pause dimming, hidden primitive FPS hands and BTR proxy handling.

## Runtime spawn hardening

- Weapon and BoxTruck variant bridges retry up to 20 times with a 0.5 s retry delay while waiting for `AOCWorldSectorOster`.
- Both skip clients and standalone frontend-only sessions.
- Partial weapon sets are destroyed before retry to prevent duplicates.
- BoxTruck spawn points use UE 5.8 deferred spawning so `BoxTruck` style is configured before `BeginPlay`; both pending spawn points must be prepared before finalization.

## Still required

This checkpoint is source-only. It does **not** replace the local Unreal Engine validation gate.

Before PR #2 is merge-ready, run the installed Windows UE 5.8.x compile path and validate the runtime visually, especially:

- startup menu backdrop and Escape/resume input;
- character/weapon presentation;
- BoxTruck and weapon variant spawns;
- fences, park benches and museum/college partial art bridges;
- collision and interaction behavior around replaced proxies.
