# S17B Test Matrix

Після реальної UE-компіляції виконати мінімум такі тести.

1. Frontend -> SETTINGS відкриває Graphics page, курсор активний, gameplay input заблокований.
2. Resolution/display mode Apply реально змінює режим і переживає restart.
3. VSync/frame limit/resolution scale переживають restart.
4. Preset High змінює individual quality rows; manual individual edit працює як Custom behavior backend.
5. Audio Master slider/checkbox і окремий Weapons/Ambience/Music/UI slider працюють незалежно та зберігаються.
6. Menu Music checkbox не обнуляє збережений Music %.
7. FOV 75/90/120 помітно змінює FPS camera FOV після Apply.
8. Mouse sensitivity та ADS multiplier застосовуються без reconnect.
9. Invert Y працює в піхотному огляді; vehicle/gunner sensitivity не ігнорує user sensitivity.
10. Rebind Fire LMB -> інший допустимий ключ оновлює Enhanced Input після Apply.
11. Rebind на вже зайнятий primary key робить swap, не дубль.
12. Rebind Scoreboard/Chat змінює Controller Mapping Context.
13. Escape під час очікування клавіші скасовує лише rebind, не закриває Settings.
14. CANCEL після незастосованих slider/key змін повертає останні saved values.
15. RESET DEFAULTS не зберігає одразу; CANCEL після нього відновлює старий профіль.
16. RESET DEFAULTS + SAVE & BACK зберігає defaults.
17. Show FPS/Ping/Crosshair/Hitmarker кожен незалежно впливає на HUD.
18. Gore Off не змінює damage/death на сервері, лише presentation.
19. Reduce Flashes зменшує local flash alpha, але не змінює server gameplay result.
20. Camera Shake 0% прибирає local weapon camera shake; 100% повертає повний authored shake.
21. Dedicated server запускається без UI/settings LocalPlayer assumptions/crash.
22. Два клієнти з різними FOV/audio/key profiles не впливають один на одного.
23. Custom frame limit, який не входить у стандартний список, коректно відображається після Sync.
24. Невалідна/відсутня config-секція не ламає запуск: використовуються class defaults.
25. Після restart збережені Engine/Audio/Player settings відновлюються.

Відомі content-залежності milestone:
- full HUD scale потребує фінальної UMG gameplay HUD migration;
- ColorVisionMode потребує production viewport/post-process transform;
- subtitles потребують майбутнього dialogue/subtitle presentation layer.
