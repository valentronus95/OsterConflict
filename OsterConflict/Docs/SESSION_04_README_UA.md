# Oster Conflict — S04 Weapon Framework

## Статус
S04 source implementation complete. Unreal Engine compile/runtime validation must be performed on a Windows machine with the UE 5.8 toolchain.

## Що додано
- Primary / Secondary inventory slots.
- Starter loadout: OC-AR1 + OC-PST1.
- Keys `1` / `2` switch slots.
- `E` interacts with the nearest server-validated pickup/ammo box inside 260 cm.
- `G` drops the current weapon into the replicated world.
- Picking up a weapon replaces only its preferred slot and drops the replaced weapon.
- World weapons remain network actors and may be picked up again.
- 6 prototype weapon behavior classes:
  - OC-AR1 — assault rifle;
  - OC-SMG1 — SMG;
  - OC-PST1 — pistol;
  - OC-SNP1 — sniper rifle;
  - OC-SG1 — shotgun with multi-pellet hitscan;
  - OC-LMG1 — LMG.
- Ammo types and ammo boxes.
- Replicated attachment state with functional prototype modifiers.
- Attachment slots: Optic, Muzzle, Underbarrel, Magazine, Stock.
- Prototype attachment IDs:
  - RedDot: improves ADS spread;
  - Suppressor: small damage trade-off; audio behavior comes in S15;
  - VerticalGrip: reduces recoil;
  - ExtendedMag: increases effective magazine capacity;
  - LightStock: reduces recoil.
- `UOCWeaponDefinition : UPrimaryDataAsset` for future editor-authored weapon tuning.
- C++ fallback weapon presets keep the source-only project runnable without custom `.uasset` files.
- HUD now shows active weapon name, ammo, fire mode, attachments, both inventory slots, and nearby interaction prompt.
- Test arena seeds all weapon prototypes plus ammo boxes.

## Server authority
Client input requests interactions, but the server:
1. finds nearby world pickups/ammo boxes;
2. validates distance;
3. changes inventory pointers;
4. changes ammo;
5. changes attachments;
6. applies weapon damage.

The client never sends an arbitrary weapon pointer to the server for pickup.

## Key bindings after S04
- WASD — movement
- Mouse — look
- LMB — fire
- RMB — ADS
- R — reload
- B — AUTO/SEMI where supported
- Shift — sprint
- Ctrl — crouch
- Space — jump
- E — interact / pick up / ammo box
- G — drop active weapon
- 1 — primary
- 2 — secondary
- TAB — scoreboard

## Deliberately deferred
- Actual weapon meshes/animations/sounds.
- Named real-world weapon catalog and balance pass.
- Optic view models/scopes.
- Underbarrel grenade launcher behavior.
- Inventory UI/loadout screen.
- Attachment pickup/customization UI.
- Per-bone/headshot damage.
- Weapon sway/suppression.
- Projectile ballistics for weapons that need them.

These belong to later combat/art/UI sessions. S04 is the reusable network weapon foundation.
