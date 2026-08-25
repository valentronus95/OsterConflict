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
BTR_DEST = "/Game/Production/Vehicles/BTR4"


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


def package_name(asset_path):
    return str(asset_path).split(".", 1)[0]


def verify_btr_texture_dependencies(materials):
    """Reject the white-shell class of failure where imported materials exist but use no imported BTR texture."""
    asset_paths = list(unreal.EditorAssetLibrary.list_assets(BTR_DEST, recursive=True, include_folder=False) or [])
    material_packages = {package_name(material.get_path_name()) for material in materials if material is not None}
    discovered_material_packages = set()
    texture_paths = []

    for asset_path in asset_paths:
        asset = unreal.load_asset(asset_path)
        if asset is None:
            continue
        if isinstance(asset, unreal.MaterialInterface):
            discovered_material_packages.add(package_name(asset.get_path_name()))
        elif isinstance(asset, unreal.Texture):
            texture_paths.append(str(asset.get_path_name()))

    owned_material_packages = material_packages.intersection(discovered_material_packages)
    if not owned_material_packages:
        fail(
            f"BTR4 authored-material dependency gap: canonical mesh materials are not present under {BTR_DEST}. "
            f"mesh_materials={sorted(material_packages)} discovered_materials={sorted(discovered_material_packages)}"
        )
    if not texture_paths:
        fail(f"BTR4 authored-texture dependency gap: no imported Texture asset exists under {BTR_DEST}")

    referenced_textures = []
    for texture_path in texture_paths:
        referencers = {
            package_name(ref)
            for ref in list(
                unreal.EditorAssetLibrary.find_package_referencers_for_asset(
                    texture_path, load_assets_to_confirm=True
                ) or []
            )
        }
        if referencers.intersection(owned_material_packages):
            referenced_textures.append(texture_path)

    if not referenced_textures:
        fail(
            "BTR4 authored-texture dependency gap: imported BTR material assets exist, but none reference an "
            f"imported BTR texture. materials={sorted(owned_material_packages)} textures={sorted(texture_paths)}"
        )

    unreal.log(
        f"[OC Fresh Production Load] BTR4_TEXTURE_DEPENDENCIES_READY materials={len(owned_material_packages)} "
        f"textures={len(texture_paths)} referenced_textures={len(referenced_textures)} white_default_guard=1"
    )


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

        if label == "BTR4":
            verify_btr_texture_dependencies(materials)

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
