# S12 — Grenades + Engineering + Sandbox / Test Range

Статус: source milestone. Unreal Engine 5.8 toolchain у середовищі генерації відсутній, тому цей архів проходить структурну/regression-перевірку, але не видається за реально скомпільований UE build.

## Що додано

### 1. Гранати
- `F` — кинути вибраний тип гранати.
- `4` — перемкнути FRAG / SMOKE / FLASH.
- Frag: server-authoritative radial damage.
- Smoke: replicated smoke-cloud actor з обмеженим часом життя.
- Flash/stun: server-side distance + line-of-sight + facing evaluation; локальний HUD flash fade.
- Стартовий прототипний запас: 2 гранати кожного типу.

### 2. Інженер
- Роль Engineer може ремонтувати пошкоджену незнищену техніку через `E` біля машини.
- Engineer може розміновувати/прибирати deployable trap через `E`.
- `M` — встановити вибраний game-only trap preset.
- `N` — циклічно вибирати один із 15 абстрактних trap presets.
- Усі 15 preset-ів — суто ігрові класи. Проєкт не містить інструкцій з реального виготовлення мін або вибухових пристроїв.

### 3. Anti-armour launcher
- Додано окремий launcher weapon class.
- Магазин: 1 постріл; резерв прототипу: 4.
- Сервер створює projectile і визначає impact/damage.
- Projectile використовує `UOCAntiArmorDamageType`.
- BTR із S11 уже приймає саме цей anti-armour damage class, але ігнорує звичайний ballistic damage.

### 4. Sandbox / Test Range
Новий окремий gameplay mode, призначений для перегляду карти та тестування гри без нормального раунду Conquest.

Запуск через URL option:

`?Mode=Sandbox`

У Sandbox:
- немає ticket bleed;
- раунд не завершується через tickets;
- можна вільно пересуватися картою;
- можна користуватися зброєю, гранатами, технікою та інтерактивними об'єктами;
- `F10` відкриває тестову admin panel.

Admin panel S12:
1. Spawn all implemented weapons + ammo box.
2. Refill ammo.
3. Restore player health/state.
4. Spawn civilian vehicle.
5. Spawn gun truck.
6. Spawn BTR.
7. Toggle server-side Sandbox god mode.
8. Reset doors / gates / lights.
9. Teleport to Museum.
10. Teleport to Stadium.
11. Teleport to Park.
12. Teleport to College.

Керування панеллю: `F10`, `Up/Down`, `Enter`.

У S12 всі клієнти Sandbox-сесії отримують test-admin capability навмисно для локальної розробки. Перед release/network-hardening цей доступ має бути обмежений host/admin permission на сервері.

### 5. Взаємодія в будинку
Перший enterable house із S08 тепер додатково має:
- replicated interior light: `E LIGHT ON/OFF`;
- replicated yard gate: `E OPEN/CLOSE GATE`;
- admin reset повертає door/gate/light у початковий стан.

## Важливі межі S12
- Візуальні grenade/smoke/explosion assets залишаються source-only prototype; фінальні Niagara/VFX — S14.
- Повний звук гранат, вибухів, launchers та інженерних дій — S15.
- Engineer repair поки prototype action без фінальної ремонтної анімації, ресурсу та прогрес-бара.
- Частина 15 trap preset-ів у S12 має placeholder gameplay behavior і буде поглиблена AI/objective системами.
- Sandbox admin — dev/test інструмент, не production moderation system.
