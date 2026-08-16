# S03 Test Matrix

## Acceptance tests

| ID | Test | Expected result |
|---|---|---|
| S03-01 | Dedicated server starts on port 7777 | Server reaches playable map without local viewport |
| S03-02 | Alpha connects to 127.0.0.1:7777 | Server logs join; Alpha receives pawn |
| S03-03 | Bravo connects to same server | Both clients remain in same server world |
| S03-04 | Hold TAB on Alpha | Scoreboard contains Alpha and Bravo |
| S03-05 | Hold TAB while dead | Scoreboard remains available without possessed pawn |
| S03-06 | Alpha kills Bravo | Alpha K +1, Score +100; Bravo D +1 |
| S03-07 | Bravo respawns | Existing K/D/Score persist in PlayerState |
| S03-08 | Observe PING column | Ping is read from PlayerState in milliseconds |
| S03-09 | Execute `SetNickname Test_User` | Name replicates to remote scoreboard |
| S03-10 | Execute `DisconnectFromServer` | Client leaves and server logs disconnect |
| S03-11 | Start 4 PIE clients | PlayerArray/scoreboard list all connected players |
| S03-12 | Respawn several times | PlayerState survives pawn replacement; stats do not reset |

## Failure conditions

S03 is not accepted if any of the following occurs:

- K/D is stored on Character and resets on respawn;
- one client cannot see another player's updated stats;
- TAB only works while the player pawn is alive;
- a client directly changes another player's score;
- scoreboard uses a fake/static ping value;
- join/leave leaves stale rows in `GameState->PlayerArray`;
- controller input mapping erases character input mapping or vice versa.
