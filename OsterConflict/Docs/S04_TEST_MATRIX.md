# S04 acceptance test matrix

Run with UE 5.8 Editor or packaged Client/Server build.

| # | Test | Expected result |
|---|------|-----------------|
| 1 | Spawn player | Primary OC-AR1 and Secondary OC-PST1 exist; AR is active. |
| 2 | Press `2`, then `1` | Current weapon switches without changing K/D/PlayerState. |
| 3 | Press `G` | Active weapon appears in the world and can be picked up again. |
| 4 | Stand near a world weapon | HUD shows `E PICK UP <name>`. |
| 5 | Press `E` near primary weapon | Existing primary is dropped; new primary becomes active. |
| 6 | Press `E` near pistol | Existing secondary is dropped; new pistol becomes active. |
| 7 | Try `E` farther than interaction range | Server does not pick anything up. |
| 8 | Fire/reload, then use ammo box | Reserve ammo increases up to its weapon maximum. |
| 9 | Use ammo box with both reserves full | Ammo box remains because it granted zero ammo. |
| 10 | OC-AR1 arena pickup | Attachment summary includes RedDot + VerticalGrip. |
| 11 | OC-SMG1 arena pickup | Attachment summary includes Suppressor. |
| 12 | OC-LMG1 arena pickup | ExtendedMag increases effective magazine capacity on reload. |
| 13 | OC-SG1 fire | One shell is consumed; server performs multi-pellet traces. |
| 14 | Two clients observe pickup/drop | Inventory/world weapon state is consistent on both clients. |
| 15 | Die after switching/picking weapons | Current weapon drops; pawn respawns with starter loadout; PlayerState stats survive. |
| 16 | Hold fire then switch/drop/pickup | Server fire timer stops and does not continue firing the new/dropped weapon. |

S04 is accepted only when all tests pass in a real UE runtime. The included Python verifier checks source structure, not engine execution.
