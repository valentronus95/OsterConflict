# SQUAD_CHAT_SPEC — S14A

## Squads
- До 4 учасників у базовому squad.
- Назви: Alpha, Bravo, Charlie, Delta; резерв: Echo, Foxtrot, Golf, Hotel.
- `SquadId` і `bSquadLeader` зберігаються в PlayerState.
- Автопризначення: найменш заповнений squad своєї команди.
- Якщо leader-bot і входить Human, leadership переходить Human.
- При виході leader сервер ремонтує leadership: Human first, потім Bot.

## Orders
- Attack Objective
- Defend Objective
- Move
- Regroup

Тільки Squad Leader видає order. Сервер перевіряє objective/позицію і доставляє order учасникам squad. AI squad враховує order після combat/revive/self-preservation пріоритетів.

## Chat
- ALL: усі Human-клієнти.
- TEAM: тільки одна TeamId.
- SQUAD: тільки TeamId + SquadId.
- TEAM/SQUAD маршрутизує сервер; заборонені клієнти не отримують message payload.
- Message max: 120 символів.
- Control chars/newlines sanitization.
- Rate limit: 0.6 с між повідомленнями одного відправника.
- Bots не генерують текстові chat messages у S14A.
