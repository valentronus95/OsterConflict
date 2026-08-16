# S13 — AI / Bots

## Що додано

S13 додає server-authoritative ботів без обов'язкових Behavior Tree/Blackboard `.uasset`, щоб source-only контрольна точка залишалась самодостатньою.

- `AOCAIController` + `UAIPerceptionComponent` / sight.
- Dynamic NavMesh runtime configuration.
- Чотири складності: Easy / Normal / Hard / Veteran.
- Різні reaction time, sight radius, field of view, aim error, combat range, cover cadence.
- Пошук ворога, line-of-sight, стрільба штатною weapon framework S04.
- Простий пошук укриття через sampling + nav projection + visibility trace.
- Пріоритет objective A/B/C; боти фізично заходять у capture radius і захоплюють точки через існуючу S06 систему.
- Medic-бот шукає пораненого союзника та виконує той самий 3-секундний revive S05.
- Боти відкривають двері/ворота перед маршрутом.
- Перша vehicle logic: при далекій цілі бот може сісти за кермо вільної техніки, їхати до objective та вийти біля точки.
- URL: `?Bots=8?BotDifficulty=Normal`.
- Sandbox F10: `Spawn 4 AI bots` / `Clear AI bots`.

## Межі S13

Фінальні animation graphs, tactical squads, grenade AI, gunner AI, suppression, true cover/EQS, hearing, vehicle convoy behavior і Behavior Tree/DataAsset authoring залишаються наступним AI/animation polish. Нинішній bot brain є функціональною C++ основою для тестування матчу.

## Round lifecycle / cleanup

- AI respawn uses the same match lifecycle, but returns as `AOCBotCharacter`.
- Round restart re-seats bots at team spawns instead of leaving survivors at the previous objective.
- If an AI driver is in a vehicle, it exits before its old character pawn is replaced; the vehicle remains available.
- `Clear AI bots` removes AI controllers/pawns without intentionally deleting occupied test vehicles.
- Revive is cancelled when medic AI changes priority away from the wounded target.
