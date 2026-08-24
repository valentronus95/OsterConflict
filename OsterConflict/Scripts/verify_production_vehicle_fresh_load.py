from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
IMPORT_SENTINEL = CACHE_DIR / "production_import_success.txt"
SENTINEL = CACHE_DIR / "production_fresh_load_success.txt"

EXPECTED = (
    ("HMMWV", "/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA"),
    ("M2", "/Game/Production/Weapons/M2/SM_M2_Browning"),
    ("BTR4", "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus"),
)


def fail(message):
    unreal.log_error(f"[OC Fresh Production Load] {message}")
    raise RuntimeError(message)


def is_placeholder_material(material):
    if material is None:
        return True
    path = str(material.get_path_name()).lower()
    name = str(material.get_name()).lower()
    return (
        "/engine/enginematerials/defaultmaterial" in path
        or "/engine/basicshapes/basicshapematerial" in path
        or "/engine/enginematerials/worldgridmaterial" in path
        or name in ("defaultmaterial", "basicshapematerial", "worldgridmaterial")
    )


def static_material_interfaces(asset):
    result = []
    for slot in list(asset.get_editor_property("static_materials") or []):
        try:
            result.append(slot.get_editor_property("material_interface"))
        except Exception:
            result.append(None)
    return result


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()
    if not IMPORT_SENTINEL.exists():
        fail(f"Import result sentinel is missing: {IMPORT_SENTINEL}")

    import_result = IMPORT_SENTINEL.read_text(encoding="utf-8", errors="replace")
    imported_paths = {
        line.split("=", 1)[1].strip()
        for line in import_result.splitlines()
        if line.startswith("IMPORTED=") and "=" in line
    }
    if not imported_paths:
        fail("Import result sentinel contains no independently imported canonical asset.")

    loaded = []
    for label, object_path in EXPECTED:
        if object_path not in imported_paths:
            unreal.log_warning(f"[OC Fresh Production Load] CONTENT GAP {label}: not imported in this intake pass")
            continue

        asset = unreal.load_asset(object_path)
        if asset is None:
            fail(f"Fresh UE process cannot load canonical production asset: {object_path}")
        if not isinstance(asset, unreal.StaticMesh):
            fail(f"Canonical production asset is not a StaticMesh: {object_path} -> {asset.get_class().get_name()}")

        materials = static_material_interfaces(asset)
        if not materials:
            fail(f"Canonical production mesh has no material interfaces: {object_path}")

        placeholder_slots = [index for index, material in enumerate(materials) if is_placeholder_material(material)]
        material_paths = [material.get_path_name() if material is not None else "NULL" for material in materials]
        if placeholder_slots:
            fail(
                f"Canonical production mesh is not authored-material ready: {object_path} "
                f"placeholder_slots={placeholder_slots} materials={material_paths}"
            )

        loaded.append(object_path)
        unreal.log(
            f"[OC Fresh Production Load] AUTHORED_MATERIALS_READY {label} path={object_path} "
            f"slots={len(materials)} materials={material_paths} placeholder_slots=0"
        )

    if set(loaded) != imported_paths:
        missing = sorted(imported_paths.difference(loaded))
        fail(f"Fresh-load verification did not reopen every imported canonical asset: {missing}")

    SENTINEL.write_text("\n".join(loaded) + "\n", encoding="utf-8")
    unreal.log(
        f"[OC Fresh Production Load] PASS imported={len(loaded)} authored_materials_ready={len(loaded)} "
        f"sentinel={SENTINEL}"
    )


if __name__ == "__main__":
    main()
