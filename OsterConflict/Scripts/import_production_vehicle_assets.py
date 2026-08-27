import json
import shutil
import struct
import sys
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_ROOT = PROJECT_DIR / "SourceAssets" / "Production"
CACHE_ROOT = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
SUCCESS_SENTINEL = CACHE_ROOT / "production_import_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_BTR_AXIS_OPTIC_20260827_R2"

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))
from generate_btr4_game_visual import build_btr4_glb

HMMWV_SOURCE = SOURCE_ROOT / "Vehicles" / "HMMWV" / "ukrainian_hmmwv_mk_19.glb"
M2_SOURCE = SOURCE_ROOT / "Weapons" / "M2" / "m2_50cal_machinegun_cc0.glb"
BTR_SOURCE = SOURCE_ROOT / "Vehicles" / "BTR4" / "BTR4_Bucephalus.fbx"
BTR_TEXTURE_DIR = SOURCE_ROOT / "Vehicles" / "BTR4" / "Textures"
BTR_GENERATED_SOURCE = CACHE_ROOT / "BTR4" / "btr4_bucephalus_oc_authored.glb"

HMMWV_DEST = "/Game/Production/Vehicles/HMMWV"
M2_DEST = "/Game/Production/Weapons/M2"
BTR_DEST = "/Game/Production/Vehicles/BTR4"

HMMWV_NAME = "SM_HMMWV_UA"
M2_NAME = "SM_M2_Browning"
BTR_NAME = "SM_BTR4_Bucephalus"

HMMWV_CANONICAL_ROTATION = [0.0, -0.7071067811865476, 0.0, 0.7071067811865476]


def log(message):
    unreal.log(f"[OC Production Import] {message}")


def fail(message):
    unreal.log_error(f"[OC Production Import] {message}")
    raise RuntimeError(message)


def _read_glb(path):
    data = Path(path).read_bytes()
    if len(data) < 12:
        fail(f"Invalid GLB header: {path}")
    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != 0x46546C67 or version != 2 or total_length != len(data):
        fail(f"Unsupported/invalid GLB: {path}")

    chunks = []
    offset = 12
    while offset < len(data):
        chunk_length, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        payload = data[offset:offset + chunk_length]
        offset += chunk_length
        chunks.append([chunk_type, payload])
    return chunks


def _write_glb(path, chunks):
    encoded_chunks = []
    for chunk_type, payload in chunks:
        if chunk_type == 0x4E4F534A:
            payload = payload.rstrip(b"\x00 ")
            payload += b" " * ((4 - len(payload) % 4) % 4)
        else:
            payload += b"\x00" * ((4 - len(payload) % 4) % 4)
        encoded_chunks.append(struct.pack("<II", len(payload), chunk_type) + payload)

    body = b"".join(encoded_chunks)
    header = struct.pack("<III", 0x46546C67, 2, 12 + len(body))
    Path(path).write_bytes(header + body)


def add_scene_root_rotation(document, node_name, rotation):
    nodes = document.setdefault("nodes", [])
    wrapped = 0
    for scene_index, scene in enumerate(document.get("scenes", [])):
        scene_roots = list(scene.get("nodes", []))
        if not scene_roots:
            continue
        root_index = len(nodes)
        nodes.append({
            "name": f"{node_name}_{scene_index}",
            "rotation": list(rotation),
            "children": scene_roots,
        })
        scene["nodes"] = [root_index]
        wrapped += 1
    if wrapped <= 0:
        fail("GLB contains no populated scene to orient for Unreal import.")
    return wrapped


def make_hmmwv_without_mk19(source, destination):
    chunks = _read_glb(source)
    json_index = next((i for i, chunk in enumerate(chunks) if chunk[0] == 0x4E4F534A), None)
    if json_index is None:
        fail(f"GLB has no JSON chunk: {source}")

    document = json.loads(chunks[json_index][1].decode("utf-8").rstrip("\x00 "))
    nodes = document.get("nodes", [])
    mk19_nodes = {
        index for index, node in enumerate(nodes)
        if "mk19" in str(node.get("name", "")).lower()
    }
    if not mk19_nodes:
        log("HMMWV source contains no Mk19 node; importing source unchanged apart from canonical orientation.")
    else:
        for node in nodes:
            if "children" in node:
                node["children"] = [child for child in node["children"] if child not in mk19_nodes]
        for scene in document.get("scenes", []):
            if "nodes" in scene:
                scene["nodes"] = [node for node in scene["nodes"] if node not in mk19_nodes]
        log(f"Detached Mk19 node(s) before HMMWV import: {sorted(mk19_nodes)}")

    wrapped = add_scene_root_rotation(document, "OC_HMMWV_CanonicalAxis", HMMWV_CANONICAL_ROTATION)
    log(f"Applied HMMWV canonical axis rotation to {wrapped} glTF scene(s).")

    chunks[json_index][1] = json.dumps(document, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    Path(destination).parent.mkdir(parents=True, exist_ok=True)
    _write_glb(destination, chunks)


def configure_ue58_interchange_static_mesh_pipeline(mesh_pipeline, common_meshes):
    combine_behavior = getattr(unreal, "InterchangeCombineStaticMeshesBehavior", None)
    if combine_behavior is None or not hasattr(combine_behavior, "ALL"):
        fail("UE 5.8 InterchangeCombineStaticMeshesBehavior.ALL is unavailable; refusing ambiguous GLB import.")

    force_mesh_type = getattr(unreal, "InterchangeForceMeshType", None)
    if force_mesh_type is None or not hasattr(force_mesh_type, "IFMT_STATIC_MESH"):
        fail("UE 5.8 InterchangeForceMeshType.IFMT_STATIC_MESH is unavailable; refusing ambiguous GLB import.")

    mesh_pipeline.set_editor_property("combine_static_meshes_behavior", combine_behavior.ALL)
    mesh_pipeline.set_editor_property("import_static_meshes", True)
    mesh_pipeline.set_editor_property("import_skeletal_meshes", False)
    mesh_pipeline.set_editor_property("collision", False)
    common_meshes.set_editor_property("force_all_mesh_as_type", force_mesh_type.IFMT_STATIC_MESH)
    common_meshes.set_editor_property("bake_meshes", True)
    common_meshes.set_editor_property("convert_statics_with_animated_transform_to_skeletals", False)


def make_interchange_task(filename, destination_path, asset_name):
    pipeline = unreal.InterchangeGenericAssetsPipeline()
    pipeline.set_editor_property("asset_name", asset_name)
    pipeline.set_editor_property("asset_type_sub_folders", False)
    pipeline.set_editor_property("scene_name_sub_folder", False)
    pipeline.set_editor_property("use_source_name_for_asset", False)

    mesh_pipeline = pipeline.get_editor_property("mesh_pipeline")
    common_meshes = pipeline.get_editor_property("common_meshes_properties")
    if mesh_pipeline is None or common_meshes is None:
        fail("UE 5.8 Interchange generic mesh pipeline is unavailable.")
    configure_ue58_interchange_static_mesh_pipeline(mesh_pipeline, common_meshes)

    stack = unreal.InterchangePipelineStackOverride()
    stack.add_pipeline(pipeline)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(filename))
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    task.set_editor_property("options", stack)
    return task


def verify_import_task_updated_asset(task, asset_path):
    imported = [str(path).replace("\\", "/") for path in list(task.get_editor_property("imported_object_paths") or [])]
    expected_object_prefix = f"{asset_path}."
    updated_expected_asset = any(
        path == asset_path or path.startswith(expected_object_prefix) or expected_object_prefix in path
        for path in imported
    )
    if not updated_expected_asset:
        fail(f"Import task did not report creating/updating {asset_path}. Imported/updated paths: {imported}")

    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        fail(f"Import task reported {asset_path}, but the canonical asset does not exist after import.")

    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    return asset_path


def import_glb_combined(filename, destination_path, asset_name):
    log(f"Importing GLB {filename.name} -> {destination_path}/{asset_name}")
    task = make_interchange_task(filename, destination_path, asset_name)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return verify_import_task_updated_asset(task, f"{destination_path}/{asset_name}")


def import_btr_fbx(filename, texture_dir, destination_path, asset_name):
    """Development-only helper retained for explicitly calibrated local experiments.

    PASS45 canonical runtime import does not call this helper until the local FBX has a factual
    forward-axis sign contract and redistribution provenance.
    """
    stage = CACHE_ROOT / "BTR4"
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True, exist_ok=True)
    staged_fbx = stage / filename.name
    shutil.copy2(filename, staged_fbx)
    if texture_dir.exists():
        for texture in texture_dir.iterdir():
            if texture.is_file() and texture.suffix.lower() in (".png", ".tga", ".jpg", ".jpeg"):
                shutil.copy2(texture, stage / texture.name)

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    static_data = options.get_editor_property("static_mesh_import_data")
    if static_data is None:
        fail("UE 5.8 FBX static-mesh import settings are unavailable.")
    static_data.set_editor_property("combine_meshes", True)
    static_data.set_editor_property("generate_lightmap_u_vs", True)
    static_data.set_editor_property("auto_generate_collision", False)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(staged_fbx))
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    task.set_editor_property("options", options)

    log(f"Importing FBX {filename.name} -> {destination_path}/{asset_name}")
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return verify_import_task_updated_asset(task, f"{destination_path}/{asset_name}")


def attempt(label, source, import_fn, gaps, imported, provenance, source_kind):
    if source is not None and not source.exists():
        gaps.append(f"{label}_SOURCE_MISSING={source}")
        unreal.log_warning(f"[OC Production Import] CONTENT GAP: {label} source missing: {source}")
        return
    try:
        imported_path = import_fn()
        imported.append(imported_path)
        provenance.append(f"SOURCE_KIND={label}:{source_kind}")
        if source is not None:
            provenance.append(f"SOURCE_PATH={label}:{source}")
    except Exception as exc:
        gaps.append(f"{label}_IMPORT_FAILED={exc}")
        unreal.log_error(f"[OC Production Import] {label} import failed but other independent assets will continue: {exc}")


def import_btr4(provenance):
    # PASS45 item 30: only the repository-authored fallback has a factual positive-X nose contract.
    # A local FBX may still be used manually for development, but never auto-promoted to canonical runtime
    # until its forward sign and redistribution provenance are explicitly verified.
    if BTR_SOURCE.exists():
        log(
            f"BTR-4 local FBX detected at {BTR_SOURCE}, but canonical import skips it because "
            "forward-axis sign/provenance are unverified."
        )

    BTR_GENERATED_SOURCE.parent.mkdir(parents=True, exist_ok=True)
    build_btr4_glb(BTR_GENERATED_SOURCE)
    if not BTR_GENERATED_SOURCE.is_file() or BTR_GENERATED_SOURCE.stat().st_size <= 0:
        fail("Repository-safe authored BTR-4 fallback GLB generation produced no usable file.")
    log(
        "BTR-4 importing repository-safe authored GLB fallback with explicit +X forward and "
        "M_BTR4_OC_Authored PBR material contracts."
    )
    imported_path = import_glb_combined(BTR_GENERATED_SOURCE, BTR_DEST, BTR_NAME)
    provenance.append("SOURCE_KIND=BTR4:authored_external_visual_canonical_plus_x")
    provenance.append(f"SOURCE_PATH=BTR4:{BTR_GENERATED_SOURCE}")
    provenance.append("BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored")
    provenance.append("BTR4_FORWARD_AXIS=+X")
    return imported_path


def main():
    CACHE_ROOT.mkdir(parents=True, exist_ok=True)
    if SUCCESS_SENTINEL.exists():
        SUCCESS_SENTINEL.unlink()

    imported = []
    gaps = []
    provenance = [f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}"]

    def import_hmmwv():
        cleaned_hmmwv = CACHE_ROOT / "ukrainian_hmmwv_no_mk19.glb"
        make_hmmwv_without_mk19(HMMWV_SOURCE, cleaned_hmmwv)
        return import_glb_combined(cleaned_hmmwv, HMMWV_DEST, HMMWV_NAME)

    attempt("HMMWV", HMMWV_SOURCE, import_hmmwv, gaps, imported, provenance, "canonical_glb_no_mk19")
    attempt("M2", M2_SOURCE, lambda: import_glb_combined(M2_SOURCE, M2_DEST, M2_NAME), gaps, imported, provenance, "canonical_glb")

    try:
        imported.append(import_btr4(provenance))
    except Exception as exc:
        gaps.append(f"BTR4_IMPORT_FAILED={exc}")
        unreal.log_error(f"[OC Production Import] BTR4 import failed but other independent assets will continue: {exc}")

    if not imported:
        fail("No production vehicle/weapon source could be imported. See CONTENT GAP messages above.")

    unreal.EditorAssetLibrary.save_directory("/Game/Production", only_if_is_dirty=False, recursive=True)
    log("Independent production import complete:")
    for path in imported:
        log(f"  IMPORTED {path}")
    for line in provenance:
        log(f"  {line}")
    for gap in gaps:
        log(f"  {gap}")

    sentinel_lines = [f"IMPORTED={path}" for path in imported]
    sentinel_lines.extend(provenance)
    sentinel_lines.extend(f"CONTENT_GAP={gap}" for gap in gaps)
    SUCCESS_SENTINEL.write_text("\n".join(sentinel_lines) + "\n", encoding="utf-8")
    log(f"Import result sentinel written: {SUCCESS_SENTINEL}")


if __name__ == "__main__":
    main()
