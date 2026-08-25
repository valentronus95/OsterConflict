# Pass 45 local UE build/import rejection — 2026-08-25

Status: **LOCAL UE BUILD REJECTED / CORRECTIONS CODED_UNTESTED**

## Test route

User launched the current project through `START_HERE.cmd` and selected `1. ЗВИЧАЙНА ГРА` after pulling the Pass 45 completion merge.

## Factual results

### C++ build

UnrealBuildTool reached `OCTacticalMapVisual.cpp` and failed with `C2131: expression did not evaluate to a constant` while instantiating the Pass 45 reference-road table.

Root cause: the table was declared `constexpr` while its entries contain `FVector2D`; the factual UE 5.8.1 / MSVC toolchain does not accept that constructor in this constant-expression context.

Correction: keep the exact topology data as a normal namespace-scope `const` table. Runtime behavior and `UE_ARRAY_COUNT` semantics remain unchanged.

### Production vehicle import

Local source recovery found HMMWV, M2 Browning and BTR-4 sources.

- BTR-4 imported successfully to the canonical production path.
- HMMWV failed because UE 5.8 rejected deprecated Interchange property `auto_detect_mesh_type` / `bAutoDetectMeshType`.
- M2 failed for the same reason.
- launcher correctly reported the partial result instead of printing a false all-ready message.

Correction: remove the deprecated property and set `convert_statics_with_animated_transform_to_skeletals=false` while retaining explicit `IFMT_STATIC_MESH`, static import enabled and skeletal import disabled.

## Acceptance

Neither correction is VERIFIED yet. Required next factual evidence:

1. local UBT exits 0;
2. HMMWV and M2 imports complete without the deprecated-property exception;
3. only after successful build continue to frontend FPS and gameplay acceptance.

Source CI is regression protection only and cannot promote these corrections to verified UE build/runtime status.
