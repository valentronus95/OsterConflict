from __future__ import annotations

from pathlib import Path

import unreal

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "SteinWeapons"
SENTINEL = CACHE_DIR / "pass45_stein_material_fresh_load_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"
DEST_ROOT = "/Game/R13/Weapons/Stein"

WEAPONS = {
    "1911": "SKM_1911",
    "LeverAction": "SKM_LeverAction",
    "M14": "SKM_M14",
    "M700": "SKM_M700",
    "MP5": "SKM_MP5",
    "Mac10": "SKM_Mac10",
    "Tec9": "SKM_Tec9",
}

PLACEHOLDER_MATERIAL_MARKERS = (
    "BasicShapeMaterial",
    "DefaultMaterial",
    "WorldGridMaterial",
    "_defaultMat",
)
PLACEHOLDER_TEXTURE_MARKERS = (
    "DefaultTexture",
    "WhiteSquareTexture",
)


def log(message: str) -> None:
    unreal.log(f"[PASS45 Stein Fresh Load] {message}")


def fail(message: str) -> None:
    raise RuntimeError(f"PASS45 Stein fresh-load validation failed: {message}")


def is_placeholder_material(material) -> bool:
    if material is None:
        return True
    path = str(material.get_path_name()).lower()
    return any(marker.lower() in path for marker in PLACEHOLDER_MATERIAL_MARKERS)


def is_placeholder_texture(texture) -> bool:
    if texture is None:
        return True
    path = str(texture.get_path_name()).lower()
    return any(marker.lower() in path for marker in PLACEHOLDER_TEXTURE_MARKERS)


def used_textures(material) -> list:
    textures = []
    try:
        textures = list(unreal.MaterialEditingLibrary.get_material_used_textures(material) or [])
    except Exception as exc:
        log(f"GetMaterialUsedTextures failed for {material.get_path_name()}: {type(exc).__name__}: {exc}")

    if not textures and isinstance(material, unreal.Material):
        try:
            textures = list(unreal.MaterialEditingLibrary.get_used_textures(material) or [])
        except Exception as exc:
            log(f"GetUsedTextures fallback failed for {material.get_path_name()}: {type(exc).__name__}: {exc}")
    return textures


def validate_weapon(folder: str, mesh_name: str) -> str:
    destination = f"{DEST_ROOT}/{folder}"
    mesh_path = f"{destination}/{mesh_name}.{mesh_name}"
    expected_material = f"{destination}/M_PASS45_{folder}_Authored_R3"

    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if mesh is None or not isinstance(mesh, unreal.StaticMesh):
        fail(f"{folder} mesh cannot be fresh-loaded as StaticMesh: {mesh_path}")

    slots = list(mesh.get_editor_property("static_materials") or [])
    if not slots:
        fail(f"{folder} fresh-loaded mesh has zero material slots")

    aggregate_textures: set[str] = set()
    for slot_index, slot in enumerate(slots):
        material = slot.get_editor_property("material_interface")
        if is_placeholder_material(material):
            actual = material.get_path_name() if material else "<missing>"
            fail(f"{folder} slot {slot_index} fresh-loaded placeholder/missing material: {actual}")

        actual_material_package = material.get_path_name().split(".", 1)[0]
        if actual_material_package != expected_material:
            fail(
                f"{folder} slot {slot_index} does not fresh-load the R3 authored material; "
                f"expected={expected_material} actual={material.get_path_name()}"
            )

        textures = used_textures(material)
        if not textures:
            fail(f"{folder} slot {slot_index} R3 material still exposes zero used textures after fresh load")

        slot_local_textures = []
        for texture in textures:
            if is_placeholder_texture(texture):
                actual = texture.get_path_name() if texture else "<missing>"
                fail(f"{folder} slot {slot_index} uses placeholder/missing texture: {actual}")

            texture_path = str(texture.get_path_name())
            reloaded = unreal.EditorAssetLibrary.load_asset(texture_path)
            if reloaded is None:
                fail(f"{folder} slot {slot_index} texture dependency cannot be reopened: {texture_path}")

            aggregate_textures.add(texture_path)
            if texture_path.lower().startswith((destination + "/").lower()):
                slot_local_textures.append(texture_path)

        if not slot_local_textures:
            fail(
                f"{folder} slot {slot_index} has used textures but none belong to its authored Stein destination {destination}"
            )

    log(
        f"{folder}: fresh-loaded mesh slots={len(slots)} usedTextures={len(aggregate_textures)} "
        f"material={expected_material}"
    )
    return f"{folder}=PASS | slots={len(slots)} | usedTextures={len(aggregate_textures)} | material={expected_material}"


def main() -> None:
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()

    results = [validate_weapon(folder, mesh_name) for folder, mesh_name in WEAPONS.items()]
    SENTINEL.write_text(
        f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}\n"
        "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS\n"
        "PASS45_STEIN_FRESH_LOAD=READY\n"
        "PASS45_STEIN_UE58_EXPLICIT_BINDING=READY\n"
        "STATUS=FRESH_LOAD_VALIDATED_RUNTIME_VISUAL_PENDING\n"
        + "\n".join(results)
        + "\n",
        encoding="utf-8",
    )
    log(
        "PASS: every R3 Stein runtime mesh fresh-loaded with its explicit authored material and real local "
        "texture dependencies. Rendered runtime visual acceptance remains pending."
    )


if __name__ == "__main__":
    main()
