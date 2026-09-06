# PASS45 Remington 870 local MotionLab candidate quarantine — 2026-09-02

Parent: `PASS45_RUNTIME_RECOVERY_TZ.md`  
Canonical branch at the proof checkpoint: `fix/pass45-runtime-rejection-material-closure-20260826`  
Canonical HEAD at the proof checkpoint: `bcd3f53ccab6696e2855a7b296ee9662abe707b6`

## Factual local evidence

A local UE 5.8 MotionLab proof on the canonical HEAD successfully imported a file named:

`Remington_870_FREE.glb`

The local proof preserved named actions:

- `PumpAction`
- `Cube.002Action`

and produced an imported motion object named:

`Remington_870_FREE_PumpAction`

`SetupMotionLab` and `ShowMotion` completed the imported-motion proof.

Classification of that result:

`IMPORTED-MOTION PROOF ONLY`

It proves that the local payload can carry non-trivial imported motion in the tested UE 5.8 editor path. It does **not** by itself prove source identity, licensing, visual pump identity, a production-ready standalone pump cycle, production cutover, gameplay acceptance, or checklist item 16.

## Mandatory donor separation

The repository already has one separately registered Remington donor:

`SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb`

That donor is pinned to the 8sianDude Sketchfab source through the reviewed `Parking-Master/FPS` transport and has its own SHA-256, license and structure/motion evidence.

The local file `Remington_870_FREE.glb` is **not** allowed to inherit any provenance, license, hash, node identity, material identity or acceptance claim from that registered donor.

Likewise, `PumpAction` / `Cube.002Action` evidence from the local candidate is not allowed to retroactively describe the older `PBody_058` / `Pmag_061` donor.

## Current status

`Remington_870_FREE.glb` is classified as:

`UNREGISTERED_LOCAL_CANDIDATE`

Required promotion blockers remain:

1. exact SHA-256 and byte size;
2. exact source URL;
3. exact creator/publisher;
4. exact license terms and redistribution/public-repository permission;
5. exact source/version identity or immutable transport;
6. visual identification of the physical fore-end/pump;
7. validation that the accepted motion is a standalone post-shot manual-action cycle rather than an unrelated object action;
8. production import/cutover;
9. current-head UE 5.8 gameplay runtime acceptance.

Until those are resolved:

- do not copy the local candidate into `SOURCE_ASSETS/PASS45/Remington870`;
- do not add it to production asset paths;
- do not populate accepted `ManualActionAnimationObjectPath` from it;
- do not mark `runtime_ready=true`;
- do not mark item 16 complete;
- do not merge PR #94 on this evidence.

## Identity capture command

Run the repository-owned fail-closed audit against the exact local payload:

```bat
py -3 PASS45_REMINGTON870_LOCAL_CANDIDATE_IDENTITY_AUDIT.py inspect "C:\path\to\Remington_870_FREE.glb" --output "_DOCS\PASS45_REMINGTON870_LOCAL_CANDIDATE_IDENTITY_LOCAL.json"
```

The output records the exact SHA-256, byte size, glTF asset metadata, action names, node/mesh/skin counts and materials. It intentionally leaves `source_url`, `creator`, `license_id` and `public_repo_allowed` unresolved.

Do not commit the generated identity JSON as a production source record until its exact payload is matched to a verifiable source/license record.

## Next factual operation

Capture the exact local candidate identity with the audit above, then match that exact fingerprint to the source from which the file was obtained. Only after the source/license record and payload identity agree may the candidate enter normal source-intake review.

This quarantine is a progress-preserving boundary: the UE 5.8 motion proof remains useful, but it cannot silently become a production/content-license claim.
