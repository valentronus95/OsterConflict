# Animation / Injury / Blood / Dismemberment specification

Implementation status: S14B source framework implemented; final art assets remain pending. This document is the code-facing summary of MASTER-TЗ v2.2.

## Authoritative state
- Server owns damage, body zone result, LifeState, killer attribution and canonical gore outcome.
- Clients render hit reactions, Niagara blood, decals and most detached-part physics.
- Do not replicate droplets, every decal, or every ragdoll bone as independent gameplay data.

## Body zones
Minimum tags: HeadNeck, Torso, Pelvis, ArmL, ArmR, LegL, LegR.
A hit event carries zone + impact location + direction/normal + damage profile + applied damage + fatal flag + deterministic visual seed.

## Blood severity
- Light: small impact mist + small decal.
- Medium: stronger directional spray + surface decal + local character blood mask.
- Heavy: larger spray / limited extra decals / stronger wound material state.
- Extreme: fatal high-severity/explosive presentation and eligibility for authored dismemberment.

Severity is selected from body zone, damage profile/weapon class, applied damage and fatality. It must never be a fixed effect shared by every weapon.

## Animation layers
First person: equip, idle, sway, fire, ADS, tactical/empty reload, dry fire, grenade/gadget, interaction.
Third person: locomotion, crouch, sprint, aim offset, reload, grenade, interact, vehicle enter/exit, gunner, downed/crawl/revive, hit reactions and death.
Light hits are additive/brief; severe hits may use stronger reactions with anti-stunlock limits. Explosive deaths can skip canned death animation and enter ragdoll immediately.

## Ragdoll and dismemberment
Normal small-arms deaths keep a whole-body ragdoll. Dismemberment is authored and gated by eligible high-severity damage profiles.
Suggested outcome levels: None, SingleLimb, MultiPart, Catastrophic.
Suggested authored break zones for alpha: left/right arm and left/right leg families. Detached pieces use simplified collision, cannot block objectives/doors, sleep quickly and are cleaned before the main corpse when budget requires it.

## Cleanup / scalability
- Gore setting: Off / Reduced / Full; server may cap maximum level.
- Prototype corpse lifetime: 30 s with a 20-corpse hard cap; later art/performance tuning may raise this toward 45–90 s on capable servers.
- Decal and particle pools with per-area and global caps, distance culling and fade.
- Detached-part lifetime lower than corpse lifetime.
- Late join needs current corpse/persistent gore state only, not historical transient sprays.
- Performance degradation removes cosmetic layers first and never changes gameplay damage.
