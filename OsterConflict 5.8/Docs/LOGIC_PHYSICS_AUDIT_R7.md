# Oster Conflict — Logic / Physics Audit R7

Date: 2026-08-13  
Baseline: MASTER-ТЗ v4.2 CORRECTED  
Source: R6 PRE-COMPILE HARDENED → R7 LOGIC + PHYSICS HARDENED  
Status: **SOURCE/STATIC HARDENING PASS; UE 5.8 EXECUTION PENDING**

## 1. Scope of this audit

- Full source-tree structural/preflight scan: 129 C++ headers/sources, 44 RPC declarations.
- Manual logic/physics review of the critical runtime path: Character, CharacterMovement integration, WeaponBase, Health/downed/revive, grenades/anti-armour, vehicles/armed vehicles/BTR/spawn points, AI/perception/navigation fallback, GameMode/GameState/capture/respawn/round restart, doors/gates/windows/destructibles/smoke/traps/gore, PlayerController/PlayerState authority paths.
- Error-class scan across the full C++ tree for per-frame iteration/ticks, direct teleports, damage paths, timers, server RPC state validation and physics simulation calls.
- This is still not a substitute for UHT/C++ compilation or runtime physics measurement.

## 2. What R7 fixed

- Correct `MaxWalkSpeedCrouched`; downed crawl no longer inherits a faster generic crouch speed.
- Source-proxy hit-zone damage now affects gameplay. Head/neck has an elevated multiplier; lower-limb reduction no longer swallows the lower torso/pelvis band.
- Server reload and fire-mode mutations require an alive state, closing a direct-RPC downed-state inconsistency.
- Smoke blocks AI gameplay LOS, and currently perceived actors are additionally filtered through that LOS before becoming a combat target.
- Frag explosions apply bounded physics impulse only to simulated light bodies; heavy bodies are excluded and solid LOS blockers prevent impulse through a wall.
- Breakable windows reset between rounds; cosmetic shards collide with world geometry, ignore pawns, and do not simulate on dedicated server.
- Destructible props reset; transient chunks are tracked/cleaned, ignore pawns, and are skipped on dedicated server.
- Dismemberment cosmetic chunks ignore pawn collision.
- Gates use `ReplicatedUsing=OnRep_Open`, tick only during interpolation, and disable idle tick.
- Vehicle steering uses vehicle-local up; no forced minimum pivot torque at standstill.
- Coarse vehicle damage stages reduce power/grip/steering instead of being presentation-only.
- Vehicle damage passes the same friendly-fire authority policy as infantry damage.
- Driver/gunner exit searches multiple candidates with `UWorld::FindTeleportSpot` rather than one unsafe hardcoded exit.
- BTR proxy has four axles / eight suspension contacts matching eight visible wheels; extra wheels animate.
- Round restart resets K/D/R/Score, capture points, doors, gates, lights, windows, destructibles, smoke/traps, vehicle ownership and the vehicle fleet.
- Vehicle spawn points have a deterministic round-reset path rather than inheriting damaged vehicles or pending respawn timers.

## 3. Confirmed remaining gaps — do NOT call these complete

### P0 / release-relevant

1. **No real UE 5.8 UHT/C++ compile yet.** Static checks cannot prove API/build correctness or runtime physics.
2. **No production `.umap`, NavMesh, World Partition/HLOD or packaged traversal evidence.**
3. **Spawn danger is ranking-only.** The safest point is selected, but there is no hard LOS/distance reject + delayed retry if every spawn is under direct threat.
4. **AI hearing is absent.** AI Perception configures sight; shots/explosions do not yet create a proper hearing-stimulus layer.
5. **AI has no useful last-known-position/memory model.** Smoke now prevents acquisition/fire through the cloud, but richer search/memory behavior is not implemented.
6. **Vehicle damage remains coarse.** No independent tire puncture, engine module, turret-disable, drivetrain/differential or authored damage zones.
7. **Vehicle ground physics are generic.** No PhysicalMaterial-driven asphalt/dirt/grass/wet grip/rolling-resistance model.
8. **Vehicle AI is primitive.** No road graph, obstacle-aware vehicle routing or coordinated driver+gunner crew tactics.
9. **Passenger seats are absent.** The MASTER P0 criterion explicitly needs driver+gunner, but other MASTER text/QA also mentions passengers; that contract and code are not yet aligned.
10. **Water gameplay/physics remain proxy-only.** No final shallow/deep-water hazard, drowning/OOB or deterministic vehicle-water policy.
11. **Wreck lifetime remains below the 90–180 s MASTER baseline.** R7 deliberately does not raise it blindly before packaged performance/cap testing.
12. **Round end currently restarts the gameplay round directly.** A dedicated summary/redeployment flow matching the final S19C UX state graph is not yet a proven runtime path.

### Realism / P1+ / production-content gaps

13. Normal firearms are server hitscan; no ordinary-bullet travel time, gravity/drop, penetration or ricochet layer.
14. No near-wall weapon lowering/pose system yet.
15. Character hit zones are capsule/source-proxy bands; production skeletal hitboxes and PhysicsAsset are required.
16. Final ragdoll/dismemberment quality requires production skeletal meshes/PhysicsAssets; current detached chunks are placeholders.
17. No AI tactical grenade/gadget use, deliberate flank or suppression/retreat state machine.
18. No advanced vehicle client prediction/resimulation; high-ping vehicle feel is unknown until packaged testing.
19. No production Chaos/Chaos Modular Vehicle migration; current server rigid-body + raycast suspension is a prototype foundation.
20. BTR armour currently accepts anti-armour damage only; vehicle-cannon/heavy-direct-fire penetration policy needs an explicit balance decision.
21. Final impact/traction surfaces need authored Physical Materials, not just actor tags/proxy rules.
22. Infantry acceleration/braking/air-control/fall behaviour mostly inherits Unreal CharacterMovement defaults; only speeds/state constraints are project-tuned so far.
23. There is no explicit project fall-damage model. The current MASTER does not make fall damage a hard P0 acceptance item, so this is a design decision/gap, not a failed acceptance criterion.
24. Vehicle crash damage affects the vehicle, but occupant injury from severe collision is not a defined system.
25. D2/D3 destruction is still source/proxy authored-state logic; production Chaos geometry collections and structural acceptance require content/runtime work.

## 4. Logic verdict

Core match authority, server fire cadence/ammo/reload, damage/downed/revive, Conquest capture/tickets, Human+Bot population, squads/chat, driver/gunner ownership and interactions are structurally present. R7 removes several state/authority/lifecycle defects that would otherwise contaminate the first multi-round test.

The game is **not logic-complete for release** until the P0 gaps above are implemented, explicitly re-scoped, or proven through the real runtime gates.

## 5. Physics verdict

Current physics is deliberately hybrid/prototype:

- infantry: Unreal CharacterMovement foundation;
- grenades/anti-armour: ProjectileMovement;
- vehicles: server-authoritative rigid body + raycast suspension;
- windows/destructibles/gore: bounded cosmetic physics;
- production Chaos/content assets: pending.

R7 is materially more coherent than R6, especially for downed movement, explosion occlusion/impulse, vehicle steering/damage, BTR suspension, debris collision, smoke/AI and multi-round reset. It is still not final vehicle/destruction physics.

## 6. Static verification status

- Root source regression: PASS.
- Internal source gates: 25/25 PASS.
- R6 launch regression retained under R7: 92/92 PASS.
- R7 logic/physics verifier: 48/48 PASS.
- S18C static preflight: 129 C++ headers/sources, 44 RPC declarations, PASS.
- Release-tree audit: PASS.
- Real UHT/C++/cook/package/runtime: **PENDING**.

## 7. Required real-runtime gate

1. `START_HERE.cmd` → `1. Compile only`.
2. Editor/Client/Server UHT+C++ must pass.
3. Full validation creates project-owned release map and Automation report.
4. Packaged dedicated server + two packaged clients smoke.
5. Multi-round test specifically checks round reset, vehicle reset, body/head damage, downed crawl, smoke vs bots, frag impulse/occlusion, BTR suspension, safe vehicle exits and friendly fire.
6. Then latency/loss/jitter, collision/vehicle-route, destruction stress and performance scenarios.

Until those artifacts exist: **EXECUTION PENDING**.
