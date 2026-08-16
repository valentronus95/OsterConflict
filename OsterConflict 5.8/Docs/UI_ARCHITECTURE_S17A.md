# UI Architecture — S17A

## 1. Шари
1. `AOCHUD` — gameplay HUD/fallback Canvas.
2. `UOCGameUIRootWidget` — source-only UMG orchestration.
3. `AOCPlayerController` — local input/UI state + validated RPC boundary.
4. `AOCPlayerState/AOCGameState` — replicated data model.
5. `AOCGameMode` — authority-only team/squad/spawn/chat/population rules.

## 2. Input modes
- Gameplay: Game Only, mouse hidden.
- Frontend / Deployment / Sandbox Admin / Chat Input: Game + UI, mouse available, movement/look locked where appropriate.
- TAB scoreboard: overlay only; no gameplay state mutation.

## 3. Direct connect
Frontend builds a travel URL `IP:Port?Name=Username`; the server still sanitizes/uniquifies the name in `InitNewPlayer`.

## 4. Team request
A player can request Team 1/2 while not deployed/ready. The server rejects requests that would make the human team imbalance worse. Bots remain filler and can be removed/refilled by existing population logic.

## 5. Spawn request
PlayerController holds authority-side requested spawn id: `BASE`, `A`, `B`, `C`.
`FindBestSpawnTransform` first tries the selected available spawn. If it is not available (objective neutral/lost/contested), it falls back to the safest available team spawn.

## 6. Chat
`T` activates the entry field. Channel cycle: `ALL -> TEAM -> SQUAD -> ALL`.
Messages still pass through S14A server sanitation, routing and rate limiting.

## 7. Future S17B
- graphics/audio/control pages;
- FOV/FPS/HUD/Gore settings;
- key remapping;
- persistence and reset-to-defaults;
- final visual skin/icons/transitions and accessibility pass.
