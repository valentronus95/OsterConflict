# Oster Conflict — runtime playtest audit — 2026-08-23 — Pass 37

Status: **runtime regression evidence; Pass 37 is CODED_UNTESTED until the next UE 5.8 playtest**.

## User runtime evidence

The first user playtest after Pass 36 reached gameplay but still rejected the presentation:

- the deployed player is standing beside the physical 11-weapon BASE rack in a flat/open field;
- the Museum is not visibly present in the initial view, so the BASE still reads as far away even though source coordinates are tied to `MuseumAnchor()`;
- most recovered rack weapon meshes remain white/grey while the equipped AK-47 retains a textured/authored appearance;
- the current screenshot shows `FPS 27`, below the existing 30 FPS acceptance target;
- the user explicitly reports that the visual presentation has regressed.

Runtime evidence overrides the previous green Pass 35/36 source CI. Neither pass is considered visual-runtime verified by this run.

## Concrete source findings

1. Pass 35 accepted Museum presence from `R137_MuseumPhotoModel` / `R138_MuseumHighFidelityArchitecture` actor counts only. It did not prove that registered visible `MuseumStructural` components actually existed near `MuseumAnchor()`.
2. R13.8 still has its own delayed startup at 5.35 s, so an early Pass 35 recovery can later coexist with another R13.8 owner unless runtime explicitly stabilizes ownership through that window.
3. The Pass 30 primary BASE was approximately 41 m from the Museum. This is collision-safe but the user has now rejected that distance twice as visually too far.
4. Pass 36 treated only null/Engine `DefaultMaterial` as broken. The restored Stein directories contain mesh/WPN payloads but no standalone authored material/texture payload beside the meshes, and runtime proves that some non-null assignments still render visually blank.

## Pass 37 correction contract

- primary BASE moves to the front-side approach at approximately 27.8 m from `MuseumAnchor`; secondary remains farther out;
- BASE yaw faces the Museum directly;
- deployment guard accepts the new 20–45 m exterior approach instead of silently preferring a farther secondary BASE;
- Museum runtime acceptance counts visible registered `MuseumStructural` components near the anchor, rebuilds an empty/stale R13.8 owner, and retires duplicate architecture owners through the delayed startup window;
- known incomplete restored Stein weapon payloads receive a deterministic dark-metal / wood / polymer presentation palette; the already-correct AK authored appearance is preserved;
- the full runtime test requires Pass 37 Museum visibility, closer BASE deployment, weapon-palette evidence and the unchanged >=30 FPS floor.

No claim is made that a runtime palette is the same thing as restoring missing exact authored textures. Exact source texture/material payloads remain a separate content-fidelity requirement where they are absent.
