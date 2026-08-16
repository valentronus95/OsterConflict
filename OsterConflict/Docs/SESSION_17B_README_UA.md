# S17B — Full Settings / Remapping / Persistence

## Статус
Source milestone. Додано багатосторінкове runtime UMG меню Settings поверх S17A frontend без обов'язкових Widget Blueprint `.uasset`.

## Що реалізовано

### Graphics
- Resolution + поточна нестандартна роздільна здатність.
- Fullscreen / Borderless / Windowed.
- Resolution scale 50–100%.
- Frame limit: 30/60/90/120/144/165/240/Unlimited + поточний нестандартний ліміт.
- VSync та Dynamic Resolution.
- Overall preset Low/Medium/High/Epic/Cinematic/Custom.
- Окремі View Distance, Shadows, Textures, Effects, Foliage, Post Process, Anti-Aliasing, Shading, Global Illumination, Reflections, Landscape.
- Engine video/scalability backend: `UGameUserSettings`.

### Audio
- Master / Weapons / Vehicles / Characters / World SFX / Ambience / Music / UI / Voice Chat / Dialogue.
- Для кожної категорії: slider 0–100% + окремий checkbox enable/mute.
- Menu Music toggle.
- Dynamic Range: Night / Standard / High Dynamic Range.
- Output: Stereo Speakers / Headphones / Spatial Headphones.
- Використовується S15B `UOCAudioUserSettings`.

### Controls
- Mouse sensitivity 0.10–4.00.
- ADS sensitivity multiplier 0.25–1.50.
- Invert Y.
- Runtime primary-key rebinding для Move Forward/Back/Left/Right, Jump, Sprint, Crouch, Fire, Aim, Reload, Interact, Grenade, Scoreboard, Chat.
- Якщо нова клавіша вже зайнята, primary bindings міняються місцями замість створення конфлікту.
- Character і PlayerController перебудовують runtime `UInputMappingContext` після Apply/Save.

### Interface / Accessibility
- FOV 75–120; застосовується до локальної FPS-камери.
- HUD scale 75–125% збережений як contract для фінальної UMG gameplay-HUD міграції; legacy Canvas HUD поки не масштабується цілком.
- Show FPS / Ping / Crosshair / Hitmarker.
- Gore Off / Reduced / Full -> `oc.GoreLevel` (лише presentation).
- Subtitles preference.
- Reduce flashes: зменшує локальну flash presentation.
- Camera shake 0–100%.
- Color vision mode: Off / Deuteranopia / Protanopia / Tritanopia збережений як accessibility profile; фінальний color-transform/post-process потребує production renderer/content pass.

### Apply semantics
- APPLY: застосовує та зберігає, меню лишається відкритим.
- SAVE & BACK: застосовує, зберігає, закриває Settings.
- CANCEL / Escape: перечитує останні збережені Engine/Audio/Player settings і відкидає незастосовані зміни.
- RESET DEFAULTS: лише ставить defaults у staging; зберігає їх тільки після Apply/Save.

## Persistence
- Engine graphics: `UGameUserSettings` / стандартний GameUserSettings persistence.
- Oster controls/HUD/accessibility: `UOCPlayerUserSettings`, `Config=GameUserSettings`.
- Audio: `UOCAudioUserSettings` із S15B.

## Важливі обмеження
- Це source-only UI та backend. Фінальні UI art, іконки, локалізація, gamepad/controller navigation polish і повний color-vision renderer transform не є готовими production assets.
- `HUDScale` зберігається і відображається, але повне масштабування старого Canvas HUD переноситься в фінальну UMG gameplay-HUD міграцію/QA.
- Реальна компіляція та runtime-тест UE 5.8 у цьому середовищі недоступні; milestone перевіряється статично та regression-verifier-ами.
