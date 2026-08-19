# R13 Oster Silpo photo reconstruction checkpoint

This checkpoint records the runtime reconstruction currently integrated into `r13-content-gameplay-pass` from the supplied exterior photo set.

## Runtime layers

1. `UOCR13SilpoPhotoModelSubsystem` — replaces only source building/landmark instances inside the Silpo footprint and builds the one-storey shell, stepped parapet, entrance, advertising band and parking apron.
2. `UOCR13SilpoFacadeDetailSubsystem` — replaces the simple sign treatment with the layered blue/orange Silpo sign and authored Ukrainian promotional-board text.
3. `UOCR13SilpoSiteDetailSubsystem` — adds poster rails, facade seams, entrance hardware, blue entrance bin, planted strip and small sign copy.
4. `UOCR13SilpoFoliageUpgradeSubsystem` — uses bundled PN Foliage flower/ground-plant assets when available; procedural plant geometry remains as fallback.
5. `UOCR13SilpoParkingDetailSubsystem` — adds a visual-only row of existing VehicleVarietyPack civilian cars with collision/navigation disabled.
6. `UOCR13SilpoLogoFallbackSubsystem` — verifies Cyrillic glyph support of the active TextRender font and falls back to a geometric `СІЛЬПО` word if required glyphs are unavailable.
7. `UOCR13SilpoCartDetailSubsystem` — adds the shopping trolley visible in the frontage reference using lightweight procedural geometry, with collision/navigation disabled.

## Safety constraints

- Existing road and sidewalk source families are not removed by the Silpo footprint replacement.
- Decorative parked vehicles and shopping cart do not affect navigation or collision.
- Raw supplied photographs are not copied into runtime content.
- The reconstruction does not invent an interior because the supplied reference set is exterior-only.
- `VERIFY_R13_SILPO_PHOTO_MODEL.py` is included in `RUN_ALL_VERIFY.py` and covers the ordered runtime passes and the critical replacement/detail invariants.

## Remaining validation

A fresh UE 5.8 local compile and runtime visual pass is still required for final scale/orientation tuning, font result verification, parked-car spacing, and final facade alignment against the live road geometry.
