# Oster Conflict — Runtime Playtest Audit — 2026-08-21 17:44

## Evidence

Authoritative evidence for this audit:
- user runtime description from the 2026-08-21 normal-game playtest;
- `Oster-photo-bag.docx` supplied by the user after the run;
- 13 screenshots in that document;
- previous 2026-08-21 runtime/legacy audits for comparison.

Runtime evidence overrides optimistic source-only status.

## What is now proven to work at least partially

1. UE 5.8 build reaches gameplay, so the previous tactical-map `C4458` compile blocker is no longer active.
2. The previous immediate `Pure virtual function being called` weapon-fallback crash did not recur during this long run.
3. Normal frontend main menu is visible again.
4. AK-47 has a real first-person visual.
5. Reload presentation exists, although still basic.
6. Shot smoke/muzzle presentation exists.
7. A real full-body character model is visible in runtime for at least one profile.
8. Silpo has an interior blockout/detail pass.
9. Vehicles are present and can be observed/driven.

None of the above implies overall gameplay/location verification.

## New/current runtime defects

### PT-01 — Splash → black gap → main menu
The splash appears, then a full black interval, then the main menu. The frontend transition must be continuous. A black startup gap is not accepted as final presentation.

### PT-02 — START transition / double-START confusion
After pressing START, the screen/background becomes gray and the deployment flow later presents another `СТАРТ` button. The user perceives this as having to start twice. Loading and deployment must be visually distinct and the final deployment action must not reuse the main-menu START meaning.

### PT-03 — Player BASE spawn is still an empty field
Normal gameplay places the player in a broad flat field. This is not an acceptable final base/spawn. The historical weapon-rack requirement is `weapons beside the actual/current player spawn`; no specific landmark was previously locked as the spawn location. A real base/spawn site must now be chosen from the canonical map and diagnostics must follow it.

### PT-04 — Weapon visuals remain mixed real/proxy
AK-47 is real. M1911/world weapon presentation still shows crude primitive/proxy geometry in the screenshots. Existing fallback code is not an acceptable production visual solution.

### PT-05 — Chat panel is permanently visible
Large `КАНАЛ: КОМАНДА` panel is permanently visible during gameplay. Required contract:
- hidden when not typing;
- `Y` opens Team chat;
- `U` opens Global chat;
- closing/submitting hides the input panel again.

### PT-06 — Distant world flicker
The user reports visible flicker at distance. First suspects to eliminate are duplicated/overlapping geometry and late replacement layers, then LOD/material issues.

### PT-07 — Grass coverage is sparse/clumped
Grass exists only as isolated clumps on a mostly flat green plane. Required result is believable landscape/foliage coverage tied to terrain and settlement surfaces.

### PT-08 — BTR remains a visible block proxy
Runtime screenshot still shows a green blocky BTR. The previous expected `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus` path was not present. Re-inventory all Content for the actual imported BTR asset before adding any more proxy geometry.

### PT-09 — Pickup/HMMWV gun is not an M2 Browning
The base vehicle is visible, but the gun mount contains large black/blue primitive shapes. Required result is the imported/real M2 Browning model, with proxy/collision geometry hidden in gameplay.

### PT-10 — Vehicle speed requirements not met
Required runtime caps:
- pickup/HMMWV: 120 km/h;
- BTR: 90 km/h.
User reports pickup remains around 30 km/h.

### PT-11 — Accidental debugger/spectator view is not a finished free-fly mode
The user accidentally invoked a view showing the character from outside and gameplay-debugger text. This is useful evidence that UE debug/spectator tooling is reachable, but it is not a finished user/dev flight mode. A deliberate, labeled free-fly diagnostic mode may be added later without debugger clutter.

### PT-12 — Museum / Silpo / Culture House still violate separation
The user still sees the three landmark scopes visually mixed/nested. Source exclusion cleanup did not solve authoritative ownership. Required result:
- Museum = one owner + museum geo site;
- Silpo = one owner + Silpo geo site;
- Culture House = one owner + Hranovskoho 3 geo site;
- no late subsystem may rebuild another landmark inside those protected sites.

### PT-13 — Silpo exterior regressed
Silpo interior exists, but exterior presentation is much poorer than earlier work. User specifically recalls parking, main signage, banners and recognizable `Сільпо` identity. These elements must be restored from the existing Silpo TZ/source/reference history, not reinvented as generic boxes.

### PT-14 — Stadium is still blockout and incorrectly oriented
Runtime stadium scope is box-like and placement/orientation is not acceptable. Georeference must include more than a coordinate:
- footprint;
- front/door direction;
- relation to road/forest;
- local elevation/relief;
- photo-consistent neighboring elements.

### PT-15 — Terrain remains unrealistically flat
The world is visibly dominated by flat planes. Terrain work must include actual elevation changes and location-specific relief, especially around the museum/rear slope and other documented sites.

### PT-16 — Visible box houses/buildings dominate the map
Overview screenshots show many white/gray rectangular blockout buildings. Imported/canonical house assets must replace visible blockout where they exist. Collision-only primitives may remain only if hidden.

### PT-17 — Silpo interior is a positive but unfinished
The interior shelves/counter layout is visible. Keep this work while replacing the exterior/blockout and preventing unrelated location owners from overwriting it.

## Screenshot manifest from Oster-photo-bag.docx

The document contains 13 playtest screenshots covering:
1. pickup/HMMWV with oversized proxy gun geometry;
2. blockout/landmark overview;
3. broad flat settlement overview with box buildings;
4. green blocky BTR;
5. additional world/vehicle view;
6. Silpo interior shelves/counter;
7. pickup driving/runtime view;
8. pickup/HMMWV gun proxy close view;
9. BTR proxy from another angle;
10. character + vehicle world view;
11. full-body character close view;
12. first-person AK in flat field;
13. additional first-person flat-field view.

The screenshots are regression evidence. Do not treat visible proxy geometry in them as accepted production content.

## Immediate repair order

1. Chat visibility + Y/U bindings.
2. Vehicle speed tuning.
3. Actual Content inventory for BTR/M2/exact weapon assets and wire real assets.
4. Normal spawn/base and test weapon contract.
5. Disable confirmed late/legacy landmark owners instead of post-spawn cleanup-only behavior.
6. Restore Silpo exterior identity.
7. Stadium footprint/orientation/relief.
8. Replace visible house/blockout boxes with real assets.
9. Terrain + grass coverage.
10. Re-test distant flicker after overlap removal.
11. Fix splash/black/loading/deployment UX.

## Acceptance rule

No affected item becomes `VERIFIED` until a fresh UE 5.8 current-main playtest demonstrates the runtime result. No new decorative R15/R16 work is authorized while this backlog remains open.
