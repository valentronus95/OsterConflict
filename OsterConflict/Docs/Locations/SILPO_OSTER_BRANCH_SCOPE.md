# SILPO OSTER — BRANCH SCOPE

Branch: `silpo-oster`
Base: `main` at `fbe66f7502f0cf6ecc621bba575c1e1b35e7e76b`

## Hard scope

This branch is reserved for the Oster Silpo location at Bohdana Khmelnytskoho Street 54.

Allowed work:
- verified geolocation and site anchor;
- Silpo building footprint/massing/facade;
- enterable supermarket shell;
- entrance doors and interior collision/navigation;
- sparse phase-one interior: empty shelves, ordinary checkouts, ordinary lighting;
- immediate Silpo forecourt/site details needed to match the reference location;
- Silpo-only validation and reference documentation.

Out of scope:
- museum geometry/photos;
- stadium geometry/photos;
- unrelated weapons, vehicles, bots, menu/UI or global gameplay refactors;
- moving Silpo work directly into `main` before the location pass is validated.

## Isolation rule

Museum references from the preceding chat turn are explicitly excluded from the Silpo reference set. They must not be copied into any `Silpo_Oster` reference directory or used as modeling evidence for this branch.

All changes created for this task are committed only to `silpo-oster`.
