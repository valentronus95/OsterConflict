# LOC_CULTURE_003 — OSTER CULTURE HOUSE REFERENCE SPEC

Status: **BOUND PROVISIONAL REFERENCE CONTRACT / VERIFIED IDENTITY + SITE / PARTIAL VISUAL GROUNDING / UE 5.8 RUNTIME ACCEPTANCE REQUIRED**  
Parent TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Binding index: `PASS45_REFERENCE_PACK_BINDINGS.md`  
Location id: `LOC_CULTURE_003_OSTER_CULTURE_HOUSE`  
Reviewed: 2026-08-28

## 0. Purpose

This file defines the current conservative Pass45 contract for the Oster Culture House while a dedicated user photo pack is not yet available.

Unlike the Museum and Silpo packs, this document deliberately separates **verified identity/site facts** from the current project's **provisional visual hypothesis**. Existing source geometry is not promoted to photographic truth merely because a verifier already knows its component names.

Authority:

1. latest explicit user correction + latest factual local UE 5.8 screenshot/log;
2. `PASS45_RUNTIME_RECOVERY_TZ.md`;
3. this spec;
4. current implementation;
5. historical source verifiers/passes.

A later user-supplied Culture House photo pack supersedes any conflicting `PROVISIONAL` visual statement in this file.

## 1. Factual grounding

`VERIFIED` identity/site facts:

- object: **Oster Culture House / Остерський будинок культури**;
- current institutional address: **Oster, Hranovskoho Street 3**;
- current project canonical geo owner is `FOCGeoReference::CultureHouse()` and is separate from Museum and Silpo;
- public Ukrainian reference sources identify the building as a former synagogue that was substantially rebuilt/adapted in the Soviet period for Culture House use;
- public travel description places an old park directly by the Culture House.

Public grounding reviewed for this contract:

- Chernihiv regional service directory entry for `Остерський будинок культури Остерської міської ради`, address `м. Остер, вул. Грановського, 3`;
- 2025 energy-efficiency procurement for `Остерський міський будинок культури м. Остер, Грановського,3`;
- Encyclopaedia of Modern Ukraine entry for Oster, which states that a synagogue was adapted as the Culture House during the Soviet period;
- RBC Ukraine travel feature identifying the Culture House as the former synagogue and describing the old park beside it.

These sources establish identity/history/site context. They do **not** establish survey-grade dimensions or exact current facade bearing.

## 2. Hard landmark ownership

The Culture House is its own real-world landmark.

Required:

- Culture House exterior owner remains separate from Museum exterior owner;
- Culture House remains separate from Silpo shell/facade owner;
- no Museum brick/timber hero identity may be copied onto Culture House merely to reuse content;
- no Silpo sign/storefront identity may appear here;
- Culture House geometry may not spawn at the Museum anchor;
- validators may reject bad ownership but may not silently relocate/replace the building to manufacture a green result.

Hard fail:

- Culture House visible inside/intersecting Museum shell;
- Culture House six-column/current source facade instantiated at Museum site;
- Museum identity instantiated at Culture House site;
- Silpo signage at Culture House site.

## 3. Current source visual hypothesis

The current project source builds a civic facade with:

- six front columns/pillars;
- three entrance bays;
- broad single civic-building mass;
- authored modular wall/window/door/roof/foundation components;
- no Engine BasicShape structural fallback in the current authoritative source path.

For Pass45 this configuration is classified:

**PROVISIONAL WORKING HYPOTHESIS, NOT PHOTO-VERIFIED EXACT GEOMETRY.**

The six-column count is currently useful as an ownership/contamination guard because it distinguishes the Culture House source owner from the Museum. It must not be described as an externally verified architectural fact until a direct photo/reference set proves it.

## 4. Bearing and dimensions

Current source explicitly leaves facade bearing provisional.

Therefore:

- exact yaw/bearing: `PROVISIONAL`;
- current 32 m x 18.5 m source hall dimensions: `PROVISIONAL SOURCE GREYBOX`, not survey data;
- column spacing/diameter/height: `PROVISIONAL SOURCE GREYBOX`;
- exact roof profile: `PROVISIONAL` unless directly visible in accepted evidence;
- exact window/door rhythm beyond accepted screenshot matching: `PROVISIONAL`;
- hidden/rear facade: `UNKNOWN / PROVISIONAL`.

No verifier may upgrade these values to `VERIFIED` solely by matching C++ constants.

## 5. Historical identity constraint

Because the building is publicly documented as a former synagogue substantially rebuilt/adapted in the Soviet period, the accepted model should not invent an unrelated contemporary shopping/office typology.

This historical fact is **context**, not permission to reconstruct an unverified pre-Soviet synagogue facade.

Do not:

- restore speculative original synagogue ornament as if factual current-state geometry;
- add domes/towers/religious symbols without direct evidence for the selected time state;
- erase the current civic-building character in favor of an imagined historical reconstruction.

## 6. Old park relationship

Public evidence supports an old park beside the Culture House.

Required contextual intent:

- Culture House must read as connected to a mature civic/park setting rather than isolated on an empty slab;
- mature trees and ordinary paths/ground treatment should frame the building without hiding the entire hero facade;
- park context must not be confused with the separate Museum/stadium grounds;
- any Sespel memorial/monument relationship is handled by its own verified location evidence and may not be guessed into the Culture House forecourt merely because a travel article mentions the park.

Exact tree placement, path geometry and monument transforms remain `PROVISIONAL` unless separately grounded.

## 7. Materials and facade treatment

Without a dedicated accepted Culture House photo pack, exact facade color/material layering remains `PROVISIONAL`.

Rules:

- authored material slots should remain physically plausible and coherent;
- no Engine BasicShape/default material may be accepted as final hero facade;
- do not borrow Museum red-brick/blue-grey timber palette as a shortcut;
- do not borrow Silpo graphite/orange-blue retail palette;
- weathering/age should be appropriate to a long-used civic building, but exact stains/paint patches require photo evidence.

A later photo pack should replace this section with exact material zones and temporal states.

## 8. Entrance / facade composition

Until direct photos are bound:

`PROVISIONAL` source cues that may remain for iteration:

- broad civic front;
- six-column/pillar rhythm;
- three entrance bays;
- symmetrical or near-symmetrical front organization.

They are **not final acceptance facts**.

Source work must prefer reversible/modular construction so a future photo pack can correct:

- column count/spacing;
- entrance width/height;
- window rhythm;
- pediment/parapet/roof form;
- facade depth;
- material zones;
- front steps/forecourt.

## 9. UE5 ownership / modularity

Current authoritative source path uses committed authored modular content and emits `PASS45_CULTURE_HOUSE_AUTHORED_SHELL_READY` when its source conditions are met.

This marker means only:

- authoritative source owner exists;
- committed authored modular meshes resolved;
- visible BasicShape structural fallback is not used by that source path.

It does **not** mean:

- photo fidelity accepted;
- exact dimensions accepted;
- current six-column configuration historically verified;
- local UE 5.8 screenshot accepted.

The Culture House implementation should remain modular enough to replace incorrect facade parts without changing the canonical geo owner or contaminating Museum/Silpo ownership.

## 10. Required screenshot set

Until a direct Culture House photo pack defines tighter match cameras, Gate K requires at minimum:

- `CUL-CAM-01_FRONT_WIDE` — whole building and immediate site/park relationship;
- `CUL-CAM-02_FRONT_CLOSE` — entrance/facade/material readability;
- `CUL-CAM-03_OBLIQUE_LEFT` — left massing/depth and front-to-side transition;
- `CUL-CAM-04_OBLIQUE_RIGHT` — right massing/depth and front-to-side transition;
- `CUL-CAM-05_SITE_CONTEXT` — independent proof that Culture House occupies its own parcel and does not overlap Museum/Silpo.

If later user photos define exact camera analogues, those views replace or refine these provisional camera intents.

## 11. Current hard failures

Culture House Gate E/K is hard FAIL if:

- it overlaps or appears inside the Museum;
- Museum/Silpo identity contaminates it;
- it uses visible Engine BasicShape/proxy/default hero geometry/material as accepted content;
- source verifier calls the current six-column geometry `photo verified` without direct evidence;
- exact bearing/dimensions are claimed despite remaining provisional;
- building is missing from its canonical separate site;
- runtime screenshot cannot distinguish it as a separate civic landmark;
- a later user photo pack conflicts with the model and the older source hypothesis is retained anyway.

## 12. Evidence confidence summary

- identity as Oster Culture House: `VERIFIED`;
- address Hranovskoho 3: `VERIFIED`;
- separate canonical site owner: `VERIFIED PROJECT CONTRACT`;
- former synagogue / Soviet Culture House adaptation: `VERIFIED PUBLIC HISTORICAL CONTEXT`;
- old park beside Culture House: `VERIFIED PUBLIC CONTEXT`;
- current six-column source facade: `PROVISIONAL WORKING HYPOTHESIS`;
- exact facade bearing: `PROVISIONAL`;
- exact dimensions: `PROVISIONAL`;
- exact facade colors/material zones: `PROVISIONAL`;
- exact roof/window/door rhythm: `PROVISIONAL`;
- rear/interior: `UNKNOWN / PROVISIONAL`.

## 13. Pass45 implementation consequence

Binding this spec closes documentation ambiguity, not runtime fidelity.

It means:

- Culture House has a dedicated evidence namespace and may no longer be defined by Museum references;
- current source six-column ownership can remain as a guarded provisional implementation while direct photo evidence is incomplete;
- future photo evidence must update this spec before source geometry is promoted to accepted exact fidelity;
- Gate E requires strict landmark separation;
- Gate K requires direct local UE 5.8 screenshots and may reject the current model even with all structural verifiers green.
