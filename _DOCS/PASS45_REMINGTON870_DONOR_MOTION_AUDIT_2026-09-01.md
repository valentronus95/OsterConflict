# PASS45 Remington 870 donor motion audit — 2026-09-01

Parent: `PASS45_RUNTIME_RECOVERY_TZ.md` item 16  
Source intake: `_DOCS/PASS45_REMINGTON870_SOURCE_INTAKE.md`  
Third-party record: `_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md` / `PASS45-3P-WEAPON-001`  
Pinned donor SHA-256: `147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2`

## Purpose

This audit answers a narrower question than the source-intake contract: does the exact acquired CC-BY-4.0 Remington 870 donor contain factual weapon-node motion that can justify continuing the isolated UE 5.8 import pilot, and does that evidence already prove a standalone pump/manual-action sequence?

It does not promote content into `/Game/Production`, does not populate `ManualActionAnimationObjectPath`, does not create a second gameplay/action timer, and does not change PASS45 runtime acceptance.

## Pinned action-target evidence

The acquisition probe records deterministic target fingerprints for the three action clips already pinned by the upstream consumer contract:

- fire index 2: `5446382d16de5fa56ad784bce41e80be3ae70dc76c03a8a6456e660957f9e5e1`;
- easy reload index 3: `4e96b343cc6268a6239d104ae635abf6dbab5609b01dbca37c956e406e26083f`;
- full reload index 4: `a93ebf3318791345771e8f775d718d2f89427f568378249d1077090687f29487`.

All three clips target 56 unique nodes. The named weapon-side targets common to those clips include `Rif_059`, `Trigger_060`, `PBody_058` and `Pmag_061`.

## Measured weapon-node motion

The probe reads the pinned GLB binary animation accessors directly. Translation values are reported in donor/glTF units; rotation is the maximum quaternion angular delta from the clip's first sample.

| Clip | Node | Translation max displacement | Rotation max angle |
|---|---|---:|---:|
| fire index 2 | `PBody_058` | `0.050905` | `24.037146°` |
| fire index 2 | `Pmag_061` | `0.050845` | `24.037145°` |
| easy reload index 3 | `PBody_058` | `0.067139` | `25.570902°` |
| easy reload index 3 | `Pmag_061` | `0.606719` | `76.062898°` |
| full reload index 4 | `PBody_058` | `0.070092` | `25.570902°` |
| full reload index 4 | `Pmag_061` | `0.606719` | `76.062898°` |

`Rif_059` and `Trigger_060` are also animation targets, but the pinned measurement does not establish them as the Remington pump. The large reload-specific `Pmag_061` motion proves that the donor is not merely static geometry with arm-only animation: a named weapon-side part moves materially during both reload clips.

## Fail-closed interpretation

Verdict: **ARTICULATED_RELOAD_MOTION_PROVEN / PUMP_NODE_IDENTITY_UNPROVEN / STANDALONE_PUMP_CLIP_UNPROVEN**.

What is proven:

- exact pinned donor bytes contain non-trivial weapon-side motion;
- reload motion is materially different from the fire clip;
- the donor remains worth testing through the existing isolated UE 5.8 import pilot.

What is not proven:

- `Pmag_061` is the physical fore-end/pump rather than another weapon-side part;
- any one of the five donor animations is an isolated post-shot pump cycle suitable for direct `ManualActionAnimationObjectPath` use;
- UE 5.8 preserves the needed hierarchy/track semantics after import;
- the clip is visually acceptable in Oster first person;
- the donor can be promoted to a production `.uasset` or close item 16.

A reload animation must not be silently re-labelled as the authoritative pump cycle merely because it moves a weapon part. The next allowed promotion step remains the existing local UE 5.8 isolated import/visual inspection. A second Remington donor must not be stacked beside this primary donor unless that proof produces a factual blocker or rejection, per reuse-first replace-not-stack policy.

## Status

`source_motion_evidence=PASS isolated_ue58_import=PENDING pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN production_cutover=0 runtime_acceptance=0 item16_checked=0`
