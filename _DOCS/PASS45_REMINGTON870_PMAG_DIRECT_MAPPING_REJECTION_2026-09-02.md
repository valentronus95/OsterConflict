# PASS45 Remington 870 Pmag direct-mapping rejection — 2026-09-02

Parent: `PASS45_RUNTIME_RECOVERY_TZ.md` item 16  
Canonical branch checkpoint before this document: `77cb5621c152f804f2f548a099aa9f09d3fadf63`  
Registered donor SHA-256: `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`

## Factual evidence

The exact registered 8sianDude donor already passed isolated UE 5.8 imported-motion proof. The remaining semantic question was whether `Pmag_061` could be treated directly as the physical fore-end/pump.

Two exact-donor source audits now narrow that question:

1. `PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT.py`
   - CI run `33640380077`: **SUCCESS**;
   - exact donor verified before analysis;
   - bind-pose world-space overlay generated from inverse-bind matrices;
   - `PBody_058` influenced geometry: 7,318 vertices;
   - `Pmag_061` influenced geometry: 4,411 vertices;
   - result deliberately remained `pump_node_identity=UNPROVEN`.

2. `PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT.py`
   - CI run `33641383294`: **SUCCESS**;
   - exact donor verified before analysis;
   - `Pmag_061` / `Object_95`: 4,411 vertices, 4,982 triangles;
   - connected-component count: **106**;
   - artifact: `pass45-remington870-pmag-component-evidence` / artifact id `9850965771`;
   - result remained `pump_node_identity=UNPROVEN`.

The component evidence is enough to reject one narrow implementation shortcut:

`DIRECT_PMAG_061_AS_PUMP_MAPPING = REJECTED`.

A single `Pmag_061` transform drives a composite set of disconnected authored geometry. Therefore wiring that entire joint directly as Oster's accepted pump/fore-end would move all of those components together and would not be a factually isolated fore-end identity.

This is a rejection of the **direct mapping**, not yet a rejection of the registered donor itself.

## Reuse-first consequence

The registered donor remains the primary source while a narrower salvage path is still factually available. Do not stack or promote a second Remington donor merely because the original skeleton grouping is inconvenient.

The next legal operation is to partition the `Pmag_061` weighted geometry into physical groups and identify which connected-component group is the actual fore-end/pump. If that group can be isolated without altering unrelated weapon geometry, prepare a source-derived re-rig/extraction pilot that gives only the fore-end authoritative pump motion. The original reload clips may be used as motion evidence, but no full reload clip may be silently relabelled as a standalone post-shot pump sequence.

If the fore-end cannot be isolated cleanly or the resulting source-derived rig cannot preserve acceptable authored geometry/motion through UE 5.8, then the primary donor may be factually rejected for item 16 and only then may the quarantined alternate-donor path be considered under the reuse-first rules.

## Acceptance boundary

- registered donor UE 5.8 imported motion: **PASS**;
- direct `Pmag_061` -> pump mapping: **REJECTED**;
- exact fore-end component group: **UNPROVEN**;
- source-derived fore-end re-rig/extraction: **NOT YET PROVEN**;
- standalone post-shot pump sequence: **UNPROVEN**;
- production cutover: **NOT AUTHORIZED**;
- runtime acceptance: **NOT ACCEPTED**;
- item 16: **UNCHECKED**;
- PR #94: **OPEN / UNMERGED**.

Official checklist progress remains **22/36 = 61.1% complete, 38.9% remaining**.
