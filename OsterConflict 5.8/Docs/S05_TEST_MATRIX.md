# S05 Test Matrix

Minimum setup: dedicated server + 2 clients. Recommended: 3 clients.

1. Ordinary lethal damage -> victim becomes Downed, not immediately respawned.
2. Downed HUD starts near 01:00 and counts down consistently on the victim.
3. Downed player can move slowly with WASD but cannot sprint, ADS, fire, reload, jump, drop or switch weapons.
4. Downed camera is lower and equipped weapon is hidden.
5. Wait 60 seconds without revive -> Death increments and normal respawn occurs.
6. While Downed, hold Space less than 2 s then release -> give-up cancels.
7. While Downed, hold Space >= 2 s -> final death and respawn flow.
8. Alive second player approaches Downed target -> `HOLD E REVIVE <name>` prompt appears.
9. Hold E for 3 s inside range with clear line-of-sight -> target revives at 35 HP.
10. Revived player does not receive a Death and begins normal regeneration after delay.
11. Reviver receives +1 Revive and +50 Score; TAB scoreboard shows R column.
12. Start revive then release E early -> revive cancels.
13. Start revive then move farther than 220 cm -> revive cancels.
14. Start revive then place blocking geometry between players -> revive cancels.
15. Two revivers target one player -> first successful completion revives target; other revive cancels when target is no longer Downed.
16. Shoot a Downed player with a normal firearm -> qualifying finishing damage causes final death.
17. Apply one hit >= instant-death threshold to Alive player -> bypass Downed and go directly Dead.
18. Hold TAB while Downed -> scoreboard replaces combat/downed HUD and remains readable.
19. Bleed-out kill attribution: attacker downs victim, nobody finishes, victim bleeds out -> attacker receives kill.
20. Finisher attribution: attacker A downs victim, attacker B finishes -> B receives final kill attribution under current prototype rule.

S05 passes only when all gameplay-relevant checks above work on server + remote client, not just listen-server local play.
