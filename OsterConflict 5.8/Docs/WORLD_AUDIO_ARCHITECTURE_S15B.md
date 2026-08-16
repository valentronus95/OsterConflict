# World Audio Architecture — S15B

## Routing

`Gameplay event on server -> semantic EOCWorldAudioEvent -> NetMulticast -> local variant selection -> user bus volume -> spatial playback`.

Ambient life is different: the server-spawned `AOCAmbientAudioZone` actor/config replicates so every client knows where the zone is, but the bird, wind, yard-animal and distant-dog one-shots are chosen and played locally. The dedicated server does not replicate every decorative ambient event.

## User buses

- Master
- Weapons
- Vehicles
- Characters
- World SFX
- Ambience
- Music
- UI
- Voice Chat
- Dialogue

Every bus has a 0–100% slider backend. Major categories also have an enable checkbox. Music additionally has `Menu Music` on/off.

## World interactions

Variant banks are required, not one sample repeated forever:

- Door open: 4–8 per material/door family.
- Door close: 4–8.
- Gate open/close: 3–6 each.
- Light switch: 3–5 mechanical clicks; electrical hum is separate if needed.
- Window break: 5–10.
- Wood/metal/masonry destruction: 5–12 per material family.
- Generic pickup/use/confirm: 3–6 subtle variants.

Final Content can split these further by material, mass and condition: old wooden door, cheap hollow door, metal gate, stiff hinge, damaged mechanism. The source API remains semantic instead of hard-coding filenames.

## Natural ambience

Ambient layers are deliberately sparse:

- continuous bed: quiet wind/room tone/neighbourhood tone;
- one-shots: birds, leaves, yard animals, distant dogs, distant vehicle pass, water;
- time/zone context controls which pools are available;
- random intervals and slight variation reduce repetition;
- concurrency/culling budgets prevent dozens of simultaneous decorative emitters.

## Priorities

Priority order in a combat mix:

1. Immediate danger / player feedback.
2. Nearby footsteps and weapon handling.
3. Weapons / explosions / vehicles.
4. Squad/team signalling and critical UI.
5. Interaction feedback.
6. General ambience.
7. Menu/decorative music outside active gameplay.

Music should duck or stop during gameplay-critical states depending on mode; Sandbox may optionally allow it, default off during active firefights.
