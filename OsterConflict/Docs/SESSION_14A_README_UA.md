# Oster Conflict - S14A

## Lobby / human-bot population / squads / chat / household dressing

S14A deliberately precedes the S14B animation/gore/destruction pass because server population, squad identity and chat
need stable replicated identity before more presentation systems are layered on top.

### Implemented source milestone

- configurable hard human cap (`?MaxPlayers=16`);
- configurable target population (`?Population=16`);
- bots are filler only, never hard-reserved player slots; population invariant uses `HumanCount` and `BotCount`;
- if a human joins a bot-filled target population, the server removes a bot rather than rejecting the human;
- if the server reaches the hard cap with humans only, `PreLogin` rejects additional humans;
- after a human disconnects, bot fill can restore the configured target population after a short delay;
- replacement prefers a bot from the useful team and avoids active vehicle drivers when another candidate exists;
- replicated `IsBot`, `SquadId`, `SquadLeader`, `LobbyReady` in PlayerState;
- squads: max 4 by default, Alpha/Bravo/Charlie/Delta etc., auto-assignment and leader repair;
- human leader is preferred over bot leader when a human joins that squad;
- squad leader orders: attack A/B/C, defend A/B/C, move, regroup;
- AI reads current squad orders before its normal Conquest objective selection;
- global/team/squad chat routing with server validation and 0.6 s rate limit;
- team/squad chat text is routed only to eligible PlayerControllers, not replicated globally;
- source-only pre-game/deployment HUD: username, team, squad, role, ready, server human/bot counts;
- F2 role, F3 squad, F4 ready/deploy, F8 panel toggle;
- current source build changes username with `SetNickname <name>`; final editable field remains an S17 UI task;
- non-interactive household dressing: sofa, mismatched chairs, dining table, kitchen counters, fridge, stove proxy,
  wardrobe, cabinet, desktop monitor/PC tower, laptop and randomized low-cost clutter;
- deterministic house interior seed + Worn/Ordinary/Maintained condition profile;
- household props are visual/physical dressing only in this milestone, not loot/interactable inventory.

### Console chat/order test commands

- `SayGlobal hello`
- `SayTeam left side`
- `SaySquad regroup`
- `SquadAttack A`
- `SquadDefend B`
- `SquadMoveHere`
- `SquadRegroup`

### Example server URLs

- `OsterConflictServer.exe OsterPrototype?MaxPlayers=16?Population=16?BotDifficulty=Normal -log -port=7777`
- `OsterConflictServer.exe OsterPrototype?MaxPlayers=16?Bots=8?BotFill=1?BotDifficulty=Hard -log -port=7777`

If `Bots=N` is supplied without `Population`, N becomes the filler target. Humans can exceed that target up to MaxPlayers;
bots fall to zero as enough humans join.

## Готові локальні launch-сценарії
- `Scripts/RUN_S14A_HYBRID_SERVER_16.bat` — 16 target population, auto-fill bots, Normal AI.
- `Scripts/RUN_S14A_CLIENT_ALPHA.bat`
- `Scripts/RUN_S14A_CLIENT_BRAVO.bat`

Додаткові специфікації: `MATCH_POPULATION_SPEC.md`, `SQUAD_CHAT_SPEC.md`, `INTERIOR_DRESSING_SPEC.md`.
