# S13 test matrix

| # | Перевірка | Очікування |
|---|---|---|
| 1 | Server `?Bots=8?BotDifficulty=Normal` | 8 ботів, баланс 2 команд |
| 2 | Easy vs Veteran | Veteran реагує/цілиться швидше, Easy помиляється частіше |
| 3 | Bot бачить ворога | переходить у Combat, повертається, стріляє штатною weapon system |
| 4 | Ворог за стіною | LOS блокує вогонь; бот рухається/шукає позицію |
| 5 | Objective A/B/C | бот доходить у capture radius, існуюча S06 capture logic його рахує |
| 6 | Medic + downed ally | бот підходить і запускає 3-second revive |
| 7 | Двері на маршруті | бот відкриває interactable door/gate |
| 8 | Далека objective + авто | бот може сісти водієм, їхати й вийти біля objective |
| 9 | Бот помирає | stats/ticket застосовуються, після RespawnDelay бот повертається як bot pawn |
|10 | TAB | bot PlayerState показує ім'я, K/D/R/score/ping row |
|11 | Sandbox F10 Spawn 4 bots | сервер створює 4 ботів |
|12 | Sandbox F10 Clear bots | усі AI controllers/pawns видаляються |
|13 | 16 bots 10 min | немає безконтрольного spawn/PlayerState duplication |
|14 | S04–S12 regression | weapon/revive/conquest/vehicle/sandbox попередні verifiers PASS |

|15 | Round restart with surviving/driving bots | bots are reset to team spawns; vehicles are not orphaned/destroyed by bot reset |
|16 | Medic starts revive then acquires close enemy | revive timer is cancelled; no delayed ghost revive |
