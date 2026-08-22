from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
SENTINEL = CACHE_DIR / "production_fresh_load_success.txt"

EXPECTED = (
    "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA",
    "/Game/Production/Weapons/M2/SM_M2_Browning",
    "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
)


def fail(message):
    unreal.log_error(f"[OC Fresh Production Load] {message}")
    raise RuntimeError(message)


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()

    loaded = []
    for object_path in EXPECTED:
        asset = unreal.load_asset(object_path)
        if asset is None:
            fail(f"Fresh UE process cannot load canonical production asset: {object_path}")
        if not isinstance(asset, unreal.StaticMesh):
            fail(f"Canonical production asset is not a StaticMesh: {object_path} -> {asset.get_class().get_name()}")

        materials = list(asset.get_editor_property("static_materials") or [])
        if not materials:
            fail(f"Canonical production mesh has no material slots: {object_path}")

        loaded.append(object_path)
        unreal.log(f"[OC Fresh Production Load] PASS {object_path} materials={len(materials)}")

    SENTINEL.write_text("\n".join(loaded) + "\n", encoding="utf-8")
    unreal.log(f"[OC Fresh Production Load] PASS sentinel={SENTINEL}")


if __name__ == "__main__":
    main()
