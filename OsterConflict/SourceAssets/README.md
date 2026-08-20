# Oster Conflict source asset intake

Raw third-party source files live here before Unreal import. Keep authored source packages separate from `/Content` `.uasset` output.

## Current intake folders

- `Production/Weapons/M2/` - Browning M2 source model and license/source notes.
- `Production/Vehicles/HMMWV/` - Ukrainian HMMWV source model and license/source notes.
- `Incoming/Vehicles/BTR4/` - BTR-4 source files pending source/license verification before production integration.

## Import rule

Do not hand-place raw GLB/FBX files inside `/Content`. Import them through Unreal/Interchange into stable production content folders, then commit generated `.uasset` files through Git LFS.

For archives, extract them locally before integration so mesh, texture and license files can be reviewed. Preserve the original archive only when it contains license/readme material that is not present in the extracted directory.
