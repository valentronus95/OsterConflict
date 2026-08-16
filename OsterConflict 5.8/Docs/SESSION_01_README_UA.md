# Oster Conflict — S01
## Project + Network FPS Foundation

### Що вже реалізовано

Це **вихідний Unreal Engine 5.8 C++ проєкт**, а не готовий фінальний `.exe`.

У S01 є:

- C++ модуль `OsterConflict`;
- Game / Editor / Client / Server build targets;
- first-person персонаж;
- WASD + mouse look;
- стрибок, присідання, спринт;
- стандартний `CharacterMovementComponent` як основа мережевого prediction/correction;
- сервер-авторитетний hitscan-постріл;
- базова серверна перевірка origin/direction пострілу;
- магазин 30 + резерв 120;
- перезарядка;
- replicated health component;
- 100 HP, затримка регенерації 5 с, регенерація 25 HP/с;
- постійної шкали HP немає;
- червона рамка пошкодження при втраті HP;
- смерть і респавн приблизно через 3 с;
- runtime тестова арена;
- 5 тестових мішеней;
- мінімальний crosshair + ammo HUD.

### Керування

| Дія | Клавіша |
|---|---|
| Рух | WASD |
| Огляд | Mouse |
| Стрибок | Space |
| Спринт | Left Shift |
| Присідання | Left Ctrl |
| Постріл | LMB |
| Перезарядка | R |

### Чому зараз немає красивої зброї/рук/міста

`.uasset`-контент Unreal створюється/імпортується самим Unreal Editor. У цій сесії навмисно зроблене **source-only ядро**, яке не залежить від сторонніх моделей. Замість зброї використовується технічний cube-placeholder, а арена створюється кодом під час запуску.

Це дозволяє спочатку перевірити найважливіше: чи компілюється проєкт, чи працює рух, постріл, damage, replication і respawn. Художній контент підключається поверх уже працюючого ядра, а не навпаки.

### Запуск у редакторі

1. Встановити Unreal Engine **5.8** і Visual Studio з C++ toolchain для Unreal.
2. Розпакувати папку `OsterConflict`.
3. Відкрити `OsterConflict.uproject`.
4. Якщо Unreal попросить зібрати C++ модулі — погодитися.
5. Якщо автоматична збірка не пройшла: Generate Visual Studio project files → відкрити `.sln` → `Development Editor | Win64` → Build.
6. Запустити Unreal Editor і натиснути Play.

Проєкт використовує `/Engine/Maps/Entry` і створює тестову арену runtime, тому власна `.umap` для S01 не потрібна.

### Перевірка 2 клієнтів у PIE

В Editor:

1. Play dropdown → Multiplayer Options.
2. Number of Players = 2.
3. Net Mode = Play As Listen Server або Play As Client з окремим server-процесом.
4. Запустити PIE.
5. Перевірити рух обох персонажів, шкоду, смерть і respawn.

S01 закладає dedicated-server target, але **офіційний dedicated-server build UE 5.8 потребує C++ project і source build Unreal Engine**. Сам Server target уже є; cook/package dedicated server робиться пізніше після появи власної карти та контенту.

### Відомі обмеження S01

- Ввід поки використовує прості config action/axis mappings, щоб проєкт був source-only; міграція на Enhanced Input запланована в S02.
- Немає анімацій рук/персонажа.
- Немає ADS.
- Немає automatic fire loop, recoil, spread, hitmarker.
- Немає lag compensation / server rewind.
- Спринт уже синхронізує стан із сервером, але повний custom movement prediction для sprint/crawl буде зроблено окремим networking pass.
- Debug-трасер пострілу показується лише не-Shipping збірках.
- Тестові мішені не респавняться.
- Тестова арена є лише технічним стендом і не входить до Oster map.

### Критерії приймання S01

S01 вважається прийнятою після фактичного запуску в UE 5.8, якщо:

- проєкт компілюється без C++ errors;
- гравець з'являється на тестовій арені;
- WASD/mouse/jump/crouch/sprint працюють;
- LMB витрачає патрон;
- R перезаряджає;
- влучання в мішень зменшує health;
- після достатньої кількості влучань мішень знищується;
- два клієнти бачать рух один одного;
- damage між клієнтами визначається сервером;
- після смерті гравець респавниться.
