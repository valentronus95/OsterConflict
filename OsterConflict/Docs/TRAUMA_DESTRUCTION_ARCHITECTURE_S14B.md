# S14B architecture — trauma, corpse and destruction

## 1. Damage pipeline
1. Shooter/turret/projectile performs server-authoritative hit/damage.
2. Existing HealthComponent applies the actual gameplay damage and LifeState transition.
3. Only after damage is applied, CombatVisualComponent receives actual damage + hit data + final fatal flag.
4. Server creates one compact replicated trauma event.
5. Clients render local visual layers according to local gore setting and performance budget.

This order prevents visual severity from becoming the gameplay source of truth.

## 2. Blood selection
Inputs:
- actual applied damage;
- hit body zone;
- weapon class;
- explosive/non-explosive damage family;
- fatal/non-fatal result.

Outputs:
- None / Light / Medium / Heavy / Extreme.

Final Niagara, decal and wound-material assets will be selected from this severity rather than hard-coded by weapon name.

## 3. Dismemberment
Server outputs a four-bit authored mask for left arm, right arm, left leg and right leg. Eligibility requires fatal Extreme trauma plus an allowed high-energy profile. Routine rifle/SMG/pistol hits do not automatically detach limbs.

Clients may suppress detached parts when `oc.GoreLevel=0/1`, but the underlying death/gameplay state stays unchanged.

## 4. Ragdoll
When a final character skeletal mesh and PhysicsAsset are present, the corpse mesh switches to the Ragdoll collision profile, enables body simulation and receives an impulse from the replicated incoming direction.

The source-only prototype tolerates a missing character mesh/PhysicsAsset. That is intentional so code milestones remain launchable before art import.

## 5. Corpse budget
- Respawn delay and corpse lifetime are separate.
- Prototype corpse lifetime: 30 s.
- Prototype hard cap: 20 corpses.
- Oldest corpse is removed first when the cap is exceeded.
- Detached cosmetic chunks have a shorter lifetime (12 s default).

## 6. Environmental destruction
`AOCDestructibleProp` has authoritative durability and one replicated destroyed bit. When destroyed, the intact collider disappears on every client, while each client spawns its own short-lived debris chunks. No per-chunk network physics is required.

## 7. Impact surface routing
Current source categories:
- Flesh
- Glass
- Wood
- Metal
- Masonry
- Dirt
- Default

The weapon sends one representative impact event per shot for surface-specific particles, decals and audio. Final Physical Materials can replace class/tag heuristics later.
