# SILPO OSTER — INTEGRATION SCOPE

Source branch: `silpo-oster`
Integration branch: `main`
Initial branch base: `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`
Main forward-port commit: `ad689dff859bc65332669788cb94f727623ce7ab`
Status: `CODED_UNTESTED`

## Hard scope

This implementation is reserved for the Oster Silpo location at Bohdana Khmelnytskoho Street 54.

Included work:
- verified WGS84 site anchor and Unreal placement;
- Silpo building footprint, massing, facade and entrance;
- the dedicated 20-photo reference pack for this exact store;
- enterable supermarket shell and collision;
- one working replicated public entrance door;
- phase-one interior with empty shelves, checkouts, produce island, refrigeration and neutral lighting;
- R14.1 photo-detail geometry: suspended ceiling, facade trim, checkout markers, utility pole and immediate side-market edge;
- R14.2 non-colliding interior detail: floor-tile grid, shelf end trim, cooler door structure, checkout belts, produce dividers and entrance mat;
- R14.3 non-colliding facade identity: layered orange/blue sign approximation, dark parapet rails and the photographed parking sign;
- immediate forecourt/site corrections required by the reference set;
- Silpo-only validation scripts, CI contract and documentation.

Out of scope:
- geometry or references for other city locations;
- unrelated weapons, vehicles, bots, menu/UI or global gameplay refactors;
- product/inventory simulation during this pass.

## Integration rule

The user explicitly approved moving the complete Silpo pass into `main` on 2026-08-20. The integration was performed as a controlled forward-port on top of the then-current `main`, preserving newer main work rather than force-moving `main` to the older location branch.

The implementation remains `CODED_UNTESTED` until UE 5.8 build/PIE and user-visible runtime checks confirm placement, collision, door interaction, navigation, flicker and photo fidelity.

Canonical source references are retained in the repository and linked from `SILPO_OSTER_TZ.md`.

Static contract command:

`python OsterConflict/Tools/validate_silpo_location.py`

Windows UE 5.8 validation launcher:

`VALIDATE_SILPO_UE58.cmd`
