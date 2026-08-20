import json
import shutil
import struct
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_ROOT = PROJECT_DIR / "SourceAssets" / "Production"
CACHE_ROOT = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"

HMMWV_SOURCE = SOURCE_ROOT / "Vehicles" / "HMMWV" / "ukrainian_hmmwv_mk_19.glb"
M2_SOURCE = SOURCE_ROOT / "Weapons" / "M2" / "m2_50cal_machinegun_cc0.glb"
BTR_SOURCE = SOURCE_ROOT / "Vehicles" / "BTR4" / "BTR4_Bucephalus.fbx"
BTR_TEXTURE_DIR = SOURCE_ROOT / "Vehicles" / "BTR4" / "Textures"

HMMWV_DEST = "/Game/Production/Vehicles/HMMWV"
M2_DEST = "/Game/Production/Weapons/M2"
BTR_DEST = "/Game/Production/Vehicles/BTR4"

HMMWV_NAME = "SM_HMMWV_UA"
M2_NAME = "SM_M2_Browning"
BTR_NAME = "SM_BTR4_Bucephalus"


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


def make_hmmwv_without_mk19(source, destination):
    """Detach Mk19 scene nodes while retaining all HMMWV geometry/material/buffer data."""
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
        log("HMMWV source contains no Mk19 node; importing source unchanged.")
    else:
        for node in nodes:
            if "children" in node:
                node["children"] = [child for child in node["children"] if child not in mk19_nodes]
        for scene in document.get("scenes", []):
            if "nodes" in scene:
                scene["nodes"] = [node for node in scene["nodes"] if node not in mk19_nodes]
        log(f"Detached Mk19 node(s) before HMMWV import: {sorted(mk19_nodes)}")

    chunks[json_index][1] = json.dumps(document, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    Path(destination).parent.mkdir(parents=True, exist_ok=True)
    _write_glb(destination, chunks)


def configure_ue58_interchange_static_mesh_pipeline(mesh_pipeline, common_meshes):
    """Configure the non-deprecated UE 5.8 Interchange controls used by our GLB ingest."""
    combine_behavior = getattr(unreal, "InterchangeCombineStaticMeshesBehavior", None)
    if combine_behavior is None or not hasattr(combine_behavior, "ALL"):
        fail("UE 5.8 InterchangeCombineStaticMeshesBehavior.ALL is unavailable; refusing ambiguous GLB import.")

    force_mesh_type = getattr(unreal, "InterchangeForceMeshType", None)
    if force_mesh_type is None or not hasattr(force_mesh_type, "IFMT_STATIC_MESH"):
        fail("UE 5.8 InterchangeForceMeshType.IFMT_STATIC_MESH is unavailable; refusing ambiguous GLB import.")

    # UE 5.8 deprecates the old combine_static_meshes bool. The behavior enum is the authoritative control.
    mesh_pipeline.set_editor_property("combine_static_meshes_behavior", combine_behavior.ALL)
    mesh_pipeline.set_editor_property("import_static_meshes", True)
    mesh_pipeline.set_editor_property("import_skeletal_meshes", False)
    mesh_pipeline.set_editor_property("collision", False)

    # Force rigid GLB scene geometry into one static-mesh import path even if the source carries
    # hierarchy/transform metadata that Interchange might otherwise attempt to classify differently.
    common_meshes.set_editor_property("force_all_mesh_as_type", force_mesh_type.IFMT_STATIC_MESH)
    common_meshes.set_editor_property("bake_meshes", True)
    common_meshes.set_editor_property("auto_detect_mesh_type", False)


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
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("async_", False)
    task.set_editor_property("options", stack)
    return task


def import_glb_combined(filename, destination_path, asset_name):
    log(f"Importing GLB {filename.name} -> {destination_path}/{asset_name}")
    task = make_interchange_task(filename, destination_path, asset_name)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset_path = f"{destination_path}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        imported = list(task.get_editor_property("imported_object_paths") or [])
        fail(f"Expected {asset_path} was not created. Imported: {imported}")
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    return asset_path


def import_btr_fbx(filename, texture_dir, destination_path, asset_name):
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

    # Keep BTR FBX on the mature legacy FBX importer. Interchange FBX support is still documented
    # as experimental in UE 5.8, while this path needs deterministic combined static-mesh output.
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
    asset_path = f"{destination_path}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        imported = list(task.get_editor_property("imported_object_paths") or [])
        fail(f"Expected {asset_path} was not created. Imported: {imported}")
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    return asset_path


def ensure_sources_exist():
    missing = [p for p in (HMMWV_SOURCE, M2_SOURCE, BTR_SOURCE) if not p.exists()]
    if missing:
        fail("Missing production source files: " + ", ".join(str(p) for p in missing))


def main():
    ensure_sources_exist()
    CACHE_ROOT.mkdir(parents=True, exist_ok=True)

    cleaned_hmmwv = CACHE_ROOT / "ukrainian_hmmwv_no_mk19.glb"
    make_hmmwv_without_mk19(HMMWV_SOURCE, cleaned_hmmwv)

    imported = [
        import_glb_combined(cleaned_hmmwv, HMMWV_DEST, HMMWV_NAME),
        import_glb_combined(M2_SOURCE, M2_DEST, M2_NAME),
        import_btr_fbx(BTR_SOURCE, BTR_TEXTURE_DIR, BTR_DEST, BTR_NAME),
    ]

    unreal.EditorAssetLibrary.save_directory("/Game/Production", only_if_is_dirty=False, recursive=True)
    log("Production vehicle import complete:")
    for path in imported:
        log(f"  {path}")


if __name__ == "__main__":
    main()
