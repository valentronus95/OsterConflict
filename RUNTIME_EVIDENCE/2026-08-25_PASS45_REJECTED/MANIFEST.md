# Pass 45 runtime rejection evidence — 2026-08-25

This pack is authoritative user-observed UE 5.8 runtime evidence after the local build/import blocker from PR #82 was removed.

![Pass 45 rejected runtime evidence](pass45_runtime_rejection_20260825.jpg)

## Runtime verdict

**PASS 45 = RUNTIME REJECTED.**

The latest run reaches gameplay and proves the compile blocker is no longer the immediate blocker, but the visual/gameplay result is not acceptable.

## Evidence shown in the screenshots

1. **Black world/material corruption.** Large areas of ground/world render almost entirely black while isolated grass/trees/objects remain visible.
2. **Weapon material gaps remain.** AK-47 has authored appearance; several other rack/pickup weapons are white/default or otherwise untextured.
3. **Invalid Oster residential/fence visuals.** Generic foreign fence/house assets are visible and do not match the supplied Oster references.
4. **Wrong landmark identity at Museum.** A large six-column Culture-House-like civic shell is still presented at/near the Museum spawn instead of the real Oster Local History Museum appearance.
5. **Unreferenced dark tower/shack.** A tall dark steep-roof structure remains visible although it is not part of the accepted Oster reference set.
6. **HMMWV production asset is visibly deformed.** Body proportions look accordion-stretched/non-uniform; M2 Browning mount is visibly misaligned.
7. **Mounted Browning input inversion.** User reports inverted gun control; inversion must be disabled for the normal gunner route.
8. **Vehicle possession/exit teleport bug.** Entering the red civilian vehicle teleported vehicle/player to Museum; after driving to the BTR and exiting, the player was again thrown back to Museum.
9. **BTR-4 production asset is visibly deformed.** Body/rear proportions are stretched, orientation appears suspect, and a large white/default material artifact is visible.
10. **Unexpected windowed mode.** Normal route opened in a window rather than the intended normal fullscreen/borderless presentation.
11. **Thermal load remains excessive.** Runtime shows roughly 100–156 FPS while the machine heats strongly. High FPS in this broken/black visual state is not performance acceptance.

## External reference confirmation

Public Oster references checked during this corrective pass confirm:

- Oster Local History Museum is the former Solonyna house, a late-19th-century brick/wood residence at Tatarska 30; it is not the six-column neoclassical Culture House facade.
- The six-column neoclassical public building is a separate Oster civic/culture building and must never own the Museum site.

User-supplied references remain higher authority than public-web references when they conflict.

## Acceptance consequences

Pass 45 remains rejected until a later factual local UE run proves all applicable corrections:

- no black world/terrain material corruption;
- no white/default required weapon materials;
- correct, separate Museum and Culture House visual identities;
- no unapproved generic Oster fences/houses/towers;
- proportional HMMWV/BTR meshes with correct orientation/materials;
- correctly aligned M2 Browning and non-inverted normal gunner input;
- vehicle exit/enter never teleports to Museum fallback;
- intended fullscreen/borderless normal route;
- thermal-safe normal frame cap without lowering native render-scale target;
- actual local UE runtime evidence, not source CI alone.

Canonical corrective specification: `PASS45_RUNTIME_RECOVERY_TZ.md`.
