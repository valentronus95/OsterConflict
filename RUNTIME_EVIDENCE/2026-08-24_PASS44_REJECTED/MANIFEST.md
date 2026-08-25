# Pass 44 rejected runtime evidence — 2026-08-24

This pack is authoritative user-observed runtime evidence for the first local UE 5.8 gameplay run after Pass 44 was merged to `main` at `bf483f8dc473862e0d3ce6468db44f025abbeef1`.

The supplied screenshots were consolidated into one evidence sheet so the complete runtime state is retained in the repository without putting binary screenshots in `OsterConflict/Docs/WorkReports/`.

![Pass 44 rejected runtime evidence](pass44_runtime_evidence_20260824.jpg)

## Evidence index

1. **Main menu — FPS 8.** Frontend renders, but performance is already catastrophically low before normal gameplay population or gameplay-only foliage can be the sole cause.
2. **Weapon rack / pickup area.** Several weapons render white or without intended authored materials while the held AK has a readable material. A mesh loading successfully is therefore not material readiness.
3. **Large columned civic building.** Runtime world still presents a primitive/blockout landmark shell instead of a clearly separated, photo-faithful Museum/Culture House result.
4. **Tactical map.** `M` is bounded more tightly than the historical 2.4 km map but still renders an incorrect procedural topology: oversized straight/diagonal road segments and crude block geometry rather than the real compact central-Oster street layout.
5. **Open field — FPS about 11.** Large visually empty areas remain inside the live gameplay experience despite the compact-map change.
6. **Open field — FPS about 8.** Severe slowdown persists away from dense scenery, contradicting the assumption that bot count or one dense foliage patch is the sole root cause.
7. **Primitive tree forest — FPS about 8.** Tree silhouettes are visibly unsuitable for Oster and match the source primitive Cylinder/Sphere tree families rather than tall pine / conifer forest and appropriate oak assets.
8. **Landmark overlap/identity view.** The visible columned building and surrounding layout still fail to establish distinct, correctly placed Museum and Culture House identities.
9. **Weapon rack close view.** White weapon materials remain visible on multiple rack items.

## Runtime verdict

**Pass 44 = RUNTIME REJECTED.**

Source CI green status is retained as source evidence only. It does not override these screenshots.

## Linked corrective specification

`PASS45_RUNTIME_RECOVERY_TZ.md`
