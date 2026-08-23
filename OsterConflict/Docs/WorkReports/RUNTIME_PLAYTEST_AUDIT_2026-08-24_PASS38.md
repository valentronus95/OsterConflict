# Runtime playtest audit — 2026-08-24 — Pass 38 trigger

## User runtime evidence

Latest UE 5.8 playtest after Pass 37 (`main` merge `3041fd94...`) is authoritative over green source CI:

- player still sees a flat/empty field and the Museum is not visible in the initial BASE view;
- weapon rack presentation is mixed: some authored-looking models, some grey/blank models, and flat fallback-colour presentation on others;
- screenshots show FPS decreasing from 26 to 10 to 8;
- user reports approximately 60 FPS dropping to about 5 FPS within ~5 seconds;
- user also reports rapid laptop heating during the same collapse.

## Source localization

Highest-confidence lifecycle defect is `UOCMuseumVisibilityPass37Subsystem::ValidateVisibleMuseum()`:

- previous logic could retire every R13.8 architecture owner and rebuild the complete museum every 0.35 s while visible-core evidence remained below threshold;
- this destructive branch could repeat across the entire polling window;
- repeated actor/component/material construction matches the rapid progressive FPS/thermal collapse far better than a one-time static scene cost.

Additional unnecessary recurring work found:

- `UOCRealWeaponFallbackSubsystem` scanned all weapons every 0.25 s indefinitely;
- `UOCWeaponPalettePass37Subsystem` had no hard scan-count ceiling if rack audit never converged;
- Pass 37 forced recolouring overwrote non-placeholder imported materials on known restored payloads, explaining flat-colour visual regressions such as the Lever Action.

## Pass 38 correction

- Museum destructive recovery budget: maximum one rebuild; later polls are observational/fail-closed.
- Late R13.8 duplicate cleanup remains one-shot after the historical delayed startup window.
- Real-weapon fallback scan: 0.5 s interval, maximum 12 startup passes, stop on convergence.
- Weapon palette scan: maximum 12 startup passes, stop on convergence.
- Palette recovery now touches only explicit placeholder materials; non-placeholder imported materials are preserved.
- Full runtime acceptance keeps the >=30 FPS floor and fails on any Pass 38 rebuild/scan budget failure marker.

## Status

`CODED_UNTESTED` until a fresh local UE 5.8 run confirms no rapid FPS/thermal collapse and proves Museum/weapon presentation in runtime.