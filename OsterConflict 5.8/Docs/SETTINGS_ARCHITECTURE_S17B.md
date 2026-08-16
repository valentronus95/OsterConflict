# S17B Settings Architecture

## 1. Розподіл відповідальності

### `UGameUserSettings`
Є єдиним backend для renderer/display/scalability параметрів:
- resolution;
- window mode;
- VSync;
- dynamic resolution;
- frame limit;
- resolution scale;
- scalability buckets.

S17B не дублює ці дані у власному config-об'єкті.

### `UOCAudioUserSettings`
Зберігає S15B audio mixer preferences: bus percent + bus enabled, menu music, dynamic range та output mode.

### `UOCPlayerUserSettings`
Зберігає Oster-specific local preferences:
- mouse/ADS sensitivity;
- invert Y;
- FOV;
- HUD visibility/profile;
- gore/accessibility presentation;
- primary key map.

## 2. Runtime input remapping

`UOCPlayerUserSettings` зберігає `FName` ключа для primary binding. Після Apply:
1. Character викликає `UnmapAllKeysFromAction` для gameplay actions.
2. Додає новий `MapKey` у runtime Character Mapping Context.
3. PlayerController окремо remap-ить Scoreboard та Chat у Controller Mapping Context.
4. Контексти повторно додаються через Enhanced Input local player subsystem.

Fixed developer/system bindings F10/F8/F2/F3/F4/Escape не входять у S17B primary remap, щоб test/admin recovery не можна було випадково втратити.

## 3. Staging / Save / Cancel

UI widgets є staging-представленням. Key rebind тимчасово змінює mutable config default object, але не пише файл до Apply/Save. Cancel робить `ReloadConfig()`/`LoadSettings(true)` і повертає останні збережені значення.

`ResetPlayerDefaults` і `ResetAudioDefaults` навмисно не роблять `SaveConfig()` самі. Це не дозволяє RESET DEFAULTS випадково перезаписати профіль без підтвердження.

## 4. Security / networking

Усі параметри S17B локальні. Вони не змінюють server-authoritative health, damage, tickets, ammo, bot difficulty чи інші gameplay values. Gore, FOV, HUD, audio та input не дають клієнту права змінювати серверний результат бою.

## 5. Accessibility contract

- `GoreLevel`: діє лише на клієнтське presentation.
- `ReduceFlashes`: зменшує intensity flash presentation, але серверний stun/gameplay state лишається незмінним.
- `CameraShakeScale`: масштабує local camera shake.
- `ColorVisionMode`: persistent profile для фінального viewport/post-process color transform; production transform не маскується під готовий у source-only milestone.
- `Subtitles`: persistent preference; використовується майбутнім dialogue/voice subtitle layer.

## 6. Performance
Settings UI не тикає renderer/game config щокадру. Значення читаються при відкритті, labels оновлюються для поточних slider state, а Apply викликається лише за явною дією користувача.
