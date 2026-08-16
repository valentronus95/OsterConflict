# S04 weapon architecture

## Layers

### `FOCWeaponTuning`
Serializable data structure containing gameplay tuning: damage, range, RPM, spread, recoil, magazine/reserve limits, reload duration, ammo type, preferred inventory slot and pellets per shot.

### `UOCWeaponDefinition`
`UPrimaryDataAsset` type for editor-authored tuning. This is the intended long-term authoring path once the project is opened in Unreal Editor.

### C++ fallback profiles
`OCWeaponVariants.cpp` contains source-only prototype defaults. They prevent the project from depending on binary `.uasset` content during these early hand-off sessions.

### `AOCWeaponBase`
Replicated weapon actor. Owns ammo, reload state, fire mode, pickup state and attachment state. Damage and ammo mutations are server-only.

### `AOCCharacter`
Owns replicated Primary/Secondary pointers and the CurrentWeapon pointer. The server resolves interactions by searching nearby replicated actors; clients request the action but do not nominate arbitrary targets.

### `AOCAmmoBox`
Server-consumed world actor. It asks the character to distribute a finite amount of ammo across compatible inventory weapons.

## Attachment modifier rules in S04
These are deliberately simple placeholders, not final balance values.

- RedDot: ADS spread x0.88.
- VerticalGrip: recoil x0.82.
- LightStock: recoil x0.90.
- Suppressor: damage x0.95; sound suppression is deferred to S15.
- ExtendedMag: magazine capacity +max(5, base magazine / 3).

The attachment state itself already replicates, so later meshes/UI/audio can bind to the same IDs without changing the inventory protocol.
