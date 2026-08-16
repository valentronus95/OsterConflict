# Oster Conflict — поділ розробки на короткі сесії

Цей файл фіксує порядок реалізації. Одна сесія = один невеликий блок, який можна окремо відкрити, перевірити й прийняти.

## S01 — Project + Network FPS Foundation [ВИКОНАНО]
- UE 5.8 C++ project skeleton.
- Game / Editor / Client / Server targets.
- First-person camera.
- WASD, mouse look, jump, crouch, sprint.
- Replicated CharacterMovement foundation.
- Server-authoritative hitscan damage.
- Replicated health component.
- Health regeneration without permanent HP bar.
- Basic death and respawn.
- One placeholder rifle, ammo and reload.
- Minimal crosshair / ammo HUD.
- Runtime test arena and damage targets, without custom .uasset content.

## S02 — Input + combat feel [ВИКОНАНО]
- Enhanced Input migrated to runtime-created C++ Input Actions/Mapping Context, without required custom `.uasset` files.
- ADS with smooth camera FOV.
- Automatic/semi-auto fire modes with B toggle.
- Server-owned automatic fire cadence.
- Hip/ADS/movement spread.
- Local recoil and recoil recovery.
- Replicated reload state and interruption rules.
- Optional camera-shake class hook.
- Hit marker / fatal hit marker.
- Directional damage indicator.

## S03 — Multiplayer test harness [ВИКОНАНО]
- Direct IP:port prototype connect/disconnect flow.
- Existing runtime test arena reused for network testing.
- Documented 2–4 player PIE/dedicated-server test profile.
- PlayerState: nickname, kills, deaths, score, ping.
- TAB scoreboard first version.
- Join/leave handling.
- Full graphical lobby/server browser remains S17.

## S04 — Weapon framework [ВИКОНАНО]
- Data-driven weapon definition.
- Primary/secondary slots.
- Pick up / drop / swap.
- 5 prototype weapon classes by behavior.
- Ammo boxes.
- Attachments framework.

## S05 — Health / downed / medic [ВИКОНАНО]
- Explicit Alive / Downed / Dead replicated life-state model.
- 60-second server bleed-out timer with synchronized client countdown.
- Prototype wounded crawling: low camera + crouched presentation + 95 cm/s movement.
- Hold Space 2 seconds to give up.
- Hold E 3 seconds to revive.
- Continuous server distance/line-of-sight/state validation.
- 35 HP revive followed by normal health regeneration.
- Revive score/stat tracking and TAB `R` column.
- Medic capability hook prepared for S06 team/class restrictions.

## S06 — Game-mode framework [ВИКОНАНО]
- Two balanced teams per match with replicated TeamId.
- Role foundation with Medic-only friendly revive.
- Friendly fire disabled by default on the authority.
- Base + captured-objective forward spawn zones.
- Three replicated capture zones with neutralize/capture/contested logic.
- Conquest tickets: death cost + objective majority bleed.
- Replicated match phase and winner.
- Automatic round reset/restart.
- Team-based TAB scoreboard and objective/ticket HUD.

## S07 — Остер Greybox, Sector A [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Reference pass із перевіркою назви/адміністративного статусу.
- ~1.8 x 1.8 км source-only runtime greybox.
- Музей/садиба Солонини як локальний map anchor.
- Центральний стадіон біля музейного сектору.
- Coarse anchors парку і коледжу для правильного масштабу.
- Перша дорожня сітка, тротуари, low-rise blocks, дерева/паркани.
- A/B/C Conquest та spawn zones перенесені на city-scale layout.
- Firing lane залишений лише для regression tests.

## S08 — Остер Greybox, Sector B [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Розширена вул. Соломії Крушельницької.
- Щільніший приватний сектор, двори, проходи та тилові алеї.
- Перший повністю прохідний `AOCEnterableHouse`.
- Повторно використовуваний `AOCInteractableActor` framework.
- Replicated hinged door.
- Server-authoritative breakable windows + cosmetic shard burst.

## S09 — Остер: парк + коледж + reference fidelity pass [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- City park.
- College/technical school and adjacent territory.
- Connect A/B/C into first playable district.

## S10 — Vehicles core [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Server-authoritative source-only rigid-body vehicle foundation.
- Four-point raycast suspension and traction/steering forces.
- Civilian wagon / sedan / hatchback variants.
- Driver seat with authority-side Pawn possession.
- First-person cockpit greybox: wheel, dashboard, windshield.
- Third-person vehicle camera.
- Hold RMB free-look and camera recenter.
- Vehicle health, collision damage and replicated damage stages.
- Persistent wreck + independent vehicle respawn timer.
- Eight civilian vehicle spawn points distributed over the Oster road network.

## S11 — Armed vehicles [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Pickup gun truck: independent Driver + Gunner.
- BTR/APC prototype: independent Driver + Gunner; driver cannot operate the turret.
- Minimum two active crew members are required for mobile vehicle fire: gunner fire is disabled without a driver.
- Same-team crew validation and occupied-vehicle team ownership.
- Replicated turret yaw/pitch, magazine/reserve ammo and reload state.
- Server-authoritative turret cadence, hit trace and damage.
- BTR hull rejects ballistic/vehicle-gun/collision damage; dedicated anti-armour damage class is prepared for S12 RPG/launcher weapons.
- Four combat vehicle respawners added: two gun trucks and two BTR-class vehicles.

## S12 — Grenades + engineering + Sandbox/Test Range [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Frag / smoke / flash-stun grenades with server-authoritative gameplay results.
- 15 selectable game-only trap presets; Engineer deploy/disarm rules.
- Engineer repair interaction for damaged vehicles.
- Anti-armour launcher + projectile wired to S11 BTR armour filtering.
- Sandbox/Test Range gameplay mode (`?Mode=Sandbox`) without ticket bleed or ticket victory.
- F10 test admin panel: all implemented weapons, ammo/restore/god mode, vehicle spawn, interaction reset, landmark teleport.
- Replicated interior light and yard gate added to the first enterable house.

## S13 — AI bots [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Bot controller.
- Difficulty presets.
- Patrol, attack, defend, seek cover.
- Pre-match bot configuration.
- Basic team objectives.
- Medic revive, door/gate use, first-pass vehicle driving and Sandbox bot spawn/clear.

## S14 — Destruction + animation + damage visuals [S14B SOURCE MILESTONE ВИКОНАНО]
- Upgrade S08 breakable windows to final-quality destruction/VFX.
- Doors/fences/light structures.
- Vehicle visual damage states.
- Debris budget.
- Bullet decals.
- Hit reactions by direction/body zone and full first-/third-person animation event framework.
- Blood severity Light/Medium/Heavy/Extreme, decals/material masks/pools and Gore Off/Reduced/Full.
- Ragdoll + authored dismemberment profiles with server canonical outcome and client cosmetic physics.
- Explosion VFX hooks.

## S15 — Audio framework
- Weapon event layers.
- Reload/mechanical sounds.
- Impacts and ricochets.
- Explosions.
- Vehicles.
- Footsteps and ambience.
- Licensed/original/generated assets only.

## S16 — Art pass + large-world foundation
- World Partition map.
- HLOD strategy.
- Modular Oster building kit.
- Vegetation and yards.
- Lighting/weather baseline.

## S17 — UI/settings [S17A + S17B SOURCE MILESTONES ВИКОНАНО]
- Main menu / direct connect / deployment / chat / scoreboard / Sandbox admin: S17A.
- Full Graphics / Audio / Controls / Interface / Accessibility pages: S17B.
- Engine graphics via `UGameUserSettings`; audio via S15B settings; local controls/HUD/accessibility via `UOCPlayerUserSettings`.
- Runtime primary-key remapping, FOV/sensitivity, HUD visibility, FPS toggle, gore/reduced-flash/camera-shake preferences.
- Apply / Save & Back / Cancel / staged Reset Defaults and persistence.
- Remaining polish: final UI art/localization/gamepad navigation, full UMG gameplay HUD scale and production color-vision transform.

## S18 — Release candidate
### S18A — Optimization / QA / release preparation [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Runtime server performance profiles (`LowCPU / Balanced / Quality`).
- AI think/corpse budgets without changing match rules.
- Event-driven door tick, client-only ambient ticking, reduced update rates for static/slow replicated actors.
- `PerfReport` authoritative server snapshot.
- 8/16 population scripts, Unreal Insights/Networking Insights launch scripts.
- Performance budget, test matrix, packaging readiness and release-tree audit.
- Full source regression after ZIP extraction.

### S18B — First real build / RC gate [BUILD AUTOMATION SOURCE MILESTONE ВИКОНАНО]
- Build pipeline під UE 5.8 source build доданий; фактичний C++/UHT compile ще має бути виконаний на build-машині.
- Project-owned `.umap` автоматично створюється Unreal Editor Python перед cook.
- C++ Automation smoke tests `OsterConflict.Release.*`.
- UAT BuildCookRun scripts для Dedicated Server + Windows Client.
- Post-build EXE/container/SHA-256 audit і packaged server + 2-client smoke harness.
- 8/16 packaged load/soak, actual performance baselines і RC bug-fix pass залишаються фактичним release gate після першої компіляції.



## S14A - Lobby / hybrid population / squads / chat / household dressing
Status: implemented source milestone. Human-priority bot filler, squads/orders, chat backend, deployment HUD and non-interactive household props.

## S14B - Animation / gore / destruction
Status: implemented source milestone. Server-authored trauma events, body zones, blood severity, gore levels, ragdoll hooks, authored limb mask, corpse budget, impact routing and destructible props are now in code. Final skeletal/animation/Niagara/decal assets remain an art-content task.


## S15A — Weapon Audio
Near/indoor/suppressed/distant weapon reports, mechanics, reload, bullet cracks, surface impacts, data-driven audio profiles.

## S15B — World / Vehicle / Character Audio
Engines, tires, BTR, doors, glass, footsteps, equipment, injury/voice, ambience and environmental mix.

## S15B — World / Vehicle / Character / Menu Audio — DONE (source milestone)
Interaction variants, ambient zones, vehicle/character presentation, menu music/UI hooks and persistent category audio settings backend.

## S16A — Reference-driven Oster map / georeference [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- 2.4 × 2.4 km source-only world proxy.
- WGS84 -> local UE centimeter reference layer with Museum/Solonyna estate as origin.
- Official Oster General Plan used for macro street/water/quarter topology.
- Corrected Central City Park anchor; separate north culture-house park reference retained.
- Expanded irregular road network, Desna/Oster hydrography proxies and wider residential districts.
- Museum/stadium/college fidelity pass and explicit A/B/C confidence model.
- No third-party photo/video/PDF assets bundled into game archive.

## S16B — Private-sector fences / vegetation / ground-cover palette [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Tall opaque street-facing private fences split into Wood / Metal / LightSheet families.
- Wood is the dominant private-sector proxy family; public landmark fences stay separate.
- Soviet-era urban planting proxies: poplar, mixed broadleaf, birch and pine.
- Grass zoning split into mown civic, rough roadside/private and wetland/floodplain families.
- Private-yard small broadleaf proxies reserve space for final fruit-tree content.
- Final species placement remains reference-driven; current geometry is source-only proxy content.


## S16C — Character / faction / animation framework [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Four data-driven visual faction archetypes; two selected per match.
- Replicated faction + deterministic appearance seed in PlayerState for humans and bots.
- CharacterVisualProfile DataAsset contract for TP body, FPS arms, gear and animation classes/montages.
- CharacterVisualComponent applies profiles and provides source-only fallback proxy bodies/arms.
- CharacterAnimInstance exports locomotion/combat/life/vehicle/aim parameters to future Animation Blueprints.
- Cosmetic network hooks for Fire / Reload / Revive / Downed / Revived / Death.
- Production skeletal meshes, Animation Blueprints, IK rigs and animation assets remain a content task.

## S17A — Rich UI / Lobby / Deployment / Chat [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Runtime C++ UMG root; no mandatory Widget Blueprint assets.
- Frontend direct-connect screen with username/IP:port.
- Deployment UI: team, faction, role, squad, Human/Bot population, Base/A/B/C spawn selection.
- Server-validated team switching and selected-spawn fallback logic.
- Rich TAB scoreboard and T-key ALL/TEAM/SQUAD chat input.
- Rich Sandbox admin panel using existing authoritative admin backend.
- Canvas HUD retained as fallback.
- S17B remains: full settings/remapping/persistence/final UI art/accessibility.


## S17B — Full Settings / Remapping / Persistence [ВИКОНАНО В ЦЬОМУ АРХІВІ]
- Runtime C++ UMG Settings with Graphics / Audio / Controls / Interface / Accessibility tabs.
- `UGameUserSettings` display/scalability integration.
- S15B audio mixer sliders + enable checkboxes.
- Persistent mouse/ADS sensitivity, invert Y, FOV, HUD toggles, gore/accessibility profile.
- Primary key rebinding with duplicate-key swap; Character + Controller Enhanced Input contexts rebuilt on Apply.
- Apply / Save & Back / Cancel / staged Defaults.
- Known production dependencies documented instead of falsely marked complete.

### S18C — Build readiness static preflight [SOURCE MILESTONE ВИКОНАНО]
- UHT/RPC/target/module/script preflight.
- Windows UE 5.8 + MSVC toolchain preflight.
- Build-log classifier for the first real compile/cook/package failure.
- Does not replace the real UE compile gate.
