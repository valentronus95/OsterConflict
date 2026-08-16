# S12 architecture — ordnance, engineer, Sandbox

## Authority boundary

Клієнт передає тільки намір: throw, select, deploy, interact, admin action. Сервер перевіряє стан персонажа/роль/кількість боєприпасів/режим і створює gameplay actors.

### Grenade path
`input -> ServerThrowSelectedGrenade -> replicated AOCGrenadeProjectile -> server detonation -> authoritative gameplay result`

- Frag applies radial damage on server.
- Smoke spawns replicated `AOCSmokeCloud`; opacity/visual detail can later move to Niagara on clients.
- Flash selects affected characters on server using radius, visibility trace and facing, then sends owner-only flash presentation.

### Trap path
`Engineer input -> ServerDeploySelectedTrap -> replicated AOCDeployableTrap -> arm delay -> overlap -> server effect`

Fifteen presets are intentionally abstract gameplay categories. They are not descriptions of real devices.

### Anti-armour path
`AOCAntiArmorLauncher -> server projectile -> impact -> UOCAntiArmorDamageType`

The BTR damage filter from S11 rejects ordinary ballistic damage and accepts the dedicated anti-armour class.

## Sandbox architecture

`AOCGameMode::InitGame` reads `Mode=Sandbox` from the travel URL.

- `AOCGameState::GameplayMode` replicates mode identity to clients.
- `AOCGameMode` suppresses ticket bleed and ticket-based round end in Sandbox.
- `AOCPlayerController` owns local F10 panel input and sends validated Server RPC admin actions.
- In S12 development builds `IsSandboxAdmin()` is intentionally equivalent to being in Sandbox mode. This is not the final security model.

Admin actions are executed only on authority. Object reset iterates server-side doors, gates and lights and calls their reset functions.

## Interaction extension

S08 `AOCInteractableActor` now covers:
- door;
- gate;
- light;
- deployable trap/disarm.

This keeps `E` extensible for future switches, mounted weapons, generators, crates and engineering objects.
