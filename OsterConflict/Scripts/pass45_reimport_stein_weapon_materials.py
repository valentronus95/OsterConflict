from __future__ import annotations

from pathlib import Path

import unreal

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
RAW_ROOT = PROJECT_DIR / "Content" / "Raw" / "R13" / "Weapons" / "SteinClassicWeapons" / "WeaponsPack"
DEST_ROOT = "/Game/R13/Weapons/Stein"
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "SteinWeapons"
SENTINEL = CACHE_DIR / "pass45_stein_material_reimport_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"

# Only the Stein models that are part of the current runtime rack contract are authored here.
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


def imported_texture_asset(destination: str, source: Path):
    object_path = f"{destination}/{source.stem}.{source.stem}"
    texture = unreal.EditorAssetLibrary.load_asset(object_path)
    if texture is None:
        fail(f"authored texture could not be loaded after import: {object_path}")
    return texture


def choose_texture_source(source_dir: Path, suffixes: tuple[str, ...]) -> Path | None:
    textures = sorted(source_dir.glob("*.png"))
    for suffix in suffixes:
        for texture in textures:
            if texture.stem.lower().endswith(suffix.lower()):
                return texture
    return None


def create_explicit_authored_material(folder: str, source_dir: Path, destination: str):
    """Create the deterministic material asset that the runtime mesh will actually own.

    R2 still allowed the FBX-created material to decide whether explicit binding was necessary. The factual
    UE 5.8 run on 2026-08-26 showed that this remained ambiguous: the commandlet reached validation with zero
    discoverable texture dependencies. R3 removes that ambiguity. Every Stein runtime mesh receives one
    revisioned material authored directly from the committed PNG source, and a separate fresh UE process is
    responsible for proving that the saved package reopens with real texture dependencies.
    """
    color_source = choose_texture_source(
        source_dir,
        ("_c", "_basecolor", "_base_color", "_albedo", "_diffuse", "_d"),
    )
    if color_source is None:
        fail(f"{folder} has PNG source textures but no identifiable authored color texture")

    material_name = f"M_PASS45_{folder}_Authored_R3"
    material_path = f"{destination}/{material_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(material_path):
        if not unreal.EditorAssetLibrary.delete_asset(material_path):
            fail(f"could not replace stale generated authored material: {material_path}")

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        material_name,
        destination,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    if material is None:
        fail(f"could not create explicit authored material: {material_path}")

    color_texture = imported_texture_asset(destination, color_source)
    color_sample = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionTextureSample,
        -420,
        0,
    )
    if color_sample is None:
        fail(f"could not create BaseColor texture expression for {folder}")
    color_sample.set_editor_property("texture", color_texture)
    if not unreal.MaterialEditingLibrary.connect_material_property(
        color_sample,
        "RGB",
        unreal.MaterialProperty.MP_BASE_COLOR,
    ):
        fail(f"could not connect authored BaseColor texture for {folder}")

    optional_maps = (
        (("_m", "_metallic"), unreal.MaterialProperty.MP_METALLIC, "R", -420, 180),
        (("_r", "_roughness"), unreal.MaterialProperty.MP_ROUGHNESS, "R", -420, 320),
        (("_ao", "_ambientocclusion", "_ambient_occlusion"), unreal.MaterialProperty.MP_AMBIENT_OCCLUSION, "R", -420, 460),
    )
    for suffixes, material_property, output_name, x, y in optional_maps:
        source = choose_texture_source(source_dir, suffixes)
        if source is None:
            continue
        texture = imported_texture_asset(destination, source)
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            material,
            unreal.MaterialExpressionTextureSample,
            x,
            y,
        )
        if sample is None:
            fail(f"could not create optional authored texture expression {source.name} for {folder}")
        sample.set_editor_property("texture", texture)
        if not unreal.MaterialEditingLibrary.connect_material_property(
            sample,
            output_name,
            material_property,
        ):
            fail(f"could not connect optional authored texture {source.name} for {folder}")

    compiler_errors = list(unreal.MaterialEditingLibrary.recompile_material(material) or [])
    if compiler_errors:
        fail(f"{folder} explicit authored material compile errors: {' | '.join(str(item) for item in compiler_errors)}")

    expression_count = unreal.MaterialEditingLibrary.get_num_material_expressions(material)
    if expression_count < 1:
        fail(f"{folder} explicit authored material contains no graph expressions")

    if not unreal.EditorAssetLibrary.save_asset(material_path, only_if_is_dirty=False):
        fail(f"could not save explicit authored material: {material_path}")

    log(
        f"{folder}: R3 explicit UE58 material authored from committed PNG source; "
        f"material={material_path}; baseColor={color_source.name}; expressions={expression_count}"
    )
    return material, material_path


def bind_material_to_mesh_slots(mesh, material, folder: str) -> int:
    static_materials = list(mesh.get_editor_property("static_materials"))
    if not static_materials:
        fail(f"{folder} mesh has zero material slots after reimport")

    for static_material in static_materials:
        static_material.set_editor_property("material_interface", material)
    mesh.set_editor_property("static_materials", static_materials)
    return len(static_materials)


def advisory_used_texture_paths(material) -> set[str]:
    """Best-effort same-process visibility only; fresh-load verification is authoritative in R3."""
    texture_paths: set[str] = set()
    try:
        for texture in list(unreal.MaterialEditingLibrary.get_material_used_textures(material) or []):
            if texture is not None:
                texture_paths.add(texture.get_path_name())
    except Exception as exc:
        log(f"same-process GetMaterialUsedTextures advisory failed: {type(exc).__name__}: {exc}")

    if not texture_paths and isinstance(material, unreal.Material):
        try:
            for texture in list(unreal.MaterialEditingLibrary.get_used_textures(material) or []):
                if texture is not None:
                    texture_paths.add(texture.get_path_name())
        except Exception as exc:
            log(f"same-process GetUsedTextures advisory failed: {type(exc).__name__}: {exc}")
    return texture_paths


def validate_authored_assignment(folder: str, fbx_name: str, destination: str, material_path: str) -> tuple[int, int]:
    mesh_name = Path(fbx_name).stem
    mesh_path = f"{destination}/{mesh_name}.{mesh_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if mesh is None:
        fail(f"runtime Stein mesh missing after R3 reimport: {mesh_path}")

    static_materials = list(mesh.get_editor_property("static_materials"))
    if not static_materials:
        fail(f"{folder} mesh has zero material slots after R3 reimport")

    for slot_index, static_material in enumerate(static_materials):
        material = static_material.get_editor_property("material_interface")
        if is_placeholder(material):
            actual = material.get_path_name() if material else "<missing>"
            fail(f"{folder} slot {slot_index} is placeholder/missing after R3 authoring: {actual}")
        if material.get_path_name().split(".", 1)[0] != material_path:
            fail(
                f"{folder} slot {slot_index} does not own the R3 authored material; "
                f"expected={material_path} actual={material.get_path_name()}"
            )

    material = unreal.EditorAssetLibrary.load_asset(material_path)
    if material is None:
        fail(f"{folder} R3 authored material cannot be reloaded in authoring process: {material_path}")

    same_process_textures = advisory_used_texture_paths(material)
    return len(static_materials), len(same_process_textures)


def reimport_weapon(folder: str, fbx_name: str) -> str:
    source_dir = RAW_ROOT / folder
    fbx_source = source_dir / fbx_name
    destination = f"{DEST_ROOT}/{folder}"

    imported_textures = import_source_textures(source_dir, destination)
    import_stein_fbx(fbx_source, destination)

    # R3 always owns the final mesh slots. We no longer accept the FBX-created material as sufficient merely
    # because it exists; that was exactly the state that produced the zero-texture R2 failure.
    material, material_path = create_explicit_authored_material(folder, source_dir, destination)

    mesh_name = Path(fbx_name).stem
    mesh_path = f"{destination}/{mesh_name}.{mesh_name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if mesh is None:
        fail(f"runtime Stein mesh missing after FBX import: {mesh_path}")
    slot_count = bind_material_to_mesh_slots(mesh, material, folder)
    if not unreal.EditorAssetLibrary.save_asset(mesh_path, only_if_is_dirty=False):
        fail(f"could not save R3 Stein mesh assignment: {mesh_path}")
    unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)

    material_slots, same_process_used_textures = validate_authored_assignment(
        folder,
        fbx_name,
        destination,
        material_path,
    )
    log(
        f"{folder}: authored texture assets imported={len(imported_textures)}; material_slots={material_slots}; "
        f"same_process_used_textures={same_process_used_textures}; binding=R3_ALWAYS_EXPLICIT; "
        "fresh_load=MANDATORY"
    )
    return (
        f"{folder}=AUTHORED | importedTextureAssets={len(imported_textures)} | "
        f"materialSlots={slot_count} | sameProcessUsedTextures={same_process_used_textures} | "
        f"material={material_path}"
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
        "PASS45_STEIN_AUTHORED_GRAPH=PASS\n"
        "PASS45_STEIN_UE58_EXPLICIT_BINDING=READY\n"
        "STATUS=EDITOR_GRAPH_AUTHORED_FRESH_LOAD_PENDING\n"
        + "\n".join(results)
        + "\n",
        encoding="utf-8",
    )
    log(
        "Stein R3 authoring completed. Every runtime Stein mesh slot owns a revisioned material graph built "
        "from committed PNG source. Same-process texture enumeration is advisory only; a fresh UE process "
        f"must still reopen and prove dependencies. revision={IMPORT_CONTRACT_REVISION}."
    )


if __name__ == "__main__":
    main()
