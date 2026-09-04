import hashlib
import json
import re
from pathlib import Path

import unreal

from import_production_vehicle_assets import import_btr_fbx, import_glb_combined

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STATE_ROOT = PROJECT_DIR / "Saved" / "LocalModelInbox"
PREPARED = STATE_ROOT / "prepared_sources.json"
BINDINGS = STATE_ROOT / "runtime_bindings.json"
SUCCESS = STATE_ROOT / "runtime_bindings_success.txt"
CONTENT_ROOT = PROJECT_DIR / "Content"

QUANTUM_BODY = "/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter"
SUPPORTED_STATIC = {".fbx", ".glb", ".gltf", ".obj"}
SUPPORTED_CHARACTER = {".fbx", ".glb", ".gltf"}
SUPPORTED_UI = {".png", ".tga", ".jpg", ".jpeg", ".bmp", ".exr"}


def log(message):
    unreal.log(f"[OC All Local Inbox Import] {message}")


def warn(message):
    unreal.log_warning(f"[OC All Local Inbox Import] {message}")


def safe_slug(value: str, limit=72):
    value = re.sub(r"[^A-Za-z0-9_]+", "_", value).strip("_")
    return (value or "Asset")[:limit]


def stable_name(path: Path):
    digest = hashlib.sha1(str(path).encode("utf-8", errors="replace")).hexdigest()[:8]
    return f"{safe_slug(path.stem, 56)}_{digest}"


def category_for(text: str):
    v = text.lower()
    if re.search(r"hud|heads.?up|crosshair|reticle|minimap|health.?bar|ammo.?ui|overlay|interface|compass|scope.?ui", v): return "HUD_UI"
    if re.search(r"btr.?4|bucephal|буцеф", v): return "BTR4"
    if re.search(r"hmmwv|humvee|hummer", v): return "HMMWV"
    if re.search(r"(^|[^a-z0-9])m2([^a-z0-9]|$)|browning|50.?cal", v): return "M2"
    if re.search(r"m249|minimi", v): return "M249"
    if re.search(r"remington|870", v): return "REMINGTON870"
    if re.search(r"m16|m4a1|(^|[^a-z0-9])m4([^a-z0-9]|$)", v): return "M16_M4"
    if re.search(r"ak.?47|(^|[^a-z0-9])akm([^a-z0-9]|$)", v): return "AK47"
    if re.search(r"(^|[^a-z0-9])mp5([^a-z0-9]|$)", v): return "MP5"
    if re.search(r"m?1911", v): return "M1911"
    if re.search(r"m700|remington.?700", v): return "M700"
    if re.search(r"(^|[^a-z0-9])m14([^a-z0-9]|$)", v): return "M14"
    if re.search(r"mac.?10", v): return "MAC10"
    if re.search(r"tec.?9", v): return "TEC9"
    if re.search(r"lever|winchester", v): return "LEVER_ACTION"
    if re.search(r"rpg|launcher|rocket", v): return "LAUNCHER"
    if re.search(r"rifle|weapon|gun|pistol|shotgun|smg|sniper", v): return "WEAPON_OTHER"
    if re.search(r"pickup|pick.?up|technical|hilux|truck", v): return "PICKUP"
    if re.search(r"skin|character|soldier|human|mannequin|uniform|operator|fighter|персона|солдат|скін|людин", v): return "CHARACTER_SKIN"
    if re.search(r"tree|foliage|grass|bush|vegetation|plant|flower|mushroom|treestump|дерев|кущ|трава", v): return "FOLIAGE"
    if re.search(r"prop|furniture|chair|table|barrel|crate|fence|bridge|lamp|light|bench|ladder|plank|wheel|whell|bowl|cauldron|kettle|mug|spoon|bucket|pot|sack|cart|axe|boat|well|torch|hay|log|stone|мебл|проп|паркан", v): return "PROP"
    if re.search(r"river|canal|stream|pond|waterway", v): return "WATER_WORLD"
    if re.search(r"road|sidewalk|pavement|pathway", v): return "ROAD_WORLD"
    if re.search(r"terrain|ground|landscape|mud|moss|field", v): return "GROUND_WORLD"
    if re.search(r"building|house|home|hut|roof|wall|porch|balcony|shed|hovel|tower|museum|silpo|stadium|culture|college|street|town|village|будин|музей|стадіон|вулиц", v): return "BUILDING_WORLD"
    return "UNCLASSIFIED"


def load_asset(path):
    try:
        return unreal.EditorAssetLibrary.load_asset(path)
    except Exception:
        return None


def object_path_from_content_file(target: str):
    path = Path(target)
    try:
        relative = path.relative_to(CONTENT_ROOT)
    except Exception:
        return None
    if path.suffix.lower() not in (".uasset", ".umap"):
        return None
    return "/Game/" + relative.with_suffix("").as_posix()


def import_texture(source: Path, destination: str):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", stable_name(source))
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    paths = [str(x).split(".", 1)[0] for x in list(task.get_editor_property("imported_object_paths") or [])]
    return list(dict.fromkeys(paths))


def import_character_fbx(source: Path, destination: str, target_skeleton):
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    if target_skeleton is not None:
        try:
            options.set_editor_property("skeleton", target_skeleton)
        except Exception as exc:
            warn(f"Could not request Quantum skeleton for {source.name}: {exc}")

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", stable_name(source))
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return [str(x).split(".", 1)[0] for x in list(task.get_editor_property("imported_object_paths") or [])]


def import_generic(source: Path, destination: str, category: str):
    ext = source.suffix.lower()
    if category == "CHARACTER_SKIN" and ext == ".fbx":
        quantum = load_asset(QUANTUM_BODY)
        skeleton = None
        if isinstance(quantum, unreal.SkeletalMesh):
            try: skeleton = quantum.get_editor_property("skeleton")
            except Exception: skeleton = None
        return import_character_fbx(source, destination, skeleton)

    if ext in (".glb", ".gltf") and category == "CHARACTER_SKIN":
        # Let UE Interchange inspect skin/joint data instead of forcing StaticMesh. Acceptance below still
        # requires an actual SkeletalMesh on the active Quantum skeleton, so a statue cannot sneak through.
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source))
        task.set_editor_property("destination_path", destination)
        task.set_editor_property("destination_name", stable_name(source))
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("replace_existing_settings", True)
        task.set_editor_property("save", True)
        task.set_editor_property("async_", False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        return [str(x).split(".", 1)[0] for x in list(task.get_editor_property("imported_object_paths") or [])]

    if ext in (".glb", ".gltf"):
        path = import_glb_combined(source, destination, stable_name(source))
        return [path]

    if ext == ".fbx":
        path = import_btr_fbx(source, source.parent, destination, stable_name(source))
        return [path]

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", stable_name(source))
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return [str(x).split(".", 1)[0] for x in list(task.get_editor_property("imported_object_paths") or [])]


def skeleton_path(mesh):
    if not isinstance(mesh, unreal.SkeletalMesh): return None
    try:
        skel = mesh.get_editor_property("skeleton")
        return str(skel.get_path_name()) if skel else None
    except Exception:
        return None


def classify_loaded_asset(asset_path, source, category, quantum_skeleton_path, bindings, source_status):
    asset = load_asset(asset_path)
    if asset is None:
        source_status.append({"source": source, "category": category, "status": "UNBOUND", "reason": f"asset_load_failed:{asset_path}"})
        return

    if isinstance(asset, unreal.StaticMesh):
        if category == "CHARACTER_SKIN":
            source_status.append({"source": source, "category": category, "status": "UNBOUND", "reason": "character_imported_as_static_mesh", "asset": asset_path})
            return
        bindings["static_assets"].append({"path": asset_path, "category": category, "source": source})
        source_status.append({"source": source, "category": category, "status": "BOUND", "binding": "STATIC_RUNTIME_POOL", "asset": asset_path})
        return

    if isinstance(asset, unreal.SkeletalMesh):
        skel = skeleton_path(asset)
        compatible = bool(skel and quantum_skeleton_path and skel == quantum_skeleton_path)
        bindings["skeletal_assets"].append({"path": asset_path, "category": category, "source": source, "skeleton": skel, "character_compatible": compatible})
        if category == "CHARACTER_SKIN" and not compatible:
            source_status.append({"source": source, "category": category, "status": "UNBOUND", "reason": "character_skeleton_incompatible", "asset": asset_path, "skeleton": skel, "required_skeleton": quantum_skeleton_path})
        else:
            source_status.append({"source": source, "category": category, "status": "BOUND", "binding": "CHARACTER_PROFILE" if category == "CHARACTER_SKIN" else "SKELETAL_RUNTIME_POOL", "asset": asset_path})
        return

    if isinstance(asset, unreal.Texture2D):
        if category == "HUD_UI":
            bindings["hud_textures"].append({"path": asset_path, "source": source})
            source_status.append({"source": source, "category": category, "status": "BOUND", "binding": "HUD_OVERLAY_TEXTURE", "asset": asset_path})
        return

    # WidgetBlueprint is editor-only. load_blueprint_class gives the generated runtime UClass when applicable.
    if category == "HUD_UI":
        try:
            widget_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
        except Exception:
            widget_class = None
        if widget_class is not None:
            class_path = str(widget_class.get_path_name())
            if "UserWidget" in str(widget_class.get_super_class().get_name()) or "Widget" in str(widget_class.get_name()):
                bindings["hud_widget_classes"].append({"path": class_path, "source": source})
                source_status.append({"source": source, "category": category, "status": "BOUND", "binding": "HUD_WIDGET_CLASS", "asset": asset_path, "class": class_path})


def main():
    STATE_ROOT.mkdir(parents=True, exist_ok=True)
    if SUCCESS.exists(): SUCCESS.unlink()
    if not PREPARED.is_file():
        raise RuntimeError(f"prepared local inbox manifest missing: {PREPARED}")

    prepared = json.loads(PREPARED.read_text(encoding="utf-8-sig"))
    if prepared.get("status") not in ("PASS", "NO_INBOX"):
        raise RuntimeError(f"local inbox prepare status is not safe: {prepared.get('status')}")

    quantum = load_asset(QUANTUM_BODY)
    quantum_skeleton_path = skeleton_path(quantum)
    if not quantum_skeleton_path:
        warn("QuantumCharacter skeleton could not be resolved; local character skins cannot be accepted as playable yet.")

    bindings = {
        "schema": "oster-conflict-pass45-local-runtime-bindings-v1",
        "static_assets": [],
        "skeletal_assets": [],
        "hud_textures": [],
        "hud_widget_classes": [],
        "source_status": [],
        "unbound_models": [],
        "all_models_bound": False,
    }

    seen_source_asset = set()

    # First register UE-ready packages that were copied into Content while preserving package names.
    # Mesh-looking .uasset files are acceptance-owned individually. A material may be a dependency,
    # but a StaticMeshes/Meshes/SM_/SK_/SKM_ source is not allowed to disappear silently.
    expected_ue_model_sources = set()
    unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(["/Game"], True)
    for item in prepared.get("ue_packages", []):
        if str(item.get("extension", "")).lower() != ".uasset":
            continue
        source = item.get("source", "")
        target = item.get("target", "")
        hint = (str(source) + " " + str(target)).replace("\\", "/").lower()
        expects_runtime_model = (
            "/staticmeshes/" in hint or "/meshes/" in hint or
            re.search(r"(?:^|/)(?:sm_|sk_|skm_)[^/]*\.uasset$", hint) is not None
        )
        if expects_runtime_model:
            expected_ue_model_sources.add(str(source))

        asset_path = object_path_from_content_file(target)
        category = item.get("category") or category_for(source)
        if not asset_path:
            if expects_runtime_model:
                bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": "ue_model_package_path_unresolved"})
            continue

        before = len(bindings["source_status"])
        classify_loaded_asset(asset_path, source, category, quantum_skeleton_path, bindings, bindings["source_status"])
        if len(bindings["source_status"]) > before:
            seen_source_asset.add(source)
        elif expects_runtime_model:
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": "ue_model_package_not_loadable_as_runtime_mesh", "asset": asset_path})

    # Import every raw model. Each source gets its own deterministic destination so same-named downloads cannot collide.
    for item in prepared.get("raw_models", []):
        source = Path(item.get("source", ""))
        category = item.get("category") or category_for(str(source))
        if not source.is_file():
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": "raw_source_missing"})
            continue
        ext = source.suffix.lower()
        if category == "CHARACTER_SKIN": supported = ext in SUPPORTED_CHARACTER
        else: supported = ext in SUPPORTED_STATIC
        if not supported:
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": f"unsupported_model_format:{ext}"})
            continue

        archive_hint = item.get("archive") or str(source.parent)
        archive_slug = safe_slug(Path(archive_hint).stem, 36)
        destination = f"/Game/LocalInbox/Imported/{archive_slug}/{safe_slug(category, 28)}"
        try:
            imported_paths = import_generic(source, destination, category)
        except Exception as exc:
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": f"import_failed:{type(exc).__name__}:{exc}"})
            continue

        imported_paths = list(dict.fromkeys(p for p in imported_paths if p))
        if not imported_paths:
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": "import_returned_no_assets"})
            continue

        source_had_model_binding = False
        for asset_path in imported_paths:
            before = len(bindings["source_status"])
            classify_loaded_asset(asset_path, str(source), category, quantum_skeleton_path, bindings, bindings["source_status"])
            if len(bindings["source_status"]) > before:
                source_had_model_binding = True
        if not source_had_model_binding:
            bindings["source_status"].append({"source": str(source), "category": category, "status": "UNBOUND", "reason": f"imported_assets_not_runtime_model:{imported_paths}"})

    # Import explicit HUD images. Companion model textures are intentionally not promoted to HUD.
    for item in prepared.get("hud_images", []):
        source = Path(item.get("source", ""))
        if not source.is_file() or source.suffix.lower() not in SUPPORTED_UI:
            bindings["source_status"].append({"source": str(source), "category": "HUD_UI", "status": "UNBOUND", "reason": "unsupported_or_missing_hud_image"})
            continue
        try:
            paths = import_texture(source, "/Game/LocalInbox/UI")
        except Exception as exc:
            bindings["source_status"].append({"source": str(source), "category": "HUD_UI", "status": "UNBOUND", "reason": f"hud_import_failed:{type(exc).__name__}:{exc}"})
            continue
        if not paths:
            bindings["source_status"].append({"source": str(source), "category": "HUD_UI", "status": "UNBOUND", "reason": "hud_import_returned_no_asset"})
            continue
        for path in paths:
            classify_loaded_asset(path, str(source), "HUD_UI", quantum_skeleton_path, bindings, bindings["source_status"])

    # Deduplicate assets while preserving deterministic first-source preference.
    for key in ("static_assets", "skeletal_assets", "hud_textures", "hud_widget_classes"):
        unique = []
        seen = set()
        for entry in bindings[key]:
            path = entry.get("path")
            if path in seen: continue
            seen.add(path)
            unique.append(entry)
        bindings[key] = unique

    # Every raw model source must have a model binding. For UE packs, only actual mesh/widget assets count;
    # materials/textures are dependencies and are not falsely reported as separate 'unbound models'.
    model_sources = {str(item.get("source", "")) for item in prepared.get("raw_models", [])}
    model_sources.update(expected_ue_model_sources)
    for status in bindings["source_status"]:
        if status.get("status") == "UNBOUND" and (status.get("source") in model_sources or status.get("category") == "CHARACTER_SKIN"):
            bindings["unbound_models"].append(status)

    # A HUD asset is optional unless one was actually supplied. If supplied, it must bind.
    supplied_hud = bool(prepared.get("hud_images")) or any((x.get("category") == "HUD_UI" and str(x.get("extension", "")).lower() == ".uasset") for x in prepared.get("ue_packages", []))
    if supplied_hud and not (bindings["hud_textures"] or bindings["hud_widget_classes"]):
        bindings["unbound_models"].append({"source": "HUD_UI", "category": "HUD_UI", "status": "UNBOUND", "reason": "hud_supplied_but_no_runtime_binding"})

    compatible_skins = [x for x in bindings["skeletal_assets"] if x.get("category") == "CHARACTER_SKIN" and x.get("character_compatible")]
    bindings["summary"] = {
        "static_assets": len(bindings["static_assets"]),
        "skeletal_assets": len(bindings["skeletal_assets"]),
        "compatible_character_skins": len(compatible_skins),
        "hud_textures": len(bindings["hud_textures"]),
        "hud_widget_classes": len(bindings["hud_widget_classes"]),
        "unbound_models": len(bindings["unbound_models"]),
    }
    bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0

    BINDINGS.write_text(json.dumps(bindings, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if bindings["all_models_bound"]:
        SUCCESS.write_text("PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS\n", encoding="utf-8")
        log(f"PASS all local inbox models bound. summary={bindings['summary']} manifest={BINDINGS}")
    else:
        warn(f"GAP unbound local models={len(bindings['unbound_models'])}. manifest={BINDINGS}")


if __name__ == "__main__":
    main()
