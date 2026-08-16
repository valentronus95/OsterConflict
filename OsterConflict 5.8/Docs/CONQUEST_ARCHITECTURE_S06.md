# S06 — Conquest architecture

## Відповідальність класів

### `AOCGameMode`
Server-only rules:
- balanced team assignment;
- friendly-fire policy;
- death -> ticket consumption;
- capture-point ticket bleed;
- spawn selection;
- round end/restart.

### `AOCGameState`
Replicated match state:
- Team 1 tickets;
- Team 2 tickets;
- starting tickets;
- Waiting/InProgress/Ended;
- winning team.

### `AOCPlayerState`
Replicated per-player state:
- TeamId;
- Role;
- K/D/Revives/Score/Ping.

### `AOCCapturePoint`
Server-owned objective simulation:
- overlap counting by team;
- contested state;
- signed capture progress from -1 to +1;
- neutralization;
- ownership changes.

### `AOCTeamSpawnPoint`
Spawn metadata:
- team;
- permanent base spawn or forward spawn;
- optional capture-point dependency.

## Ticket model

Death cost = 1 ticket.

Every `TicketBleedInterval` seconds:
- if Team 1 owns more points, Team 2 loses `Team1Points - Team2Points`;
- if Team 2 owns more points, Team 1 loses `Team2Points - Team1Points`;
- tie means no bleed.

This is intentionally simple for S06. Final balance values belong in a later gameplay-tuning pass.

## Security/network rule

The client never directly changes:
- TeamId;
- ownership of a capture point;
- tickets;
- winner;
- revive success;
- damage acceptance.

Those outcomes are authority-side.
