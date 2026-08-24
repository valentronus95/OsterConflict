from pathlib import Path
import json

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_DIR = PROJECT_DIR / "Saved" / "AutomationReports" / "ProductionModels"
REPORT_PATH = REPORT_DIR / "required_weapon_asset_preflight.txt"
SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_asset_preflight_success.txt"
MATERIAL_REPORT_PATH = REPORT_DIR / "required_weapon_authored_material_preflight.txt"
MATERIAL_SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_authored_material_preflight_success.txt"
DEPENDENCY_REPORT_PATH = REPORT_DIR / "required_weapon_material_texture_dependencies.json"
DEPENDENCY_SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_material_texture_dependencies_success.txt"

# Mesh-load, material readiness and texture-dependency readiness are deliberately separate. A mesh that
# opens with DefaultMaterial or a white/unresolved authored slot is geometry evidence only.
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


def is_placeholder_texture(texture):
    if texture is None:
        return True
    lowered = str(texture.get_path_name()).lower()
    name = str(texture.get_name()).lower()
    return (
        "/engine/engineresources/defaulttexture" in lowered
        or "/engine/engineresources/whitesquaretexture" in lowered
        or name in ("defaulttexture", "whitesquaretexture")
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


def used_textures(material):
    """Return texture objects used by an authored material, fail-visible if UE cannot introspect it."""
    if material is None:
        return [], "material_null"
    try:
        textures = list(unreal.MaterialEditingLibrary.get_used_textures(material) or [])
        return textures, None
    except Exception as exc:
        return [], f"texture_introspection_error:{type(exc).__name__}:{exc}"


def inspect_material_texture_dependencies(material):
    if material is None:
        return {
            "material": "NULL",
            "textures": [],
            "missing_textures": [],
            "placeholder_textures": [],
            "texture_result": "UNAVAILABLE",
            "reason": "material_null",
        }

    material_path = str(material.get_path_name())
    textures, introspection_error = used_textures(material)
    texture_paths = []
    missing_texture_paths = []
    placeholder_texture_paths = []

    for texture in textures:
        if texture is None:
            missing_texture_paths.append("NULL")
            continue
        texture_path = str(texture.get_path_name())
        texture_paths.append(texture_path)
        if is_placeholder_texture(texture):
            placeholder_texture_paths.append(texture_path)
            continue

        # Fresh-load the dependency by object path. This catches redirector/package/dependency problems that
        # a material already resident in memory can otherwise mask.
        loaded_texture = unreal.EditorAssetLibrary.load_asset(texture_path)
        if loaded_texture is None:
            missing_texture_paths.append(texture_path)

    # A production firearm material with no discoverable texture dependency is kept fail-visible. This does not
    # mean the material is necessarily broken, but it is not sufficient evidence to close the user's white-model bug.
    ready = (
        introspection_error is None
        and bool(texture_paths)
        and not missing_texture_paths
        and not placeholder_texture_paths
    )

    reason = "ready"
    if introspection_error:
        reason = introspection_error
    elif not texture_paths:
        reason = "no_texture_dependencies_discovered"
    elif missing_texture_paths:
        reason = "missing_texture_dependency"
    elif placeholder_texture_paths:
        reason = "placeholder_texture_dependency"

    return {
        "material": material_path,
        "textures": texture_paths,
        "missing_textures": missing_texture_paths,
        "placeholder_textures": placeholder_texture_paths,
        "texture_result": "PASS" if ready else "GAP",
        "reason": reason,
    }


def main():
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    for sentinel in (SUCCESS_SENTINEL, MATERIAL_SUCCESS_SENTINEL, DEPENDENCY_SUCCESS_SENTINEL):
        if sentinel.exists():
            sentinel.unlink()

    report = ["OSTER CONFLICT REQUIRED REAL WEAPON MESH PREFLIGHT"]
    material_report = ["OSTER CONFLICT REQUIRED WEAPON AUTHORED MATERIAL PREFLIGHT"]
    dependency_report = {
        "schema": "oster-conflict-pass45-weapon-material-texture-dependencies-v1",
        "status": "IN_PROGRESS",
        "weapons": [],
    }
    mesh_failures = []
    material_gaps = []
    dependency_gaps = []

    for label, object_path, expected_type in REQUIRED_ASSETS:
        weapon_dependency = {
            "label": label,
            "mesh": object_path,
            "mesh_result": "FAIL",
            "slot_count": 0,
            "materials": [],
            "material_result": "UNAVAILABLE",
            "texture_dependency_result": "UNAVAILABLE",
        }

        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        loaded = asset is not None
        type_ok = loaded and isinstance(asset, expected_type)
        mesh_result = "PASS" if type_ok else "FAIL"
        actual_type = asset.get_class().get_name() if loaded else "MISSING"
        weapon_dependency["mesh_result"] = mesh_result
        weapon_dependency["actual_type"] = actual_type
        report.append(
            f"{label} | path={object_path} | loaded={loaded} | "
            f"expected={expected_type.__name__} | actual={actual_type} | MESH_RESULT={mesh_result}"
        )
        if not type_ok:
            mesh_failures.append(f"{label}: {object_path} ({actual_type})")
            material_report.append(
                f"{label} | path={object_path} | MATERIAL_RESULT=UNAVAILABLE | "
                f"TEXTURE_RESULT=UNAVAILABLE | reason=mesh_not_loaded"
            )
            material_gaps.append(f"{label}: mesh unavailable")
            dependency_gaps.append(f"{label}: mesh unavailable")
            dependency_report["weapons"].append(weapon_dependency)
            continue

        materials = material_interfaces(asset)
        material_paths = [m.get_path_name() if m is not None else "NULL" for m in materials]
        placeholder_slots = [index for index, material in enumerate(materials) if is_placeholder_material(material)]
        authored_ready = bool(materials) and not placeholder_slots
        material_result = "PASS" if authored_ready else "GAP"

        dependency_slots = []
        for slot_index, material in enumerate(materials):
            dependency = inspect_material_texture_dependencies(material)
            dependency["slot"] = slot_index
            dependency_slots.append(dependency)

        texture_dependencies_ready = (
            bool(dependency_slots)
            and all(slot["texture_result"] == "PASS" for slot in dependency_slots)
        )
        texture_result = "PASS" if texture_dependencies_ready else "GAP"

        weapon_dependency.update({
            "slot_count": len(materials),
            "placeholder_slots": placeholder_slots,
            "materials": dependency_slots,
            "material_result": material_result,
            "texture_dependency_result": texture_result,
        })
        dependency_report["weapons"].append(weapon_dependency)

        material_report.append(
            f"{label} | path={object_path} | slots={len(materials)} | "
            f"placeholder_slots={placeholder_slots} | materials={material_paths} | "
            f"MATERIAL_RESULT={material_result} | TEXTURE_RESULT={texture_result}"
        )

        if not authored_ready:
            material_gaps.append(
                f"{label}: slots={len(materials)} placeholder_slots={placeholder_slots} materials={material_paths}"
            )
            unreal.log_warning(
                f"[OC Weapon Asset Preflight] AUTHORED MATERIAL GAP {label}: "
                f"slots={len(materials)} placeholder_slots={placeholder_slots} materials={material_paths}"
            )

        if not texture_dependencies_ready:
            gap_slots = [
                {
                    "slot": slot["slot"],
                    "material": slot["material"],
                    "reason": slot["reason"],
                    "missing": slot["missing_textures"],
                    "placeholder": slot["placeholder_textures"],
                }
                for slot in dependency_slots
                if slot["texture_result"] != "PASS"
            ]
            dependency_gaps.append(f"{label}: {gap_slots}")
            unreal.log_warning(
                f"[OC Weapon Asset Preflight] TEXTURE DEPENDENCY GAP {label}: {gap_slots}"
            )

    report.append("")
    report.append(f"MESH_SUMMARY={len(REQUIRED_ASSETS) - len(mesh_failures)}/{len(REQUIRED_ASSETS)} PASS")
    REPORT_PATH.write_text("\n".join(report) + "\n", encoding="utf-8")

    material_report.append("")
    material_report.append(
        f"AUTHORED_MATERIAL_SUMMARY={len(REQUIRED_ASSETS) - len(material_gaps)}/{len(REQUIRED_ASSETS)} PASS"
    )
    material_report.append(
        f"TEXTURE_DEPENDENCY_SUMMARY={len(REQUIRED_ASSETS) - len(dependency_gaps)}/{len(REQUIRED_ASSETS)} PASS"
    )
    MATERIAL_REPORT_PATH.write_text("\n".join(material_report) + "\n", encoding="utf-8")

    dependency_report["summary"] = {
        "required_weapons": len(REQUIRED_ASSETS),
        "mesh_pass": len(REQUIRED_ASSETS) - len(mesh_failures),
        "authored_material_pass": len(REQUIRED_ASSETS) - len(material_gaps),
        "texture_dependency_pass": len(REQUIRED_ASSETS) - len(dependency_gaps),
        "mesh_failures": mesh_failures,
        "material_gaps": material_gaps,
        "texture_dependency_gaps": dependency_gaps,
    }
    dependency_report["status"] = (
        "PASS" if not mesh_failures and not material_gaps and not dependency_gaps else "GAP"
    )
    DEPENDENCY_REPORT_PATH.write_text(
        json.dumps(dependency_report, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

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
            f"Normal geometry playtest may continue, but white/default slots are NOT production-ready. "
            f"Report: {MATERIAL_REPORT_PATH}"
        )
    else:
        MATERIAL_SUCCESS_SENTINEL.write_text("REQUIRED_WEAPON_AUTHORED_MATERIALS=PASS\n", encoding="utf-8")
        log(f"AUTHORED MATERIAL PASS: all {len(REQUIRED_ASSETS)} weapon visuals have non-placeholder material slots.")

    if dependency_gaps:
        log(
            f"TEXTURE DEPENDENCY STATUS: GAP on {len(dependency_gaps)}/{len(REQUIRED_ASSETS)} weapon visuals. "
            f"Dependency report: {DEPENDENCY_REPORT_PATH}"
        )
    else:
        DEPENDENCY_SUCCESS_SENTINEL.write_text(
            "REQUIRED_WEAPON_MATERIAL_TEXTURE_DEPENDENCIES=PASS\n", encoding="utf-8"
        )
        log(
            f"TEXTURE DEPENDENCY PASS: all {len(REQUIRED_ASSETS)} weapon visuals have fresh-loadable, "
            f"non-placeholder texture dependencies for every material slot."
        )

    log(
        "PASS45_WEAPON_DEPENDENCY_AUDIT_COMPLETE "
        f"mesh={len(REQUIRED_ASSETS) - len(mesh_failures)}/{len(REQUIRED_ASSETS)} "
        f"material={len(REQUIRED_ASSETS) - len(material_gaps)}/{len(REQUIRED_ASSETS)} "
        f"textures={len(REQUIRED_ASSETS) - len(dependency_gaps)}/{len(REQUIRED_ASSETS)} "
        f"report={DEPENDENCY_REPORT_PATH}"
    )


if __name__ == "__main__":
    main()
