# MATCH_POPULATION_SPEC — S14A

## Параметри
- `MaxPlayerSlots`: жорсткий ліміт реальних людей. Default: 16.
- `TargetPopulation`: бажаний сумарний склад Human + Bot. Default: 16.
- `BotFill`: 1/0, автоматичне заповнення AI.
- `BotRefillDelay`: затримка повернення AI після виходу Human. Default: 3 с.

## Human priority
1. `PreLogin` рахує лише реальних Human.
2. Якщо Human >= MaxPlayerSlots — `SERVER_FULL_HUMANS`.
3. Інакше Human приймається незалежно від поточної кількості bot-fill AI.
4. Після `PostLogin` population manager доводить склад до TargetPopulation.
5. Якщо треба звільнити місце — спочатку AI тієї команди, куди зайшла людина; перевага non-critical/unpossessed/downed AI.
6. Видалення AI не створює kill/death/ticket/score подію.
7. Якщо AI керує транспортом, seat/possession звільняється до знищення AI controller/pawn; vehicle не знищується.

## Refill
- Після Human Logout запускається delayed maintain population.
- Refill не перевищує `TargetPopulation` і не створює більше AI, ніж `MaxPlayerSlots - Humans`.
- `BotFill=0` вимикає автоматичний refill.

## Мінімальні тести
- 6 Human + 10 Bot -> входить 7-й Human -> 7 Human + 9 Bot.
- 16 Human -> 17-й Human отримує SERVER_FULL_HUMANS.
- Human disconnect -> через delay повертається AI.
- Bot-driver eviction -> vehicle залишається у світі.
