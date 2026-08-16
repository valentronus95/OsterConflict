# S14A architecture - human priority, squads and chat

## Population invariant

`HumanCount <= MaxPlayerSlots` always. Bots occupy only the difference needed to approach TargetPopulation. A human is
rejected only when the hard human cap is already reached. A session at 16/16 with 11 humans + 5 bots is therefore not
"full" to a human: the join is allowed, then one bot is removed.

Population maintenance formula:

`DesiredBots = clamp(TargetPopulation - HumanCount, 0, MaxPlayerSlots - HumanCount)`

Bot removal is authority-only. The selector prefers the relevant team, then an unpossessed/downed bot, and penalizes an
active vehicle driver. When a human leaves, refill waits a few seconds to avoid rapid join/leave churn.

## Squads

Each team has independent squad IDs. Default squad capacity is four. The first member becomes leader; when a human enters
a squad whose leader is a bot, leadership transfers to the human. If a leader leaves, the server promotes a human member
first, otherwise a bot. Orders are stored server-side per `(Team,Squad)`.

## Chat privacy

Global is routed to all human PlayerControllers. Team is routed only to the sender team. Squad is routed only to the same
team and squad. Team/squad message bodies are therefore not stored as globally replicated GameState properties. The server
sanitizes line breaks, caps messages at 120 characters and rate-limits submissions.

## Pregame/deployment UI split

S14A provides the replicated backend and a source-only HUD panel. S17 replaces this with polished UI including real text
fields for username/chat, mouse navigation, loadout cards, server browser/join flow and saved preferences. The backend RPCs
and PlayerState fields remain unchanged.
