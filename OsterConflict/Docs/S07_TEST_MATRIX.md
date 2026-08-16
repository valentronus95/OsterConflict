# S07 TEST MATRIX — Oster central greybox

1. **World spawn** — після старту server існує один `AOCWorldSectorOster`.
2. **Large scale** — ground приблизно 1.8 x 1.8 км, старі стіни OCTestArena не блокують карту.
3. **Museum landmark** — у local origin присутній музейний багатоблоковий greybox.
4. **Stadium landmark** — поруч із Museum A існує поле/трибуни/goal proxies.
5. **College anchor** — C знаходиться приблизно 333 м західніше і 94 м північніше Museum anchor.
6. **Park anchor** — B розташований у великому північно-західному секторі, не всередині старого тестового масштабу.
7. **Road collision** — road/sidewalk/building collision не пропускає персонажа крізь тверді greybox-об'єми.
8. **Conquest A** — A neutralize/capture/contested працює у museum/stadium sector.
9. **Conquest B** — B працює у park sector.
10. **Conquest C** — C працює у college sector.
11. **Forward spawn** — захоплена точка відкриває відповідні forward spawn candidates.
12. **Base spawn** — команди стартують на протилежних краях world greybox.
13. **Weapon regression** — усі 6 S04 weapon archetypes можна підібрати.
14. **Medic regression** — downed/revive S05 працює на новій карті.
15. **Scoreboard regression** — TAB K/D/R/Score/Ping і tickets S06 працюють.
16. **Round reset** — після завершення Conquest новий раунд не дублює world geometry.
17. **2–4 clients** — усі клієнти бачать однакове розташування runtime geometry.
18. **Server authority** — capture, damage, pickups і spawns як і раніше визначає server.
