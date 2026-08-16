# S06 Multiplayer Test Matrix

Use one dedicated server and four clients.

Recommended order:
1. `RUN_S06_SERVER_7777.bat`
2. Alpha
3. Bravo
4. Charlie
5. Delta

Expected auto-balance:
- Alpha: Team 1
- Bravo: Team 2
- Charlie: Team 1
- Delta: Team 2

Exact order can differ if players reconnect; required condition is max team-size difference <= 1.

## Tests

### T01 — team balance
Connect 4 clients. Verify 2/2 split and replicated TeamId on every TAB scoreboard.

### T02 — friendly fire off
Alpha shoots Charlie. Charlie must not lose HP and Alpha must not receive a hitmarker.

### T03 — enemy damage
Alpha shoots Bravo. Damage/downed/death pipeline must work normally.

### T04 — friendly-only revive
Down Charlie. Alpha (Medic) can revive Charlie. Alpha cannot revive downed Bravo.

### T05 — non-medic restriction
Connect or travel with `?Role=Rifleman`. A Rifleman must not receive a revive prompt and cannot start revive.

### T06 — capture
Only Team 1 enters A. Progress moves toward Team 1 and A eventually becomes Team 1-owned.

### T07 — contested
One alive player from each team enters B. B shows contested and capture progress stops.

### T08 — neutralize then capture
Team 1 captures C. Team 2 attacks it. Ownership first becomes neutral, then Team 2 after continued uncontested presence.

### T09 — death ticket
Record Team 2 tickets. Kill Bravo fully. Team 2 loses exactly one ticket after final death, not at Downed.

### T10 — revive preserves ticket
Down Charlie and revive before bleed-out. Team 1 ticket count must not decrease.

### T11 — ticket bleed
Give Team 1 ownership of two points and Team 2 one point. After one bleed interval Team 2 loses one ticket.

### T12 — base respawn
Kill a player while their team owns no objective. Respawn must occur at a base spawn for that team.

### T13 — forward respawn
Capture A, then die. A forward spawn becomes eligible for that team when not contested.

### T14 — round end
Drive one team to 0 tickets. HUD shows winner; new server-authoritative shots stop; no normal respawn occurs during end state.

### T15 — automatic next round
Wait 8 seconds after end. Tickets reset to 200/200, A/B/C reset neutral and players respawn.

### T16 — TAB persistence
TAB must show Team 1 and Team 2 columns, K/D/R/Score/Ping, including during Downed and end-of-round state.
