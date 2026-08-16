# S11 — озброєна техніка: Driver + Gunner

## Статус
S11 реалізує перший мережевий каркас бойової техніки поверх S10.

## Що додано

### Пікап із кулеметом
- `AOCPickupGunTruck`;
- окреме місце водія;
- окреме місце стрільця;
- водій керує лише машиною;
- стрілець керує лише туреллю;
- автоматична станкова зброя з магазином, запасом та перезарядкою.

### БТР / APC prototype
- `AOCBTR`;
- окремий Driver;
- окремий Gunner;
- більша маса, повільніша динаміка та більший запас міцності;
- окрема башта;
- прототип гармати/важкої турелі;
- корпус не приймає звичайну ballistic, vehicle-cannon або collision damage;
- для ураження корпусу створено `UOCAntiArmorDamageType`, який у S12 буде використовувати RPG/одноразовий протитанковий гранатомет.

## Правило двох гравців
Бойова зброя техніки не прив'язана до водія. Для стрільби потрібен окремий gunner. Якщо водій вийшов, gunner залишається на місці, але сервер не дозволяє вести вогонь, доки місце водія знову не зайняте.

## Командні правила
Після посадки першого члена екіпажу техніка отримує `OccupantTeam`. Другий член екіпажу допускається лише з цієї ж команди. Ворожий гравець може зайняти повністю покинуту машину, але не може сісти до чужого активного екіпажу.

## Керування стрільця
- Mouse X/Y — поворот башти / підняття ствола;
- LMB — утримувати для автоматичного вогню;
- R — перезарядка;
- E — вийти, якщо швидкість достатньо мала;
- TAB — scoreboard залишається доступним через PlayerController.

## Мережева модель
- водій контролює vehicle Pawn через authority-side possession;
- gunner залишається власником свого Character, тому його client→server RPC надсилаються через owned Character;
- Character передає серверу aim/fire/reload intent;
- сам транспорт на сервері перевіряє, чи цей Character справді є Gunner;
- сервер визначає fire cadence, ammo, trace та damage;
- `TurretYaw`, `TurretPitch`, ammo, reload state, Gunner та OccupantTeam реплікуються клієнтам.

## Spawn
У prototype map додано чотири combat vehicle respawner-и:
- 2 × gun truck;
- 2 × BTR/APC;
- по одному комплекту біля кожної сторони карти;
- gun truck respawn приблизно 52 s;
- BTR respawn приблизно 78 s;
- wreck lifetime зберігається окремо від respawn timer.

## Що навмисно ще не фінальне
- source-only геометрія замість фінальних skeletal vehicle assets;
- gunner character поки прихований усередині seat-mode;
- немає пасажирського десанту БТР;
- немає фінального turret recoil animation, muzzle flash, shell eject, audio та impact VFX;
- немає RPG у S11, є лише anti-armour damage contract для S12;
- фінальна Chaos Vehicle реалізація лишається art/vehicle pass після появи коректних моделей.

## Наступна сесія
S12: frag/smoke/flash grenades, інженерна основа, game-only mine/trap framework, repair/disarm та перший anti-armour launcher gameplay class для перевірки бронювання БТР.
