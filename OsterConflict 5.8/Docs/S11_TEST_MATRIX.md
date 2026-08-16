# S11 Test Matrix

1. Запустити dedicated server + мінімум 2 клієнти однієї команди.
2. Player A підходить до gun truck: prompt `E ENTER DRIVER`.
3. Player A входить Driver і отримує W/S/A/D/handbrake/camera input.
4. Player B тієї ж команди бачить `E ENTER GUNNER` і входить без possession vehicle Pawn.
5. Player B mouse X/Y рухає turret; Player A продовжує незалежно керувати машиною.
6. LMB gunner зменшує magazine ammo відповідно до server RPM.
7. R запускає replicated reload і переносить ammo з reserve у magazine.
8. Водій не може стріляти turret своїм LMB.
9. Driver виходить: gunner залишається, але turret fire server-side блокується до появи Driver.
10. Ворожий Player C не може зайняти вільний gunner seat, поки vehicle має crew іншої команди.
11. Після повного виходу екіпажу vehicle team reset, і ворожа команда може захопити покинуту машину.
12. Mounted MG пошкоджує персонажів та небронировану/легку техніку.
13. Infantry ballistic fire не зменшує BTR hull HP.
14. Gun-truck MG не зменшує BTR hull HP.
15. BTR vehicle-cannon damage class не зменшує іншому BTR hull HP згідно з поточним TZ rule.
16. Collision не може знищити BTR hull у S11.
17. `UOCAntiArmorDamageType` проходить BTR hull filter; фактичний launcher з'явиться в S12.
18. BTR wreck примусово висаджує Driver і Gunner та не зникає миттєво.
19. Gun truck/BTR respawn незалежний від wreck lifetime.
20. TAB scoreboard працює для Driver і Gunner.
21. Gunner HUD показує turret name, hull %, magazine/reserve ammo, reload state та ping.
22. 4 combat respawners створюються разом із 8 civilian vehicle respawners.
23. S04–S10 regression verifiers проходять після S11 змін.
