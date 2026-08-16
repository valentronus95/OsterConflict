# S12 test matrix

1. Start server in default mode. Confirm Conquest tickets/bleed still work.
2. Start with `?Mode=Sandbox`. Confirm top HUD says `SANDBOX / TEST RANGE` and tickets do not bleed.
3. Press F10 in Sandbox. Panel opens; F10 outside Sandbox does not grant admin panel.
4. Spawn weapon rack. Confirm AR/SMG/pistol/sniper/shotgun/LMG/anti-armour launcher + ammo box appear.
5. Spawn civilian car, gun truck and BTR from admin panel. Confirm they are server-spawned and visible to second client.
6. Teleport Museum/Stadium/Park/College. Confirm character moves to expected map anchors.
7. Toggle light in enterable house on client A. Confirm client B observes same state.
8. Open/close new yard gate. Confirm replicated state and collision passage.
9. Reset interactables. Door/gate/light return to defaults.
10. Toggle god mode. Confirm authority rejects incoming player damage while enabled and accepts after disabling.
11. Cycle grenade with `4`; confirm HUD selection/count. Throw Frag with `F`; both clients observe projectile/detonation and server applies damage.
12. Throw Smoke; confirm replicated smoke actor exists for both clients and expires.
13. Throw Flash; verify distance/LOS/facing modify local flash presentation.
14. Engineer: press `N` to cycle all 15 game-only trap presets and `M` to deploy; non-Engineer cannot deploy.
15. Engineer disarms enemy deployable with `E`; non-Engineer cannot.
16. Engineer repairs a damaged non-wreck vehicle with `E`; health/damage stage improves on server.
17. Fire anti-armour launcher at BTR. Confirm BTR accepts dedicated anti-armour damage.
18. Fire rifle or gun-truck MG at BTR. Confirm S11 armour filter still rejects hull damage.
19. Kill/respawn in Sandbox. Confirm normal respawn works but no ticket loss/end-of-round is triggered.
20. Run S04-S11 structural verifiers after S12 changes.
