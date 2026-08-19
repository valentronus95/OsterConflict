# OSTER S01 — КРУШЕЛЬНИЦЬКА: REFERENCE ALIGNMENT AUDIT

Status: REFERENCE EVIDENCE LOCKED / CENTERLINE NOT YET AUTHORED

## Why this exists

The retained R13 blockout represented Solomii Krushelnytskoi as an almost straight north-south corridor. Public Oster-specific address evidence shows a substantial east/north-east progression instead.

Address points are useful evidence for the street corridor, but they are **not road-center coordinates**. This audit therefore separates public evidence from the later authored carriageway centerline.

## Source identity

The official Oster College site states the College address as:

`17044, Chernihiv region, Oster, vul. Solomii Krushelnytskoi, 7A`.

The public Visicom Oster street/address index identifies the same street as `vul. Solomii Krushelnytskoi (8-ho Bereznia)` and provides Oster-specific address markers along its extent.

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

## Runtime safety rule

`FOCLocationSectorS01ReferenceData::KrushelnytskaAddressReferences()` is evidence-only. `OCWorldSectorOster.cpp` must not consume those address markers directly.

The next runtime correction needs a separately authored B-confidence centerline/skeleton derived from:

1. the public street/address progression;
2. road shape visible in map/satellite/photo references;
3. College access/junction relationship;
4. S01 ownership boundaries;
5. explicit width/profile assumptions documented per segment.

Only after that skeleton passes a no-discontinuity and landmark-clearance verifier should the current C-confidence Krushelnytska blockout be replaced.
