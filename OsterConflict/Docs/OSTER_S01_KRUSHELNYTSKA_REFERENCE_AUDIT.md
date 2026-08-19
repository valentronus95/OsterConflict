# OSTER S01 — КРУШЕЛЬНИЦЬКА: REFERENCE ALIGNMENT AUDIT

Status: REFERENCE EVIDENCE LOCKED / S01 SLICE IDENTIFIED / CENTERLINE NOT YET AUTHORED

## Why this exists

The retained R13 blockout represented Solomii Krushelnytskoi as an almost straight north-south corridor. Public Oster-specific address evidence shows a substantial east/north-east progression instead.

Address points are useful evidence for the street corridor, but they are **not road-center coordinates**. This audit therefore separates public evidence from the later authored carriageway centerline.

## Source identity

The official Oster College site states the College address as:

`17044, Chernihiv region, Oster, vul. Solomii Krushelnytskoi, 7A`.

The public Visicom Oster street/address index identifies the same street as `vul. Solomii Krushelnytskoi (8-ho Bereznia)` and provides Oster-specific address markers along its extent.

The Visicom whole-street object is also retained separately as B-confidence macro evidence:

- label/object center: `50.951601785552164, 30.883556648533790`;
- south-west extent: `50.947336834596960, 30.874850176800106`;
- north-east extent: `50.958347034213716, 30.886361188850810`.

That center is metadata for the mapped street object, **not** a point to place asphalt on. The bbox is also metadata rather than a surveyed envelope: the independently geocoded address 98 sits about `15.6 m` east of its returned east edge. The verifier therefore allows only a small `25 m` metadata tolerance instead of treating the bbox as physical truth.

## B-confidence address evidence

Ordered approximately south-to-north:

| ID | Address | Latitude | Longitude | Approx project local X cm | Approx project local Y cm |
|---|---:|---:|---:|---:|---:|
| S01_KR_REF_08 | 8 | 50.94759774583321 | 30.876405160556917 | -52319 | -7138 |
| S01_KR_REF_14 | 14 | 50.94843423662540 | 30.878767729754150 | -35749 | 2173 |
| S01_KR_REF_7A_COLLEGE | 7A | 50.949214117728445 | 30.879129750813650 | -33210 | 10855 |
| S01_KR_REF_28 | 28 | 50.94932787287711 | 30.881081789926040 | -19520 | 12121 |
| S01_KR_REF_40 | 40 | 50.95038900824071 | 30.882703249013880 | -8148 | 23934 |
| S01_KR_REF_42 | 42 | 50.95059613903775 | 30.882914353105647 | -6667 | 26240 |
| S01_KR_REF_74 | 74 | 50.953855214938635 | 30.885876996912668 | 14111 | 62520 |
| S01_KR_REF_78 | 78 | 50.95445505467416 | 30.885954252027110 | 14653 | 69197 |
| S01_KR_REF_98 | 98 | 50.957596730285466 | 30.886583072725990 | 19063 | 104170 |

Local values are explanatory conversions through the same tangent-plane approximation used by `FOCGeoReference`; the permanent verifier recomputes the important relationships instead of trusting rounded documentation values.

## What the evidence proves

- the College/7A public marker agrees closely with the canonical College anchor;
- from 7A to 42, the address evidence moves roughly 265 m east and 154 m north;
- from 42 to 78, it continues roughly 213 m east and 430 m north;
- therefore one near-vertical line around the old blockout X position cannot be treated as a factual model of the complete street.

This is enough to reject the old macro alignment as factual. It is **not** enough to place the final asphalt centerline through the address markers.

## Relationship to S01

The current S01 workflow rectangle is approximately:

- X `-68135 .. -18230 cm`;
- Y `1497 .. 53916 cm`.

The public evidence makes the intended ownership slice much clearer:

- address 8 is south of S01;
- addresses 14, 7A and 28 are inside S01;
- address 28 is only about `12.9 m` inside the current east workflow edge;
- address 40 is already about `100.8 m` east of that edge;
- address 42 is farther east again.

Therefore S01 should own the **middle College-side slice** of Krushelnytska, not pretend the whole street continues north inside the rectangle. The actual road centerline exit point still needs direct road-shape evidence, so no exact crossing coordinate is invented here.

This is a planning correction, not a cadastral statement. The continuation after the east bend belongs to adjacent-sector/shared-road work once its road shape is referenced.

## Review-only authoring gates

`FOCLocationSectorS01KrushelnytskaAuthoringData` defines two confidence-C uncertainty windows for later centerline authoring:

- `S01_KR_GATE_SOUTH_ENTRY` centered near `(-36952, 1497) cm`, derived from the address-evidence progression 8 → 14 intersecting the S01 south boundary, with ±75 m lateral uncertainty;
- `S01_KR_GATE_EAST_EXIT` centered near `(-18230, 13462) cm`, derived from the address-evidence progression 28 → 40 intersecting the S01 east boundary, with ±80 m longitudinal uncertainty.

These are **not road points**. They are wide review gates saying, roughly, “a credible future S01 centerline should enter/exit through this evidence-supported boundary region.” Their centers are deterministically recomputed by `VERIFY_R13_LOCATION_FIRST_S01_KRUSHELNYTSKA_GATES.py`, and `OCWorldSectorOster.cpp` is forbidden from consuming them.

## College transition region

`S01_KR_REGION_COLLEGE_TRANSITION` is a second review-only constraint. It is built from the envelope of addresses 14, 7A and 28, padded by `40 m`, then clipped to S01.

Approximate local values:

- center `(-28989, 8809) cm`;
- half-size `(10760, 7312) cm`.

The region deliberately touches the S01 south and east edges and contains both authoring gates. It does **not** describe a road footprint. Its job is to constrain the future College-side centerline to an evidence-supported transition area without inventing an exact bend point.

The verifier recomputes the region from the B-confidence source points and S01 margins, checks the 40 m padding/clipping, confirms addresses 14/7A/28 and both gates remain inside it, and forbids runtime use.

## Reference-conflicted runtime geometry

The current playtest still renders the migrated Krushelnytska spine because replacing it partially would create continuity/collision regressions. However newer reference evidence now explicitly conflicts with its near-vertical alignment.

The following runtime pieces are therefore recorded by `ReferenceConflictedRuntimeSegments()` and hard-limited to confidence C:

- `S01_KR_SPINE_SOUTH_SHARED`;
- `S01_KR_SPINE_INSIDE`;
- `S01_KR_SPINE_NORTH_SHARED`.

`VERIFY_R13_LOCATION_FIRST_S01_KRUSHELNYTSKA_GATES.py` checks that the shared `Provisional` road confidence alias remains exactly `EOCReferenceConfidence::C` and that all three conflicted road records actually use it. A tidy registry is no longer enough to accidentally promote these segments to “verified” geography.

Replacement must be atomic across the affected S01/shared continuity once a reviewed carriageway skeleton exists.

## Runtime safety rule

These sources are evidence/authoring-only:

- `FOCLocationSectorS01ReferenceData::KrushelnytskaAddressReferences()`;
- `FOCLocationSectorS01ReferenceData::KrushelnytskaStreetExtentReference()`;
- `FOCLocationSectorS01KrushelnytskaAuthoringData::ReviewOnlyCenterlineGates()`;
- `FOCLocationSectorS01KrushelnytskaAuthoringData::CollegeTransitionRegion()`;
- `FOCLocationSectorS01KrushelnytskaAuthoringData::ReferenceConflictedRuntimeSegments()`.

`OCWorldSectorOster.cpp` must not consume any of them directly.

The next runtime correction needs a separately authored B-confidence centerline/skeleton derived from:

1. the public street/address progression;
2. road shape visible in map/satellite/photo references;
3. College access/junction relationship;
4. S01 entry/exit ownership boundaries;
5. explicit width/profile assumptions documented per segment.

Only after that skeleton passes a no-discontinuity, sector-entry/exit and landmark-clearance verifier should the current C-confidence Krushelnytska blockout be replaced.
