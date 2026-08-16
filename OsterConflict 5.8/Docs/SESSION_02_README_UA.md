# Oster Conflict — Session 02
## Enhanced Input + Combat Feel

### Що реалізовано

S02 переводить керування з legacy Action/Axis mappings на Enhanced Input і додає перший нормальний бойовий цикл.

1. **Enhanced Input**
   - EnhancedInput plugin увімкнений у `.uproject`.
   - `EnhancedPlayerInput` і `EnhancedInputComponent` встановлені в `DefaultInput.ini`.
   - Input Actions та Mapping Context створюються в C++, тому для запуску цієї source-only збірки не треба вручну створювати `.uasset` input-файли.

2. **Керування**
   - W/A/S/D — рух.
   - Mouse — огляд.
   - Space — стрибок.
   - Left Ctrl — присідання.
   - Left Shift — спринт.
   - LMB — вогонь.
   - RMB — ADS/прицілювання.
   - R — перезарядка.
   - B — перемикання AUTO / SEMI.

3. **ADS**
   - плавна зміна FOV;
   - менший spread;
   - нижча швидкість руху;
   - спринт і ADS взаємовиключні.

4. **Стрільба і мережа**
   - клієнт надсилає серверу стан Fire Held;
   - сервер сам запускає cadence автоматичного вогню;
   - сервер робить trace, spread і damage;
   - semi mode дає один серверний постріл на натискання;
   - AUTO працює за RPM зброї;
   - сервер припиняє цикл при порожньому магазині, смерті або спринті.

5. **Spread**
   - hip spread;
   - ADS spread;
   - додатковий multiplier під час руху;
   - сервер генерує фактичний напрямок пострілу.

6. **Recoil**
   - локальний pitch/yaw kick для миттєвої реакції;
   - recovery після короткої затримки;
   - параметри recoil знаходяться в `AOCWeaponBase` і надалі будуть винесені в data-driven weapon definitions у S04.

7. **Reload state**
   - перезарядка більше не миттєва;
   - має серверний таймер;
   - стан реплікується;
   - HUD показує `RELOAD`;
   - стрільба може перервати reload, якщо в магазині ще є патрони;
   - спринт перериває reload.

8. **HUD feedback**
   - динамічний crosshair gap;
   - AUTO/SEMI;
   - hit marker;
   - fatal-hit marker червоного відтінку;
   - напрямок отриманої шкоди;
   - старий CoD-подібний damage vignette залишено.

9. **Camera shake hook**
   - `FireCameraShakeClass` готовий для майбутнього animation/VFX pass;
   - якщо клас не заданий, recoil все одно працює без asset-залежності.

### Серверна модель

Gameplay-результат пострілу визначається сервером. Клієнт відповідає за локальне відчуття recoil/FOV, але не визначає damage, RPM або spread фактичної кулі.

### Що свідомо НЕ входить у S02

- weapon animations;
- skeletal weapon mesh;
- muzzle flash Niagara asset;
- звуки;
- weapon DataAssets;
- pickup/drop;
- lobby;
- scoreboard;
- повноцінний lag compensation/history rewind.

Це переходить у наступні сесії, а не запихається в один клас розміром із кримінальний кодекс.

### Перевірка

У середовищі генерації немає встановленого UE 5.8 toolchain, тому виконана source/structure verification, а не справжній Unreal Build Tool compile.

Запустити перевірку:

```bat
python VERIFY_S02.py
```

Наступний модуль: **S03 — Multiplayer test harness + PlayerState + TAB scoreboard + ping + IP connect flow**.
