# LOC_MUSEUM_001_OSTER_MUSEUM — PASS45 normative reference specification

Status: **NORMATIVE REFERENCE APPENDIX FOR `PASS45_RUNTIME_RECOVERY_TZ.md`**  
Scope: Oster Local History Museum / Solonyna house, museum grounds, front approach and adjacent football-field context  
Runtime acceptance: **NOT YET ACCEPTED**  
Evidence authority: user-supplied photo pack + documented public grounding sources  
Target: Unreal Engine 5.8.x

## 0. Why this file exists

`PASS45_RUNTIME_RECOVERY_TZ.md` remains the canonical active recovery TZ. This file is a subordinate, versioned visual-reference contract for Gate K and landmark acceptance.

Do not copy the entire reference narrative into the main TZ. The main TZ should carry the task/gate/status; this file carries the detailed visual truth needed to build and judge the location.

The original `/mnt/data/...` paths from the ChatGPT working environment are non-portable and must never be used by build scripts or treated as repository paths. The stable identifier is:

`LOC_MUSEUM_001_OSTER_MUSEUM`

## 1. Authority rules

For this location, authority order is:

1. latest explicit user correction and latest factual UE 5.8 runtime screenshot;
2. supplied museum photo pack;
3. this reference specification;
4. public sources used only for naming/address/geographic grounding;
5. current source implementation;
6. generic asset-pack assumptions and historical proxy art.

A green source test cannot overrule a visibly wrong museum silhouette, site layout, material identity, vegetation composition or stadium relation.

Every modeling decision must be treated as one of:

- `VERIFIED` — clearly visible and repeat-confirmed by supplied references;
- `PROBABLE` — strongly implied but not fully visible;
- `UNKNOWN` — insufficient evidence. Do not fabricate fine detail and later call it exact.

## 2. Grounding

Object: **Остерський краєзнавчий музей / будинок Солонини**.  
Public address used by the supplied reference brief: **вул. Татарівська, 30, м. Остер, Чернігівська область, Україна**.

Public grounding sources carried by the reference brief:

- `https://chernihivregion.travel/places/osterskij-kraeznavcij-muzej`
- `https://chernihivregion.travel/places/budinok-general-lejtenanta-v-k-solonini`
- `https://travels.in.ua/uk/object/961/osterskyy-krayeznavchyy-muzey`

These sources ground identity/location only. Building geometry is controlled primarily by the user photo pack.

## 3. Hard landmark ownership and separation

The Museum, Culture House and Silpo are different places and different runtime/site owners.

Mandatory:

- Museum may not inherit the Culture House six-column civic facade.
- Culture House geometry may not be spawned inside, fused with, or overlap the Museum hero mass.
- Silpo geometry/signage may not contaminate the Museum site.
- Any regression that visually merges these landmarks is a Gate K failure even if coordinates/source guards are green.

## 4. Museum macro silhouette — VERIFIED anchors

The museum must read as a historic elongated residential/estate-type building adapted for museum use, not as a generic civic block.

Required silhouette:

- long predominantly **one-storey** brick main volume;
- attic / half-storey upper elements rather than a full modern second floor;
- warm red/orange to red-brown brick lower mass;
- continuous dark charcoal/near-black raised plinth;
- centered front entrance projection/porch;
- raised central timber-clad upper front volume integrated into the roof mass;
- compound sloped metal roof with smaller gabled volumes;
- secondary lower annex/service extension kept visually subordinate.

Fail conditions:

- full generic two-storey rectangular civic block;
- missing dark plinth;
- missing central upper timber projection;
- generic plaster-house replacement;
- a roof silhouette dominated by invented dormers/skylights/chimneys unsupported by evidence.

## 5. Front entrance / porch — VERIFIED anchors

The canonical front is the side with the central porch, stairs, double entrance door, raised upper timber volume and spruce-lined approach.

Required:

- entrance projection centered on the front facade;
- small gabled porch roof;
- grey-painted door/carpentry language;
- glazed or semi-glazed porch side sections;
- broad stair flight directly aligned to the main approach path;
- approximately 6–7 visible risers as a visual guide, not survey geometry;
- grey ornamental but restrained metal railings;
- side glazed/veranda-like continuation visible from oblique views.

The front approach axis must terminate visually at this stair/door composition.

## 6. Upper front timber projection — VERIFIED recognition feature

This is a non-negotiable identity feature.

Required:

- muted grey-blue / desaturated blue-grey timber cladding;
- three front-facing vertically proportioned windows;
- pale beige/cream trim;
- triangular gable and decorative timber treatment;
- integration into the roof mass rather than an unrelated box placed on top.

## 7. Brick facade and decorative articulation

Required material/detail identity:

- warm aged orange-red/red-brown brick;
- readable mortar joints;
- moderate age/weathering, not ruin treatment;
- decorative brick cornice/frieze near the top of the walls;
- projecting/dentil-like brick rhythm;
- repeated pale square inset elements;
- shallow pilaster-like facade rhythm between window bays;
- dark raised plinth with small basement/vent openings.

The repeating light square insets are geometry/material identity, not vague normal-map noise.

## 8. Window family

Required:

- tall vertically proportioned ground-floor windows;
- white/off-white frames;
- deeper window reveals than a flat decal/window texture;
- decorative upper grille/fanlight logic;
- diagonal/lattice grille motifs where supported by the photos;
- grey projecting sills;
- three slimmer windows on the upper front projection;
- long-side facade must preserve a believable repeated window rhythm rather than collapsing to one or two generic openings.

Per-window curtain/reflection/wear variation is allowed as secondary art variation if it does not alter verified geometry.

## 9. Roof

Required:

- weathered grey sheet-metal / standing-seam reading;
- visible longitudinal seam logic;
- tonal patching/repair variation;
- slight age/waviness/dirt variation;
- compound sloped roof system consistent with visible photo angles.

UNKNOWN / conservative areas:

- exact hidden rear roof slopes;
- exact hidden gutter/downpipe routing;
- hidden flashing details.

Do not invent dramatic roof features to fill missing evidence.

## 10. Museum site composition

The site is a quiet, park-like clearing, not a dense urban plaza.

Macro order from the entry side:

1. road / outer edge;
2. low white museum sign and entry edge;
3. straight pedestrian approach;
4. wide open grass;
5. tall spruce framing/allée;
6. museum centered beyond the tree corridor;
7. adjacent football-field parcel on the **left when facing the museum entrance**;
8. additional open tree cover/park land around and behind the museum.

This relationship is a hard layout rule for acceptance.

## 11. Front approach path

Required:

- straight and central;
- pedestrian scale, not a vehicle road;
- aligned directly to the main entrance stairs;
- pale grey concrete slab / continuous concrete segments;
- visible joints;
- aged but serviceable;
- restrained cracks, dirt staining and grass encroachment;
- readable as a long pale line through the spruce corridor from distance.

The iconic path must not be silently replaced by a generic wide asphalt road or decorative curved park path.

## 12. Vegetation

### Front allée

Required:

- mature tall spruce/conifer trees dominate the main approach;
- slender, high-trunked vertical reading;
- spacing irregular enough to feel natural but organized enough to frame the path;
- the main axis must not be replaced by generic broadleaf park trees.

### Secondary vegetation

Allowed/expected where supported:

- deciduous trees including birch and other broadleaf species;
- occasional leaning/non-perfect trunks;
- sparse low shrubs;
- small plant/flower zones near side-yard corners;
- open lawn remains visually dominant.

### Ground under trees

Required variation:

- darker shaded under-tree zones;
- leaf/needle accumulation;
- patchy grass;
- compacted-earth transitions near the main spruce allée.

## 13. Ground/lawn

Required:

- relatively flat site;
- museum slightly elevated visually by plinth/stairs, not by a large terrain mound;
- natural non-ornamental grass;
- mixed density and uneven color;
- occasional dry zones;
- no single uniformly saturated green carpet material.

A seasonal material system may vary lush/dry/autumn states, but the baseline map should remain coherent and not randomize geometry between network clients.

## 14. Roadside sign / edge details

Reference-supported:

- low white concrete/stone-like sign;
- blue inset name panel reading `Остерський краєзнавчий музей`;
- visibly aged/cracked surface;
- two rectangular concrete planters in front;
- light/simple fence and gate context;
- small playground/swing element may appear as secondary detail if within the playable crop.

Do not turn the edge into a heavy security perimeter.

## 15. Adjacent football field / stadium context

For the museum reference pack, the visible adjacent field must read as a **local community football ground**, not automatically as a modern stadium bowl.

Hard relation:

- when facing the museum entrance from the front approach, the football field is on player-left.

Reference-supported character:

- open grass field;
- multiple football goal structures;
- utility poles and overhead wires in background context;
- informal field edge;
- sparse/simple sports fixtures.

Do not add large bleachers, a modern arena bowl or heavy stadium architecture unless a separate location-specific reference proves that those features belong to the exact adjacent field represented in this scene.

If the project also models a separately documented modernized Oster central stadium, keep that as a distinct confidence/identity contract rather than allowing it to overwrite this museum-adjacent reference relationship.

## 16. UE5 asset decomposition

### Unique hero assets

Prefer custom/high-fidelity authored assets for:

- main museum shell;
- central porch;
- upper front timber projection;
- gable-side door canopy;
- side annex;
- museum sign;
- window/grille family;
- decorative brick trim/inset family;
- stairs and ornamental railings.

### Reusable modular assets

May be modularized:

- brick wall segments;
- plinth segments;
- roof sheet modules;
- gutters;
- concrete path modules;
- simple fence fragments;
- football goals;
- ground foliage clusters.

## 17. Minimum material set

At minimum the final authored museum/site chain needs distinct material identity for:

1. aged warm red/orange brick;
2. dark plinth;
3. weathered standing-seam/sheet-metal roof;
4. grey painted porch wood/carpentry;
5. grey-blue upper timber cladding;
6. white/off-white window frames;
7. aged/cracked pale concrete path/sign/planter surfaces;
8. mixed grass/soil/needle/leaf ground layers.

No white/default engine material may remain on the accepted hero model.

## 18. Screenshot-match acceptance cameras

Gate K must include direct UE 5.8 screenshot comparison against the source pack from at least these views:

- `MUS-CAM-01` — frontal centered entrance + stairs;
- `MUS-CAM-02` — front oblique showing porch + upper timber projection;
- `MUS-CAM-03` — long front-path shot through the spruce corridor;
- `MUS-CAM-04` — long side elevation wide shot;
- `MUS-CAM-05` — side-detail shot with plaque/window rhythm;
- `MUS-CAM-06` — roadside museum sign/context;
- `MUS-CAM-07` — football-field relation from museum-side context.

A build fails visual acceptance if these views materially fail in:

- silhouette;
- relative placement;
- landmark separation;
- material identity;
- path/entrance alignment;
- tree composition;
- stadium/field side relation.

Source tests alone cannot close this gate.

## 19. Unknowns and safe fallback rules

Known weak/unknown coverage includes:

- exact hidden rear facade window count;
- exact hidden roof/gutter details;
- full service/utility fittings on hidden elevations;
- full interior room partition plan;
- exact property perimeter;
- exact front paving module dimensions;
- exact annex dimensions/rear junction details.

Fallback rules:

- continue visible historic language conservatively;
- preserve visible rhythm rather than inventing showpiece detail;
- mark provisional geometry as provisional in production notes;
- never promote inferred hidden geometry to `VERIFIED` without new evidence.

## 20. PASS45 implementation consequences

This reference pack raises the following Pass45 work from generic visual cleanup to explicit acceptance work:

- Museum massing must be audited against the one-storey historic-house silhouette.
- Museum/Culture House overlap is a hard failure, not minor clipping.
- Front approach ownership must preserve a straight concrete pedestrian axis to the entrance.
- Main-axis vegetation must prioritize mature spruce/conifer composition.
- Museum lawn/ground must stop reading as uniform prototype turf.
- Adjacent field/stadium representation must be checked against the supplied museum-side photos before retaining modernized-track/stand assumptions.
- Decorative brick belt, pale square insets, window family, plinth and upper timber projection must survive any authored-world replacement pass.
- Gate K cannot be marked accepted until `MUS-CAM-01..07` direct screenshots pass.

## 21. Non-scope

This museum pack is not sufficient by itself to define all roads, private houses, Culture House architecture, Silpo architecture, central park architecture or the entire city.

Those should use separate location/city reference packs so evidence does not bleed between unrelated sites. In particular, generic Oster private-sector house/fence rules may inform surrounding background art, but they may not rewrite the verified Museum hero geometry.
