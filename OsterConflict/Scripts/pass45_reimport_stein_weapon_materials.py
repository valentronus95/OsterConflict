from __future__ import annotations

from pathlib import Path

import unreal

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
RAW_ROOT = PROJECT_DIR / "Content" / "Raw" / "R13" / "Weapons" / "SteinClassicWeapons" / "WeaponsPack"
DEST_ROOT = "/Game/R13/Weapons/Stein"
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "SteinWeapons"
SENTINEL = CACHE_DIR / "pass45_stein_material_reimport_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1"

# Only the Stein models that are part of the current runtime rack contract are repaired here.
# AK-47 uses its separate canonical /Game/AK-47 production asset and is intentionally untouched.
WEAPONS = {
    "1911": "SKM_1911.fbx",
    "LeverAction": "SKM_LeverAction.fbx",
    "M14": "SKM_M14.fbx",
    "M700": "SKM_M700.fbx",
    "MP5": "SKM_MP5.fbx",
    "Mac10": "SKM_Mac10.fbx",
    "Tec9": "SKM_Tec9.fbx",
}

PLACEHOLDER_MARKERS = (
    "BasicShapeMaterial",
    "DefaultMaterial",
    "WorldGridMaterial",
    "_defaultMat",
)


def log(message: str) -> None:
    unreal.log(f"[PASS45 Stein Material Reimport] {message}")


def fail(message: str) -> None:
    raise RuntimeError(f"PASS45 Stein material reimport failed: {message}")


def run_import(source: Path, destination: str, options=None) -> list[str]:
    if not source.is_file():
        fail(f"required source is missing: {source}")

    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    if options is not None:
        task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.imported_object_paths)
    if not imported:
        fail(f"Unreal imported no assets from {source}")
    return imported


def import_source_textures(source_dir: Path, destination: str) -> list[str]:
    texture_sources = sorted(source_dir.glob("*.png"))
    if not texture_sources:
        fail(f"no authored PNG textures found beside {source_dir.name} FBX")

    imported: list[str] = []
    for texture_source in texture_sources:
        imported.extend(run_import(texture_source, destination))

    for texture_source in texture_sources:
        object_path = f"{destination}/{texture_source.stem}.{texture_source.stem}"
        if not unreal.EditorAssetLibrary.does_asset_exist(object_path):
            fail(f"authored texture asset missing after import: {object_path}")

    return imported


def import_stein_fbx(source: Path, destination: str) -> list[str]:
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)

    static_data = options.get_editor_property("static_mesh_import_data")
    if static_data:
        static_data.set_editor_property("combine_meshes", True)

    return run_import(source, destination, options)


def is_placeholder(material) -> bool:
    if material is None:
        return True
    path = material.get_path_name()
    return any(marker.lower() in path.lower() for marker in PLACEHOLDER_MARKERS)


def validate_mesh_dependencies(folder: str, fbx_name: str, destination: str) -> tuple[int, int]:
    mesh_name = Path(fbx_name).stem
    mesh_path = f"{destination}/{mesh_name}.{mesh_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if mesh is None:
        fail(f"runtime Stein mesh missing after reimport: {mesh_path}")

    static_materials = list(mesh.get_editor_property("static_materials"))
    if not static_materials:
        fail(f"{folder} mesh has zero material slots after reimport")

    aggregate_texture_paths: set[str] = set()
    for slot_index, static_material in enumerate(static_materials):
        material = static_material.get_editor_property("material_interface")
        if is_placeholder(material):
            material_path = material.get_path_name() if material else "<missing>"
            fail(f"{folder} slot {slot_index} is placeholder/missing after reimport: {material_path}")

        for texture in unreal.MaterialEditingLibrary.get_material_used_textures(material):
            if texture is not None:
                aggregate_texture_paths.add(texture.get_path_name())

    if not aggregate_texture_paths:
        fail(f"{folder} authored materials use zero textures after texture-first FBX reimport")

    local_dependencies = [
        path for path in aggregate_texture_paths
        if path.lower().startswith((destination + "/").lower())
    ]
    if not local_dependencies:
        fail(
            f"{folder} materials have textures, but none resolve to the weapon destination {destination}; "
            "the FBX material dependency chain is still broken"
        )

    return len(static_materials), len(aggregate_texture_paths)


def reimport_weapon(folder: str, fbx_name: str) -> str:
    source_dir = RAW_ROOT / folder
    fbx_source = source_dir / fbx_name
    destination = f"{DEST_ROOT}/{folder}"

    # Critical Pass45 ordering: external PNG files are explicit authored source content.
    # Import them first so the subsequent FBX material import can resolve texture dependencies.
    imported_textures = import_source_textures(source_dir, destination)
    import_stein_fbx(fbx_source, destination)
    unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)

    material_slots, used_textures = validate_mesh_dependencies(folder, fbx_name, destination)
    log(
        f"{folder}: authored texture assets imported={len(imported_textures)}; "
        f"material_slots={material_slots}; used_textures={used_textures}"
    )
    return (
        f"{folder}=PASS | importedTextureAssets={len(imported_textures)} | "
        f"materialSlots={material_slots} | usedTextures={used_textures}"
    )


def main() -> None:
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()

    results: list[str] = []
    for folder, fbx_name in WEAPONS.items():
        results.append(reimport_weapon(folder, fbx_name))

    SENTINEL.write_text(
        f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}\n"
        "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS\n"
        "STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING\n"
        + "\n".join(results)
        + "\n",
        encoding="utf-8",
    )
    log(
        "Texture-first Stein reimport completed with non-placeholder material slots and used texture "
        f"dependencies. revision={IMPORT_CONTRACT_REVISION}. This is editor import evidence only; "
        "runtime rack screenshot remains authoritative."
    )


if __name__ == "__main__":
    main()
