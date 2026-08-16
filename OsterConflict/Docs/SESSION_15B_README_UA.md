# S15B — World / Vehicle / Character / Menu Audio

S15B розширює S15A за межі зброї. Це source-only audio framework: він визначає події, варіанти, категорії гучності, локальну просторову логіку та точки підключення майбутніх SoundWave/SoundCue/MetaSound assets, але не містить вкрадених або випадкових сторонніх WAV.

## Реалізовано

- `UOCAudioUserSettings`: Master + Weapons + Vehicles + Characters + World SFX + Ambience + Music + UI + Voice Chat + Dialogue, 0–100%, окремі enable/disable toggles, Menu Music toggle, Dynamic Range, Output Mode.
- `UOCWorldAudioComponent` + `UOCWorldAudioProfile`: semantic events для дверей, воріт, світла, скла, руйнування та малих/великих вибухів; один server event -> локальний variant на кожному клієнті.
- Двері, ворота, світло, breakable window, destructible props і frag/flash grenade вже мають world-audio hooks.
- `AOCAmbientAudioZone`: локальні background beds + випадкові рідкі one-shots (птахи, вітер/листя, дворові тварини, далекі собаки, трафік, вода). Ambient не реплікується як gameplay.
- 4 source-only ambient zones закладені в центральному секторі: музей, парк, коледж, приватний сектор.
- `UOCCharacterAudioComponent/Profile`: fallback footsteps за surface, gear rustle, pain/downed/revive/death pools. У S16C footsteps мають перейти на AnimNotify/foot contacts.
- `UOCVehicleAudioComponent/Profile`: exterior/interior engine loops, pitch за load/speed, tire roll, skid, start/stop і collision variants.
- `UOCMenuAudioSubsystem/Profile`: background menu music + короткі UI hover/click/back/confirm/error/panel sounds.
- S15A weapon audio тепер також враховує user Weapons Volume/checkbox.

## Принцип «не переборщити»

1. Не кожна дія повинна мати гучний feedback.
2. Для повторюваних подій використовувати variant pools, невеликий pitch/volume jitter і cooldown/concurrency.
3. Combat-critical audio (постріли, кроки поруч, вибух, lock/warning, squad ping) має пріоритет над ambience/UI.
4. Одночасно в одному ambient zone: один bed + небагато one-shots; природний фон не повинен звучати як зоопарк із відкритими всіма клітками.
5. UI hover — дуже тихий і короткий; click/confirm/error виразніші, але без «кассового апарата» на кожному русі курсора.
6. Menu music за замовчуванням нижче SFX; окремий checkbox дозволяє повністю вимкнути її без зниження інших звуків.

## Що потребує Content/Editor pass

- реальні/ліцензовані audio assets;
- SoundClass tree + SoundMix/Submix routing;
- attenuation, occlusion, reverb sends;
- MetaSound graphs для двигунів, ambience, вибухів та distant tails;
- physical materials для точного footstep/surface routing;
- UMG Audio Settings screen у S17B.
