# Acceptance sequence after merge

1. Pull current `main`.
2. Launch only through `START_HERE.cmd` → normal game.
3. Launcher must stop before gameplay if real HMMWV/M2/BTR source ingest fails.
4. Main menu must remain pixel-stable after first START.
5. Choose team/squad/role/base in deployment.
6. Press deployment START; blocking loading bar must begin at 0 and reach 100 before gameplay is exposed.
7. Player must possess near Museum, outside the building, with the test weapon rack nearby.
8. Grass must be populated in gameplay after the frontend transition.
9. Fire AK and pistol: hit trace remains camera-based while visible muzzle/tracer starts at the rendered firearm muzzle area, with no round yellow projectile bead.
10. Inspect HMMWV/M2/BTR presentation; proxy geometry is not an acceptable pass.
