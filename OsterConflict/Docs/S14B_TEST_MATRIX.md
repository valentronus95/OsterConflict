# S14B test matrix

1. Rifle torso hit -> trauma event replicates with Torso and non-zero blood severity.
2. Head/neck bone name -> HeadNeck classification and higher blood tier than equal torso hit.
3. Limb bone names -> correct Left/Right Arm/Leg zone.
4. Shotgun/sniper high-damage fatal limb hit -> eligible SingleLimb result only when Extreme.
5. Routine pistol/SMG death -> no automatic dismemberment.
6. High-energy explosive fatal -> MultiPart/Catastrophic can be selected by thresholds.
7. `oc.GoreLevel 0` -> gameplay death unchanged, local blood/chunk presentation suppressed.
8. `oc.GoreLevel 1` -> reduced local presentation, no detached chunks.
9. `oc.GoreLevel 2` -> full local presentation including eligible chunks.
10. Character with valid PhysicsAsset -> local ragdoll starts on death.
11. Character without PhysicsAsset -> death pipeline remains valid and does not require ragdoll asset.
12. Respawn occurs while old corpse can remain visible.
13. More than 20 corpses -> oldest corpse is cleaned first.
14. Wood destruction prop -> lower durability and local chunks after destruction.
15. Metal/masonry destruction props -> higher durability.
16. Destroyed prop collision disabled consistently on all clients.
17. Bullet impact routes Flesh/Glass/Wood/Metal/Masonry categories.
18. Fragmentation grenade -> exposed target receives radial trauma event; blocked target should not receive the visual event.
19. Late state replication -> destroyed prop remains destroyed for later client state.
20. Dedicated server -> does not spawn local gore/debris presentation from CombatVisualComponent.
