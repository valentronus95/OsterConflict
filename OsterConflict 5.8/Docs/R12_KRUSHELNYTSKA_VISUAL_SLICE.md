# R12 — KRUSHELNYTSKA VISUAL SLICE

## Goal

Replace the R11 source-only residential greybox with the first recognizable modern Oster street slice, centered on Solomii Krushelnytskoi Street (former 8 Bereznia).

This pass is intentionally a vertical slice rather than an attempt to decorate the whole 2.4 km source-only sector at once.

## Current-reference visual language

Primary current street references are the user's August 2026 photographs supplied during development. They show:

- a narrow, worn asphalt carriageway without a formal curb along most residential frontage;
- pale sandy shoulders and irregular grass edges;
- dense mature deciduous trees close to the road, frequently forming a partial canopy;
- detached low-rise houses set behind the street line;
- mixed opaque frontage: wood, painted metal, corrugated/light sheet and precast concrete sections;
- wide metal vehicle gates plus smaller pedestrian gates;
- overhead utility lines and simple utility poles;
- parked civilian cars partly on grass/sandy verges;
- warm low-angle late-afternoon light and long tree shadows;
- imperfect, non-uniform lot spacing rather than a suburban grid.

The 24/26 frontage photograph is used as a morphology reference for gate/fence proportions and mixed-material frontage, not as survey-grade reconstruction of a private residence.

## Public-source research

Public photographs of Oster are also used for city-wide morphology and landmark cross-checking. The Wikimedia Commons Oster category is the preferred repository source because individual files expose their reuse license on the file-description page. Public photos from other sites may be inspected as visual research but are not copied into the repository unless their license explicitly permits it.

## R12 implementation in this branch

- Adds an auto-instanced `UWorldSubsystem` for PIE/Game worlds.
- Waits until the R11 `OCWorldSectorOster` actor has been spawned.
- Hides the old source-only residential cube/sphere families while retaining macro ground/road/landmark topology.
- Builds the Krushelnytska slice from actual `AdvancedVillagePack` static meshes already present in the project:
  - `SM_House_Var01` / `SM_House_Var02`;
  - `SM_Fence_Var01..04`;
  - `SM_Tree_Var01..05`.
- Uses deterministic irregular placement for houses, frontage and mature roadside vegetation.
- Tunes the environment toward warm late-afternoon Oster lighting with longer shadows.

## Acceptance for this first pass

The visual test is successful only if the runtime no longer reads as a field of residential cubes and sphere-trees. The player should immediately see a believable low-rise Oster residential corridor with real mesh silhouettes, mixed fences and a dense roadside canopy.

This is not the final art pass. Road-edge materials, sandy shoulders, utility poles/wires, civilian vehicle art, modern gates, house-specific material variation and more accurate street geometry remain subsequent R12 passes.
