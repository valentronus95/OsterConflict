# S16C Test Matrix

1. Default server assigns TeamOne = UA Special Unit and TeamTwo = Masked Fighters.
2. `?Team1Faction=Rangers?Team2Faction=Insurgents` changes both sides.
3. Server prevents both teams from accidentally resolving to the same default visual faction.
4. Human PlayerState replicates FactionArchetype + AppearanceSeed.
5. AI PlayerState receives the same faction rule as humans.
6. Two clients see the same faction/variant for the same PlayerState.
7. Re-spawned pawn rebuilds the same appearance from PlayerState.
8. Source-only proxy is visible to other players when no production profile mesh exists.
9. Local player does not see the third-person proxy body in first-person camera.
10. Local player sees source-only FPS proxy arms when no FPS arms mesh exists.
11. Setting a production visual profile disables the proxy body and applies skeletal mesh/AnimClass.
12. `UOCCharacterAnimInstance` updates speed/direction/falling/crouch/sprint/ADS/reload/life/vehicle values.
13. Server-confirmed Fire emits cosmetic action event without changing gameplay authority.
14. ReloadStart emits only when reload actually begins.
15. ReviveStart/Complete, Downed, Revived and Death have action hooks.
16. Deployment HUD displays FACTION without overlapping READY/server lines.
17. Existing S14B ragdoll uses the production GetMesh path when a skeletal mesh is assigned.
18. S04–S16B regressions still pass.
