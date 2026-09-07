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
    """Best-effort runtime material enumeration.

    UE 5.8 commandlet + -nullrhi can return an empty list here even when the
    saved material package owns real TextureSample dependencies. This result is
    therefore advisory; validate_weapon() has a package-reference fallback that
    proves the persisted dependency from the fresh process.
    """
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


def package_path(path: str) -> str:
    return str(path).split(".", 1)[0]


def persisted_local_texture_dependencies(destination: str, expected_material: str) -> list:
    """Prove saved material -> local texture links without relying on shader/RHI state.

    find_package_referencers_for_asset() reads persisted package references and
    remains valid in the isolated -nullrhi commandlet where MaterialEditingLibrary
    may expose zero used textures. We only accept Texture assets under this exact
    weapon destination and only when the expected authored material is their
    persisted referencer.
    """
    expected_package = package_path(expected_material).lower()
    linked = []

    try:
        assets = list(unreal.EditorAssetLibrary.list_assets(destination, recursive=False, include_folder=False) or [])
    except Exception as exc:
        fail(f"could not enumerate authored assets under {destination}: {type(exc).__name__}: {exc}")

    for asset_path in assets:
        loaded = unreal.EditorAssetLibrary.load_asset(asset_path)
        if loaded is None or not isinstance(loaded, unreal.Texture):
            continue
        if is_placeholder_texture(loaded):
            continue

        try:
            referencers = list(
                unreal.EditorAssetLibrary.find_package_referencers_for_asset(
                    asset_path,
                    load_assets_to_confirm=True,
                )
                or []
            )
        except Exception as exc:
            log(f"package referencer lookup failed for {asset_path}: {type(exc).__name__}: {exc}")
            continue

        normalized = {package_path(str(item)).lower() for item in referencers}
        if expected_package in normalized:
            linked.append(str(loaded.get_path_name()))

    return linked


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
    material_api_texture_count = 0
    package_fallback_used = False

    for slot_index, slot in enumerate(slots):
        material = slot.get_editor_property("material_interface")
        if is_placeholder_material(material):
            actual = material.get_path_name() if material else "<missing>"
            fail(f"{folder} slot {slot_index} fresh-loaded placeholder/missing material: {actual}")

        actual_material_package = package_path(material.get_path_name())
        if actual_material_package != expected_material:
            fail(
                f"{folder} slot {slot_index} does not fresh-load the R3 authored material; "
                f"expected={expected_material} actual={material.get_path_name()}"
            )

        textures = used_textures(material)
        slot_local_textures = []
        for texture in textures:
            if is_placeholder_texture(texture):
                actual = texture.get_path_name() if texture else "<missing>"
                fail(f"{folder} slot {slot_index} uses placeholder/missing texture: {actual}")

            texture_path = str(texture.get_path_name())
            reloaded = unreal.EditorAssetLibrary.load_asset(texture_path)
            if reloaded is None:
                fail(f"{folder} slot {slot_index} texture dependency cannot be reopened: {texture_path}")

            material_api_texture_count += 1
            aggregate_textures.add(texture_path)
            if package_path(texture_path).lower().startswith((destination + "/").lower()):
                slot_local_textures.append(texture_path)

        if not slot_local_textures:
            # UE 5.8's isolated -nullrhi commandlet can report zero material-used
            # textures even though the saved package graph contains TextureSample
            # references. Prove the persisted package relationship instead.
            persisted = persisted_local_texture_dependencies(destination, expected_material)
            if not persisted:
                fail(
                    f"{folder} slot {slot_index} exposes no local used textures and the saved R3 material "
                    f"has no persisted local texture dependencies under {destination}"
                )
            package_fallback_used = True
            for texture_path in persisted:
                aggregate_textures.add(texture_path)
            slot_local_textures.extend(persisted)
            log(
                f"{folder} slot {slot_index}: MaterialEditingLibrary exposed zero local textures in -nullrhi; "
                f"persisted package dependency proof accepted {len(persisted)} local texture(s)"
            )

    if not aggregate_textures:
        fail(f"{folder} fresh-loaded authored material has no proven real texture dependencies")

    proof_mode = "PACKAGE_REFERENCE_FALLBACK" if package_fallback_used else "MATERIAL_USED_TEXTURES"
    log(
        f"{folder}: fresh-loaded mesh slots={len(slots)} provenTextures={len(aggregate_textures)} "
        f"materialApiTextures={material_api_texture_count} proof={proof_mode} material={expected_material}"
    )
    return (
        f"{folder}=PASS | slots={len(slots)} | provenTextures={len(aggregate_textures)} | "
        f"proof={proof_mode} | material={expected_material}"
    )


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
        "texture dependencies. Persisted package-reference proof is accepted when UE58 -nullrhi material "
        "enumeration is empty. Rendered runtime visual acceptance remains pending."
    )


if __name__ == "__main__":
    main()
