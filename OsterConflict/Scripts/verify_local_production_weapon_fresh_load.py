from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
RESULT_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
IMPORT_SENTINEL = RESULT_DIR / "production_weapon_import_result.txt"
FRESH_SENTINEL = RESULT_DIR / "production_weapon_fresh_load_result.txt"

EXPECTED = {
    "/Game/Production/Weapons/M249/SM_M249": "M249",
    "/Game/Production/Weapons/Remington870/SM_Remington870": "Remington870",
}


def fail(message):
    unreal.log_error(f"[OC Production Weapon Fresh Load] {message}")
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
        or "_defaultmat" in path
        or name in ("defaultmaterial", "basicshapematerial", "worldgridmaterial")
    )


def material_interfaces(asset):
    result = []
    for slot in list(asset.get_editor_property("static_materials") or []):
        try:
            result.append(slot.get_editor_property("material_interface"))
        except Exception:
            result.append(None)
    return result


def used_textures(material):
    try:
        return list(unreal.MaterialEditingLibrary.get_used_textures(material) or [])
    except Exception as exc:
        fail(f"Cannot inspect used textures for {material}: {exc}")


def main():
    RESULT_DIR.mkdir(parents=True, exist_ok=True)
    if FRESH_SENTINEL.exists():
        FRESH_SENTINEL.unlink()
    if not IMPORT_SENTINEL.is_file():
        fail(f"Import result sentinel missing: {IMPORT_SENTINEL}")

    result_text = IMPORT_SENTINEL.read_text(encoding="utf-8", errors="replace")
    imported_paths = [
        line.split("=", 1)[1].strip()
        for line in result_text.splitlines()
        if line.startswith("IMPORTED=")
    ]

    verified = []
    gaps = []
    for object_path in imported_paths:
        label = EXPECTED.get(object_path)
        if not label:
            fail(f"Unexpected production weapon import path: {object_path}")

        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        if asset is None or not isinstance(asset, unreal.StaticMesh):
            fail(f"Fresh load failed or asset is not StaticMesh: {object_path}")

        materials = material_interfaces(asset)
        if not materials:
            gaps.append(f"{label}:no_material_slots")
            continue

        placeholder_slots = [i for i, material in enumerate(materials) if is_placeholder_material(material)]
        if placeholder_slots:
            gaps.append(f"{label}:placeholder_slots={placeholder_slots}")
            continue

        texture_gap_slots = []
        for slot_index, material in enumerate(materials):
            textures = used_textures(material)
            if not textures:
                texture_gap_slots.append(slot_index)
                continue
            for texture in textures:
                if texture is None or unreal.EditorAssetLibrary.load_asset(texture.get_path_name()) is None:
                    texture_gap_slots.append(slot_index)
                    break

        if texture_gap_slots:
            gaps.append(f"{label}:texture_gap_slots={sorted(set(texture_gap_slots))}")
            continue

        verified.append(object_path)
        unreal.log(
            f"[OC Production Weapon Fresh Load] AUTHORED_DEPENDENCIES_READY {label} "
            f"path={object_path} slots={len(materials)}"
        )

    status = "PASS" if len(verified) == len(EXPECTED) and not gaps else "GAP"
    lines = [f"STATUS={status}"]
    lines.extend(f"VERIFIED={path}" for path in verified)
    lines.extend(f"GAP={gap}" for gap in gaps)

    # Missing exact source/imports remain explicit gaps rather than being promoted from the generic R13 fallbacks.
    for path, label in EXPECTED.items():
        if path not in imported_paths:
            lines.append(f"GAP={label}:not_imported_this_pass")

    FRESH_SENTINEL.write_text("\n".join(lines) + "\n", encoding="utf-8")
    unreal.log(
        f"[OC Production Weapon Fresh Load] RESULT status={status} "
        f"verified={len(verified)}/{len(EXPECTED)} gaps={len(gaps)} sentinel={FRESH_SENTINEL}"
    )


if __name__ == "__main__":
    main()
