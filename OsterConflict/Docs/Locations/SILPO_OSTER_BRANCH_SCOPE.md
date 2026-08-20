# SILPO OSTER — BRANCH SCOPE

Branch: `silpo-oster`
Base: `main` at `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`

## Hard scope

This branch is reserved for the Oster Silpo location at Bohdana Khmelnytskoho Street 54.

Allowed work:
- verified WGS84 site anchor and Unreal placement;
- Silpo building footprint, massing, facade and entrance;
- the dedicated 20-photo reference pack for this exact store;
- enterable supermarket shell and collision;
- one working replicated public entrance door;
- phase-one interior with empty shelves, checkouts, produce island, refrigeration and neutral lighting;
- R14.1 photo-detail geometry: suspended ceiling, facade trim, checkout markers, utility pole and immediate side-market edge;
- R14.2 non-colliding interior detail: floor-tile grid, shelf end trim, cooler door structure, checkout belts, produce dividers and entrance mat;
- immediate forecourt/site corrections required by the reference set;
- Silpo-only validation scripts, CI contract and documentation.

Out of scope:
- geometry or references for other city locations;
- unrelated weapons, vehicles, bots, menu/UI or global gameplay refactors;
- product/inventory simulation during this pass;
- moving Silpo work into `main` before compile/PIE and location validation.

## Isolation rule

Only source material belonging to this Oster Silpo pass may be stored under `SourceReferences/Locations/Silpo_Oster`.

All changes for this task stay on `silpo-oster` until the location pass is explicitly approved for integration.

Static contract command:

`python OsterConflict/Tools/validate_silpo_location.py`
