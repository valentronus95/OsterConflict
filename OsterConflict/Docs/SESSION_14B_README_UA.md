# S14B — Animation / Blood / Ragdoll / Dismemberment / Destruction

Статус: source milestone.

## Реалізовано
- `UOCCombatVisualComponent`: окремий replicated visual-trauma layer на кожному персонажі.
- Сервер формує компактний `FOCReplicatedTraumaEvent` після фактичного gameplay damage.
- Зони: Head/Neck, Torso, Pelvis, L/R Arm, L/R Leg; є fallback оцінка за локальною висотою удару, якщо skeletal bone name відсутній.
- Blood severity: None / Light / Medium / Heavy / Extreme.
- Severity залежить від applied damage, body zone, weapon class, explosive profile і fatality.
- Hit-reaction event містить location + direction + zone + damage + weapon class + fatal flag.
- `BP_PlayTraumaEvent` — art hook для AnimBP / Niagara / decals / wound material.
- `BP_PlayDeathPresentation` — art hook для death montage / ragdoll / blood pool presentation.
- `oc.GoreLevel 0|1|2`: local Off / Reduced / Full presentation without changing gameplay result.
- Whole-body ragdoll path when a real skeletal mesh + physics asset exists.
- Server-authored dismemberment mask. Prototype authored families: left/right arms and legs.
- Detached chunks are local cosmetic physics, short-lived and not gameplay blockers.
- Corpse lifetime decoupled from respawn: prototype corpse ≈30 s while respawn remains ≈3 s.
- `MaxPersistentCorpses=20` caps old bodies; oldest corpse is removed first.
- `AOCDestructibleProp`: replicated durability/destroyed state plus local physics chunks.
- Source-only test lane contains Wood / Metal / Masonry props.
- Weapon representative impacts now classify Flesh / Glass / Wood / Metal / Masonry / Dirt and expose `BP_PlayImpactFX`.
- Fragmentation grenade creates server-authored radial trauma presentation for exposed characters.

## Що свідомо ще не є фінальним артом
Source-only build не містить фінальних skeletal meshes, animation montages, Niagara systems, blood materials або decal materials. Тому в Development build існують debug-представлення, а production assets підключаються через Blueprint events без зміни damage/network framework.

## Мережева модель
Gameplay damage, fatality, body zone/severity result і dismemberment mask визначаються сервером. Клієнтські blood sprays, decals, локальні chunks та ragdoll presentation не є authoritative gameplay state.
