from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_DIR = PROJECT_DIR / "Saved" / "AutomationReports" / "ProductionModels"
REPORT_PATH = REPORT_DIR / "required_weapon_asset_preflight.txt"
SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_asset_preflight_success.txt"
MATERIAL_REPORT_PATH = REPORT_DIR / "required_weapon_authored_material_preflight.txt"
MATERIAL_SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_authored_material_preflight_success.txt"

# Mesh-load and material readiness are deliberately separate. A mesh that opens with DefaultMaterial
# is useful geometry evidence but is not production-art evidence and must never be reported as such.
REQUIRED_ASSETS = (
    ("AK-47", "/Game/AK-47/Mesh/SKM_AK-47", unreal.SkeletalMesh),
    ("MP5", "/Game/R13/Weapons/Stein/MP5/SKM_MP5", unreal.StaticMesh),
    ("M1911", "/Game/R13/Weapons/Stein/1911/SKM_1911", unreal.StaticMesh),
    ("M700", "/Game/R13/Weapons/Stein/M700/SKM_M700", unreal.StaticMesh),
    ("M14", "/Game/R13/Weapons/Stein/M14/SKM_M14", unreal.StaticMesh),
    ("MAC-10", "/Game/R13/Weapons/Stein/Mac10/SKM_Mac10", unreal.StaticMesh),
    ("TEC-9", "/Game/R13/Weapons/Stein/Tec9/SKM_Tec9", unreal.StaticMesh),
    ("Lever Action", "/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction", unreal.StaticMesh),
    ("M249 real fallback", "/Game/R13/Weapons/machinegun", unreal.StaticMesh),
    ("Remington 870 real fallback", "/Game/R13/Weapons/shotgun", unreal.StaticMesh),
    ("Anti-Armor Launcher", "/Game/R13/Weapons/rocketlauncherModern", unreal.StaticMesh),
)


def log(message):
    unreal.log(f"[OC Weapon Asset Preflight] {message}")


def is_placeholder_material(material):
    if material is None:
        return True
    path = str(material.get_path_name())
    lowered = path.lower()
    return (
        "/engine/enginematerials/defaultmaterial" in lowered
        or "/engine/basicshapes/basicshapematerial" in lowered
        or str(material.get_name()).lower() in ("defaultmaterial", "basicshapematerial")
    )


def material_interfaces(asset):
    if isinstance(asset, unreal.StaticMesh):
        slots = list(asset.get_editor_property("static_materials") or [])
    elif isinstance(asset, unreal.SkeletalMesh):
        slots = list(asset.get_editor_property("materials") or [])
    else:
        return []

    result = []
    for slot in slots:
        try:
            result.append(slot.get_editor_property("material_interface"))
        except Exception:
            result.append(None)
    return result


def main():
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    for sentinel in (SUCCESS_SENTINEL, MATERIAL_SUCCESS_SENTINEL):
        if sentinel.exists():
            sentinel.unlink()

    report = ["OSTER CONFLICT REQUIRED REAL WEAPON MESH PREFLIGHT"]
    material_report = ["OSTER CONFLICT REQUIRED WEAPON AUTHORED MATERIAL PREFLIGHT"]
    mesh_failures = []
    material_gaps = []

    for label, object_path, expected_type in REQUIRED_ASSETS:
        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        loaded = asset is not None
        type_ok = loaded and isinstance(asset, expected_type)
        mesh_result = "PASS" if type_ok else "FAIL"
        actual_type = asset.get_class().get_name() if loaded else "MISSING"
        report.append(
            f"{label} | path={object_path} | loaded={loaded} | "
            f"expected={expected_type.__name__} | actual={actual_type} | MESH_RESULT={mesh_result}"
        )
        if not type_ok:
            mesh_failures.append(f"{label}: {object_path} ({actual_type})")
            material_report.append(
                f"{label} | path={object_path} | MATERIAL_RESULT=UNAVAILABLE | reason=mesh_not_loaded"
            )
            material_gaps.append(f"{label}: mesh unavailable")
            continue

        materials = material_interfaces(asset)
        material_paths = [m.get_path_name() if m is not None else "NULL" for m in materials]
        placeholder_slots = [index for index, material in enumerate(materials) if is_placeholder_material(material)]
        authored_ready = bool(materials) and not placeholder_slots
        material_result = "PASS" if authored_ready else "GAP"
        material_report.append(
            f"{label} | path={object_path} | slots={len(materials)} | "
            f"placeholder_slots={placeholder_slots} | materials={material_paths} | MATERIAL_RESULT={material_result}"
        )
        if not authored_ready:
            material_gaps.append(
                f"{label}: slots={len(materials)} placeholder_slots={placeholder_slots} materials={material_paths}"
            )
            unreal.log_warning(
                f"[OC Weapon Asset Preflight] AUTHORED MATERIAL GAP {label}: "
                f"slots={len(materials)} placeholder_slots={placeholder_slots} materials={material_paths}"
            )

    report.append("")
    report.append(f"MESH_SUMMARY={len(REQUIRED_ASSETS) - len(mesh_failures)}/{len(REQUIRED_ASSETS)} PASS")
    REPORT_PATH.write_text("\n".join(report) + "\n", encoding="utf-8")

    material_report.append("")
    material_report.append(
        f"AUTHORED_MATERIAL_SUMMARY={len(REQUIRED_ASSETS) - len(material_gaps)}/{len(REQUIRED_ASSETS)} PASS"
    )
    MATERIAL_REPORT_PATH.write_text("\n".join(material_report) + "\n", encoding="utf-8")

    if mesh_failures:
        for failure in mesh_failures:
            unreal.log_error(f"[OC Weapon Asset Preflight] {failure}")
        unreal.log_error(
            f"[OC Weapon Asset Preflight] MESH PREFLIGHT FAILED. Report: {REPORT_PATH}"
        )
        return

    SUCCESS_SENTINEL.write_text("REQUIRED_REAL_WEAPON_MESHES=PASS\n", encoding="utf-8")
    log(f"MESH PASS: all {len(REQUIRED_ASSETS)} required real/playable weapon visuals load in a fresh UE process.")

    if material_gaps:
        log(
            f"AUTHORED MATERIAL STATUS: GAP on {len(material_gaps)}/{len(REQUIRED_ASSETS)} weapon visuals. "
            f"Normal geometry playtest may continue, but grey/default slots are NOT production-ready. "
            f"Report: {MATERIAL_REPORT_PATH}"
        )
    else:
        MATERIAL_SUCCESS_SENTINEL.write_text("REQUIRED_WEAPON_AUTHORED_MATERIALS=PASS\n", encoding="utf-8")
        log(f"AUTHORED MATERIAL PASS: all {len(REQUIRED_ASSETS)} weapon visuals have non-placeholder material slots.")


if __name__ == "__main__":
    main()
