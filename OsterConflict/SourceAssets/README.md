# Oster Conflict production source assets

This directory stores original authoring/download source files separately from Unreal `.uasset` content.
`Scripts/import_production_vehicle_assets.py` converts the source files below into stable production assets under `/Game/Production/...`.

## Required vehicle source layout

```text
SourceAssets/
  Production/
    Vehicles/
      HMMWV/
        ukrainian_hmmwv_mk_19.glb
      BTR4/
        BTR4_Bucephalus.fbx
        Textures/
          Bahnya_low_albedo.png
          Koleso_low_albedo.png
          Korpus_low_albedo.png
          Windows_low_albedo.png
          interior.png
          tire.png
    Weapons/
      M2/
        m2_50cal_machinegun_cc0.glb
```

## Canonical Unreal outputs

- `/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA`
- `/Game/Production/Weapons/M2/SM_M2_Browning`
- `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`

The importer removes the `Mk19` scene subtree from the HMMWV source before import and combines/bakes the remaining vehicle hierarchy. The M2 is imported separately and mounted by `AOCPickupGunTruck` on its existing gameplay turret pivot. The BTR-4 is imported as a combined visual shell while `AOCBTR` keeps its existing physics, suspension, armor and weapon logic.

## Source/license notes

- Ukrainian HMMWV Mk19 upload metadata identifies the source as the Sketchfab model by `42manako` and carries CC-BY-4.0 metadata. Preserve attribution.
- M2 upload metadata identifies the source as the Sketchfab model by `britdawgmasterfunk`. The model title mentions CC0, while downloaded GLB metadata was observed as CC-BY-4.0. Preserve attribution unless the source license is re-verified.
- The user-selected BTR-4 upload did not include a license file. Its FBX contains an authoring/source path referring to a GTA San Andreas BTR-4E Bucephalus mod. Treat the asset as development-only until its redistribution license/source is verified.

Do not hand-place raw GLB/FBX files inside `/Content`. Run `IMPORT_PRODUCTION_VEHICLES_UE58.cmd` after the source files are present. Generated `.uasset` files belong in `/Content/Production/...` and are tracked through Git LFS.
