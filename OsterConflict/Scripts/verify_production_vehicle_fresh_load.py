from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
IMPORT_SENTINEL = CACHE_DIR / "production_import_success.txt"
SENTINEL = CACHE_DIR / "production_fresh_load_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"

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
        or "_defaultmat" in path
        or name in ("defaultmaterial", "basicshapematerial", "worldgridmaterial", "_defaultmat")
    )


def static_material_interfaces(asset):
    result = []
    for slot in list(asset.get_editor_property("static_materials") or []):
        try:
            result.append(slot.get_editor_property("material_interface"))
        except Exception:
            result.append(None)
    return result


def parse_import_lines(text):
    result = {}
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        result.setdefault(key, []).append(value)
    return result


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()
    if not IMPORT_SENTINEL.exists():
        fail(f"Import result sentinel is missing: {IMPORT_SENTINEL}")

    import_result = IMPORT_SENTINEL.read_text(encoding="utf-8", errors="replace")
    parsed = parse_import_lines(import_result)
    revision_values = parsed.get("IMPORT_CONTRACT_REVISION", [])
    if revision_values != [IMPORT_CONTRACT_REVISION]:
        fail(
            f"Production import contract revision mismatch: expected={IMPORT_CONTRACT_REVISION} "
            f"actual={revision_values}"
        )

    imported_paths = set(parsed.get("IMPORTED", []))
    if not imported_paths:
        fail("Import result sentinel contains no independently imported canonical asset.")

    source_kinds = {}
    for value in parsed.get("SOURCE_KIND", []):
        if ":" not in value:
            continue
        label, source_kind = value.split(":", 1)
        source_kinds[label] = source_kind

    btr_forward_axis = parsed.get("BTR4_FORWARD_AXIS", [])
    btr_gltf_up_axis = parsed.get("BTR4_GLTF_UP_AXIS", [])
    btr_internal_up_axis = parsed.get("BTR4_INTERNAL_UP_AXIS", [])
    if "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus" in imported_paths:
        if btr_forward_axis != ["+X"]:
            fail(f"BTR4 canonical import is missing factual +X forward provenance: {btr_forward_axis}")
        if btr_gltf_up_axis != ["+Y"] or btr_internal_up_axis != ["+Z"]:
            fail(
                "BTR4 R3 canonical import is missing factual up-axis provenance: "
                f"gltf_up={btr_gltf_up_axis} internal_up={btr_internal_up_axis}"
            )

    loaded = []
    fresh_lines = [f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}"]

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
            source_kind = source_kinds.get("BTR4", "")
            if source_kind != "authored_external_visual_canonical_plus_x":
                fail(
                    "BTR4 canonical source kind is not the calibrated +X authored fallback: "
                    f"source_kind={source_kind!r}"
                )
            authored_names = [str(material.get_name()) for material in materials if material is not None]
            if not any("M_BTR4_OC_Authored" in name for name in authored_names):
                fail(
                    "Repository-safe canonical BTR4 was imported but its explicit "
                    f"M_BTR4_OC_Authored material is not bound: materials={material_paths}"
                )
            fresh_lines.append("BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored")
            fresh_lines.append("BTR4_FORWARD_AXIS=+X")
            fresh_lines.append("BTR4_GLTF_UP_AXIS=+Y")
            fresh_lines.append("BTR4_INTERNAL_UP_AXIS=+Z")
            fresh_lines.append(f"SOURCE_KIND=BTR4:{source_kind}")

        loaded.append(object_path)
        fresh_lines.append(f"FRESH_LOADED={object_path}")
        unreal.log(
            f"[OC Fresh Production Load] AUTHORED_MATERIALS_READY {label} path={object_path} "
            f"slots={len(materials)} materials={material_paths} placeholder_slots=0"
        )

    if set(loaded) != imported_paths:
        missing = sorted(imported_paths.difference(loaded))
        fail(f"Fresh-load verification did not reopen every imported canonical asset: {missing}")

    SENTINEL.write_text("\n".join(fresh_lines) + "\n", encoding="utf-8")
    unreal.log(
        f"[OC Fresh Production Load] PASS imported={len(loaded)} authored_materials_ready={len(loaded)} "
        f"revision={IMPORT_CONTRACT_REVISION} sentinel={SENTINEL}"
    )


if __name__ == "__main__":
    main()
