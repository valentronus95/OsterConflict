Latest UE 5.8 user runtime after Pass 36 is authoritative over green source CI: the player still appears beside the BASE rack in an empty/flat field with no visible Museum, most restored rack weapons are still white/grey while AK is textured, and the supplied screenshot shows FPS 27.

Pass 37 fixes the repeated regressions rather than adding more decorative layers:

- moves primary BASE from ~41 m to ~27.8 m from MuseumAnchor on the exterior front approach and faces it toward Museum; secondary remains ~38.6 m;
- aligns deployment guard to the new 20-45 m visible approach instead of silently preferring a farther secondary BASE;
- adds actual Museum visibility validation: >=12 registered visible MuseumStructural components must exist near MuseumAnchor, stale/empty R13.8 owners are rebuilt, and duplicate late R13.8 owners are retired through the historical 5.35 s delayed startup window;
- adds a separate weapon presentation pass for restored Stein payloads that runtime proves visually blank despite non-null materials; AK authored presentation is explicitly preserved;
- extends the full runtime test to require Pass 37 visible Museum, closer BASE, weapon-palette evidence, and the existing >=30 FPS floor;
- records the repeated runtime failure in the work ledger and a lightweight runtime audit report.

This PR does not claim exact authored texture restoration where source material/texture payloads are absent. Runtime palette recovery is an explicit fallback presentation, not a substitute for later exact content restoration.

Status: CODED_UNTESTED pending local UE 5.8 build/runtime acceptance.