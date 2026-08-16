# S10 Test Matrix

1. Сервер створює 8 vehicle spawn points і 8 машин.
2. Клієнт бачить `E ENTER VEHICLE` у межах interaction trace.
3. Після E PlayerController possesses vehicle, Character прихований.
4. Другий клієнт не може зайняти driver seat уже зайнятої машини.
5. W/S/A/D змінюють рух лише через server RPC.
6. Space створює сильне гальмування / ослаблення бокового grip.
7. RMB дозволяє free-look і камера повертається в центр після відпускання.
8. C перемикає interior / third-person camera.
9. E не випускає водія на високій швидкості.
10. E випускає водія на низькій швидкості й повертає possession Character.
11. Кулі зменшують VehicleHealth і змінюють damage stage.
12. Сильне зіткнення завдає collision damage.
13. При 0 HP driver примусово виходить, машина стає Wrecked.
14. Wreck не зникає миттєво.
15. Після respawn delay з'являється нова машина.
16. TAB scoreboard і match HUD продовжують працювати при vehicle possession.
17. S04 weapon pickup, S05 revive, S06 conquest, S08 interaction, S09 map regression не ламаються.
