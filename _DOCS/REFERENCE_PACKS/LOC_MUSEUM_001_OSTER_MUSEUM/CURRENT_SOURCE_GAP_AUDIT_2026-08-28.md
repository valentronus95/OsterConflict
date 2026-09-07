# LOC_MUSEUM_001 — CURRENT SOURCE GAP AUDIT — 2026-08-28

Parent contract: `_DOCS/REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/REFERENCE_SPEC.md`  
Pass45 binding: `PASS45_REFERENCE_PACK_BINDINGS.md`  
Runtime truth: **RUNTIME REJECTED 2026-08-27**  
Scope: source-to-reference reconciliation only; no runtime acceptance claimed.

## 1. Audit conclusion

Current Oster source already contains a dedicated Museum anchor and nontrivial Museum proxy construction, but the new photo-grounded reference pack is materially stricter than the old source blockout.

The existing source therefore must be treated as **PROXY / PARTIALLY REFERENCE-INFORMED**, not as an accepted high-fidelity Museum model.

## 2. What current source gets directionally right

`AOCWorldSectorOster::BuildMuseumAndStadium()` already attempts several correct identity cues:

- red-brick predominantly single-storey massing;
- central raised timber/gable feature;
- front porch/glazed projection;
- front steps;
- gabled roof pieces;
- repeated facade windows;
- separate Museum geo anchor;
- separate Stadium anchor.

These are useful source anchors and should be refined rather than blindly discarded.

## 3. Museum source gaps against the normative pack

### MUSEUM-GAP-01 — hero silhouette remains box/proxy-driven

The runtime source is assembled from `AddBox()` and `AddGableRoof()` proxy primitives. That can encode rough layout but cannot by itself satisfy Gate K hero fidelity.

Required closure:

- authored main shell / facade depth;
- correct one-storey historic-estate reading;
- integrated central upper timber projection;
- subordinate annex;
- compound weathered metal roof;
- no visual drift toward a generic civic block.

### MUSEUM-GAP-02 — decorative brick identity is under-specified

The reference pack makes the decorative brick frieze/dentil rhythm and repeated pale square insets a recognition feature.

Required closure:

- explicit authored geometry/material treatment;
- retain it through any runtime replacement/upgrade pass;
- do not reduce it to generic flat brick texture noise.

### MUSEUM-GAP-03 — window family needs authored identity

Current proxy window blocks establish rough rhythm only.

Required closure:

- deep-set white/off-white window family;
- upper fanlight/sunburst grille logic where verified;
- diagonal/lattice grille treatment where verified;
- grey sills;
- three slimmer upper-front windows;
- long-side window rhythm preserved.

### MUSEUM-GAP-04 — plinth/vent identity needs explicit authored treatment

Reference requires a significant dark charcoal/near-black plinth with low openings/vents.

Required closure:

- authored plinth material/geometry;
- visible height above grade;
- no terrain/material pass may wash it into the wall or lawn.

### MUSEUM-GAP-05 — porch/stair/railing fidelity

Current source has porch/step proxy geometry but does not prove the required grey carpentry, glazed side treatment, approximately 6–7-riser visual scale or restrained ornamental railings.

Required closure:

- authored porch family;
- direct alignment with the front path;
- `MUS-CAM-01` and `MUS-CAM-02` acceptance.

## 4. Site/ground gaps

### MUSEUM-GAP-06 — front path must become an explicit hero-axis contract

Reference requires a straight pale concrete pedestrian approach aligned to the entrance stairs through the spruce corridor.

Required closure:

- dedicated semantic ownership for the Museum approach rather than accidental generic sidewalk ownership;
- concrete slab/joint reading;
- aged serviceable surface, restrained cracking/dirt/grass encroachment;
- exact visual alignment to the entrance.

This is separate from the five Central Park `ParkPaths` proxies. Do not merge unrelated path families because both happen to be pedestrian paths.

### MUSEUM-GAP-07 — spruce allée composition

The main Museum axis specifically requires mature tall spruce/conifer framing.

Required closure:

- reserve the main axis for conifer composition;
- preserve irregular-but-framing spacing;
- no generic broadleaf replacement on the hero axis;
- secondary deciduous/birch remains allowed off-axis.

### MUSEUM-GAP-08 — ground cannot read as uniform prototype turf

Required closure:

- mixed grass density/color;
- occasional dry zones;
- darker under-tree treatment;
- needle/leaf accumulation;
- patchy grass/compacted earth near the allée;
- keep terrain broadly flat.

## 5. Landmark separation gap

### MUSEUM-GAP-09 — visual separation must be accepted, not merely source-guarded

Existing source guards distinguish Museum/Culture House/Silpo owners, but the latest runtime evidence has already shown that source ownership checks are not enough when visible landmark geometry overlaps or reads incorrectly.

Required closure:

- direct Museum/Culture House/Silpo screenshots;
- zero physical fusion/overlap of hero masses;
- Museum cannot inherit six-column Culture House identity;
- Museum cannot inherit Silpo facade/signage.

## 6. Stadium / field conflict requiring explicit reconciliation

### MUSEUM-GAP-10 — current modernized stadium assumptions conflict with the Museum-side pack

Current `BuildMuseumAndStadium()` source describes and constructs a modernized stadium-style treatment including:

- artificial-turf/rectangular pitch assumptions;
- track/apron proxies;
- spectator stand blocks;
- service/changing-room blocks;
- perimeter fencing.

The new Museum photo pack, specifically for the field adjacent to the Museum scene, requires a simpler local-community football-ground reading:

- open grassy field;
- multiple goal structures;
- utility poles/overhead wires in background;
- informal edge;
- no large bleachers or modern arena treatment unless separately proven for this exact adjacent field.

This must not be solved by averaging both into one fictional stadium.

Required decision in source:

1. determine whether the existing modernized `StadiumAnchor()` represents the same exact Museum-adjacent field or a separately documented sports facility/state;
2. if separate, split ownership/identity so both contracts can coexist without contamination;
3. if it is the same field, the new user photo pack takes visual priority for the Museum-context acceptance cameras unless newer factual evidence proves a dated reconstruction/state change relevant to the game’s chosen time state;
4. `MUS-CAM-07` must prove the final relation and field identity.

Until this is reconciled, Stadium/Museum context is **SOURCE CONFLICT OPEN / RUNTIME UNACCEPTED**.

## 7. Required execution slices

### MUSEUM-R1 — source ownership + semantic split

- audit exact Museum component/runtime owners;
- separate Museum approach from generic sidewalk/park-path families;
- resolve Museum-adjacent field vs modern stadium ownership;
- add source regression guards for landmark evidence bleed.

### MUSEUM-R2 — authored hero architecture

- main shell;
- central porch;
- upper timber projection;
- annex;
- roof;
- plinth;
- window/grille family;
- decorative brick frieze/insets;
- stairs/railings;
- material chain.

### MUSEUM-R3 — grounds and approach

- straight concrete approach;
- spruce allée;
- mixed lawn/soil/needle/leaf ground treatment;
- sign/planters/light fence;
- supporting utility poles/field context where reference-supported.

### MUSEUM-R4 — direct runtime acceptance

Capture and compare:

- `MUS-CAM-01` frontal entrance;
- `MUS-CAM-02` front oblique;
- `MUS-CAM-03` long spruce approach;
- `MUS-CAM-04` long side;
- `MUS-CAM-05` side detail;
- `MUS-CAM-06` roadside sign;
- `MUS-CAM-07` adjacent football-field relation.

No source verifier may mark MUSEUM-R4 complete.

## 8. Current status

- normative Museum reference: **BOUND**;
- current source proxy: **PARTIALLY REFERENCE-INFORMED / NOT HIGH-FIDELITY ACCEPTED**;
- Museum/Culture/Silpo source ownership: **SOURCE-GUARDED / RUNTIME VISUAL ACCEPTANCE OPEN**;
- Museum-adjacent field vs current modern stadium proxy: **CONFLICT OPEN**;
- final Gate K Museum acceptance: **OPEN**.
