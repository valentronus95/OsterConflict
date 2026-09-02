#!/usr/bin/env python3
"""Fail-closed UE 5.8 production importer for the derived Remington 870 pump assembly.

This script runs only inside UnrealEditor-Cmd against OsterConflict.uproject. It
rebuilds the deterministic CC-BY derivative from the exact registered donor,
reduces the derived source to the standalone pump clip, imports a combined rigid
base plus combined skeletal assembly, validates the pump bone/animation before
saving, then assigns stable production paths.

It does NOT claim gameplay/runtime acceptance and does NOT close PASS45 item 16.
"""
from __future__ import annotations

import hashlib
import json
import struct
import sys
from pathlib import Path

import unreal

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
EXPECTED_SOURCE_BYTES = 20621580
EXPECTED_FORE_END_VERTICES = 1170
EXPECTED_SIDE_SADDLE_VERTICES = 3241
PUMP_BONE = "PASS45_PumpForeEnd"
PUMP_ANIMATION_SOURCE_NAME = "PASS45_Remington870_PumpCycle"
PUMP_DURATION = 0.55
DURATION_TOLERANCE = 0.08
MIN_TRANSLATION_DELTA = 0.01
IMPORT_CONTRACT_REVISION = "PASS45_REMINGTON870_DERIVED_PUMP_PROD_R1"

SOURCE_REL = Path("SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb")
CACHE_REL = Path("OsterConflict/Saved/ProductionAssetImportCache/Remington870")
DERIVED_SOURCE_NAME = "remington_870_pass45_production_pump.glb"
DESTINATION = "/Game/Production/Weapons/Remington870"
SKELETAL_ASSET = f"{DESTINATION}/SKM_Remington870"
RIGID_ASSET = f"{DESTINATION}/SM_Remington870_Rigid"
SKELETON_ASSET = f"{DESTINATION}/SK_Remington870"
PUMP_ANIMATION_ASSET = f"{DESTINATION}/AN_Remington870_PumpCycle"
SAMPLE_TIMES = (0.0, 0.18, 0.28, 0.549)
GLB_JSON_CHUNK = 0x4E4F534A


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_PRODUCTION_IMPORT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def ensure_repo_importable(root: Path) -> None:
    text = str(root)
    if text not in sys.path:
        sys.path.insert(0, text)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def source_bytes(root: Path) -> bytes:
    path = root / SOURCE_REL
    if not path.is_file():
        fail(f"source_missing path={path}")
    data = path.read_bytes()
    if data.startswith(b"version https://git-lfs.github.com/spec/v1"):
        fail("source_is_git_lfs_pointer=1 hydrate_registered_donor_first=1")
    if len(data) != EXPECTED_SOURCE_BYTES:
        fail(f"source_size_mismatch expected={EXPECTED_SOURCE_BYTES} actual={len(data)}")
    digest = sha256_bytes(data)
    if digest != EXPECTED_SOURCE_SHA256:
        fail(f"source_sha256_mismatch expected={EXPECTED_SOURCE_SHA256} actual={digest}")
    return data


def productionize_glb(data: bytes) -> bytes:
    """Keep all geometry/materials/skins but retain only the authored PumpCycle animation."""
    if len(data) < 12:
        fail("derived_glb_header_truncated=1")
    magic, version, total_length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF" or version != 2 or total_length != len(data):
        fail("derived_glb_header_invalid=1")

    chunks: list[tuple[int, bytes]] = []
    offset = 12
    found_json = False
    while offset + 8 <= len(data):
        length, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        payload = data[offset:offset + length]
        offset += length
        if len(payload) != length:
            fail("derived_glb_chunk_truncated=1")
        if chunk_type == GLB_JSON_CHUNK:
            doc = json.loads(payload.decode("utf-8").rstrip("\x00 \t\r\n"))
            animations = list(doc.get("animations") or [])
            pump = [a for a in animations if str(a.get("name") or "") == PUMP_ANIMATION_SOURCE_NAME]
            if len(pump) != 1:
                fail(
                    f"production_pump_animation_not_unique expected=1 actual={len(pump)} "
                    f"animation_count={len(animations)}"
                )
            doc["animations"] = pump
            payload = json.dumps(doc, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
            payload += b" " * ((4 - len(payload) % 4) % 4)
            found_json = True
        elif len(payload) % 4:
            payload += b"\x00" * ((4 - len(payload) % 4) % 4)
        chunks.append((chunk_type, payload))

    if not found_json:
        fail("derived_glb_json_missing=1")
    body = b"".join(struct.pack("<II", len(payload), chunk_type) + payload for chunk_type, payload in chunks)
    return struct.pack("<4sII", b"glTF", 2, 12 + len(body)) + body


def class_name(obj: object) -> str:
    return obj.get_class().get_name()


def package_path(obj: object) -> str:
    return str(obj.get_path_name()).split(".", 1)[0]


def skeleton_path(obj: object) -> str:
    skeleton = None
    try:
        skeleton = obj.get_editor_property("skeleton")
    except Exception:
        getter = getattr(obj, "get_skeleton", None)
        if callable(getter):
            try:
                skeleton = getter()
            except Exception:
                skeleton = None
    if skeleton is None:
        return ""
    return str(skeleton.get_path_name())


def mesh_has_bone(mesh: object, bone_name: str) -> bool:
    getter = getattr(mesh, "get_bone_index", None)
    if callable(getter):
        try:
            return int(getter(unreal.Name(bone_name))) >= 0
        except Exception:
            pass
    parent_getter = getattr(mesh, "get_bone_parent", None)
    if callable(parent_getter):
        try:
            return str(parent_getter(unreal.Name(bone_name))) not in ("", "None")
        except Exception:
            pass
    return False


def animation_has_bone(animation: object, bone_name: str) -> bool:
    return bool(
        unreal.AnimationLibrary.does_bone_name_exist(animation, unreal.Name(bone_name))
    )


def animation_length(animation: object) -> float:
    getter = getattr(animation, "get_play_length", None)
    if callable(getter):
        try:
            return float(getter())
        except Exception:
            pass
    for prop in ("sequence_length", "play_length"):
        try:
            return float(animation.get_editor_property(prop))
        except Exception:
            continue
    return -1.0


def animation_motion(animation: object) -> tuple[bool, float]:
    length = animation_length(animation)
    if length <= 0.0 or not animation_has_bone(animation, PUMP_BONE):
        return False, 0.0
    poses = []
    for requested in SAMPLE_TIMES:
        sample_time = min(requested, max(length - 0.0001, 0.0))
        poses.append(
            unreal.AnimationLibrary.get_bone_pose_for_time(
                animation, unreal.Name(PUMP_BONE), sample_time, False
            )
        )
    first = poses[0]
    max_delta = 0.0
    for pose in poses[1:]:
        max_delta = max(max_delta, float((pose.translation - first.translation).length()))
    return max_delta > MIN_TRANSLATION_DELTA, max_delta


def validate_assets(skeletal: object, rigid: object, animation: object) -> tuple[float, float]:
    if class_name(skeletal) != "SkeletalMesh":
        fail(f"canonical_skeletal_wrong_class actual={class_name(skeletal)}")
    if class_name(rigid) != "StaticMesh":
        fail(f"canonical_rigid_wrong_class actual={class_name(rigid)}")
    if class_name(animation) != "AnimSequence":
        fail(f"canonical_animation_wrong_class actual={class_name(animation)}")
    if not mesh_has_bone(skeletal, PUMP_BONE):
        fail("canonical_skeletal_missing_pump_bone=1")
    if not animation_has_bone(animation, PUMP_BONE):
        fail("canonical_animation_missing_pump_bone=1")
    length = animation_length(animation)
    if abs(length - PUMP_DURATION) > DURATION_TOLERANCE:
        fail(f"canonical_pump_duration_invalid expected={PUMP_DURATION} actual={length}")
    moved, max_delta = animation_motion(animation)
    if not moved:
        fail(f"canonical_pump_motion_missing max_translation_delta={max_delta}")
    skel_skeleton = skeleton_path(skeletal)
    anim_skeleton = skeleton_path(animation)
    if not skel_skeleton or skel_skeleton != anim_skeleton:
        fail(
            "canonical_shared_skeleton_missing=1 "
            f"skeletal={skel_skeleton or 'NONE'} animation={anim_skeleton or 'NONE'}"
        )
    return length, max_delta


def canonical_assets() -> tuple[object, object, object] | None:
    paths = (SKELETAL_ASSET, RIGID_ASSET, PUMP_ANIMATION_ASSET)
    exists = [unreal.EditorAssetLibrary.does_asset_exist(path) for path in paths]
    if not any(exists):
        return None
    if not all(exists):
        fail(f"partial_canonical_production_assets=1 exists={dict(zip(paths, exists))}")
    loaded = tuple(unreal.EditorAssetLibrary.load_asset(path) for path in paths)
    if any(obj is None for obj in loaded):
        fail("canonical_asset_load_failed=1")
    return loaded  # type: ignore[return-value]


def configure_pipeline() -> object:
    pipeline = unreal.InterchangeGenericAssetsPipeline()
    pipeline.set_editor_property("asset_name", "PASS45_Remington870")
    pipeline.set_editor_property("asset_type_sub_folders", False)
    pipeline.set_editor_property("scene_name_sub_folder", False)
    pipeline.set_editor_property("use_source_name_for_asset", False)

    mesh_pipeline = pipeline.get_editor_property("mesh_pipeline")
    common_meshes = pipeline.get_editor_property("common_meshes_properties")
    common_skeletal = pipeline.get_editor_property("common_skeletal_meshes_and_animations_properties")
    animation_pipeline = pipeline.get_editor_property("animation_pipeline")
    if mesh_pipeline is None or common_meshes is None or common_skeletal is None or animation_pipeline is None:
        fail("ue58_interchange_pipeline_components_missing=1")

    if not hasattr(unreal, "InterchangeCombineSkeletalMeshesBehavior"):
        fail("ue58_combine_skeletal_enum_missing=1")
    if not hasattr(unreal, "InterchangeCombineStaticMeshesBehavior"):
        fail("ue58_combine_static_enum_missing=1")

    mesh_pipeline.set_editor_property("import_static_meshes", True)
    mesh_pipeline.set_editor_property("import_skeletal_meshes", True)
    mesh_pipeline.set_editor_property(
        "combine_static_meshes_behavior", unreal.InterchangeCombineStaticMeshesBehavior.ALL
    )
    mesh_pipeline.set_editor_property(
        "combine_skeletal_meshes_behavior", unreal.InterchangeCombineSkeletalMeshesBehavior.ALL
    )
    mesh_pipeline.set_editor_property("create_physics_asset", False)
    mesh_pipeline.set_editor_property("collision", False)

    common_meshes.set_editor_property("force_all_mesh_as_type", unreal.InterchangeForceMeshType.IFMT_NONE)
    common_meshes.set_editor_property("bake_meshes", True)
    common_skeletal.set_editor_property("import_meshes_in_bone_hierarchy", True)
    animation_pipeline.set_editor_property("import_animations", True)
    animation_pipeline.set_editor_property("import_bone_tracks", True)

    stack = unreal.InterchangePipelineStackOverride()
    stack.add_pipeline(pipeline)
    return stack


def imported_objects(task: object) -> list[object]:
    objects = list(task.get_objects())
    if objects:
        return objects
    result: list[object] = []
    for path in unreal.EditorAssetLibrary.list_assets(DESTINATION, recursive=True, include_folder=False):
        obj = unreal.EditorAssetLibrary.load_asset(path)
        if obj is not None:
            result.append(obj)
    return result


def rename_asset(obj: object, destination: str) -> object:
    old = package_path(obj)
    if old != destination:
        if unreal.EditorAssetLibrary.does_asset_exist(destination):
            fail(f"rename_destination_already_exists path={destination}")
        if not unreal.EditorAssetLibrary.rename_asset(old, destination):
            fail(f"rename_failed source={old} destination={destination}")
    loaded = unreal.EditorAssetLibrary.load_asset(destination)
    if loaded is None:
        fail(f"renamed_asset_reload_failed path={destination}")
    return loaded


def write_sentinel(cache: Path, *, derived_sha: str, production_sha: str, reused: bool,
                   animation_length_value: float, max_delta: float) -> None:
    cache.mkdir(parents=True, exist_ok=True)
    sentinel = cache / "remington870_import_success.txt"
    lines = [
        f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}",
        f"SOURCE_SHA256={EXPECTED_SOURCE_SHA256}",
        f"DERIVED_SHA256={derived_sha}",
        f"PRODUCTION_SOURCE_SHA256={production_sha}",
        f"SKELETAL={SKELETAL_ASSET}",
        f"RIGID={RIGID_ASSET}",
        f"PUMP_ANIMATION={PUMP_ANIMATION_ASSET}",
        f"PUMP_BONE={PUMP_BONE}",
        f"PUMP_PLAY_LENGTH={animation_length_value:.6f}",
        f"PUMP_MAX_TRANSLATION_DELTA={max_delta:.6f}",
        "PUMP_MOTION_PRESERVED=1",
        "SHARED_SKELETON_PRESERVED=1",
        "COMBINED_RIGID_AND_SKELETAL_ASSEMBLY=1",
        f"REUSED_VERIFIED_PRODUCTION={int(reused)}",
        "PRODUCTION_SOURCE_READY=1",
        "runtime_acceptance=0",
        "item16_checked=0",
    ]
    sentinel.write_text("\n".join(lines) + "\n", encoding="utf-8")
    unreal.log(f"PASS45_REMINGTON870_PRODUCTION_IMPORT_SENTINEL path={sentinel}")


def main() -> None:
    engine = unreal.SystemLibrary.get_engine_version()
    if not engine.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine}")

    root = repo_root()
    ensure_repo_importable(root)
    cache = root / CACHE_REL
    cache.mkdir(parents=True, exist_ok=True)
    sentinel = cache / "remington870_import_success.txt"
    if sentinel.exists():
        sentinel.unlink()

    try:
        import PASS45_REMINGTON870_DERIVED_PUMP_SOURCE as derived
    except Exception as exc:
        fail(f"derived_builder_import_failed error={exc}")

    registered = source_bytes(root)
    try:
        derived_bytes, manifest = derived.build_derived(registered)
    except Exception as exc:
        fail(f"derived_build_failed error={exc}")
    if manifest.get("source_sha256") != EXPECTED_SOURCE_SHA256:
        fail("derived_manifest_source_identity_drift=1")
    if manifest.get("low_y_vertex_count") != EXPECTED_FORE_END_VERTICES:
        fail(f"derived_fore_end_vertex_count_drift actual={manifest.get('low_y_vertex_count')}")
    if manifest.get("high_y_vertex_count") != EXPECTED_SIDE_SADDLE_VERTICES:
        fail(f"derived_side_saddle_vertex_count_drift actual={manifest.get('high_y_vertex_count')}")
    if manifest.get("derived_joint") != PUMP_BONE or manifest.get("derived_animation") != PUMP_ANIMATION_SOURCE_NAME:
        fail("derived_manifest_pump_contract_drift=1")

    derived_sha = sha256_bytes(derived_bytes)
    if derived_sha != manifest.get("derived_sha256"):
        fail("derived_sha256_mismatch=1")
    production_bytes = productionize_glb(derived_bytes)
    production_sha = sha256_bytes(production_bytes)
    production_source = cache / DERIVED_SOURCE_NAME
    production_source.write_bytes(production_bytes)

    existing = canonical_assets()
    if existing is not None:
        length, delta = validate_assets(*existing)
        write_sentinel(
            cache,
            derived_sha=derived_sha,
            production_sha=production_sha,
            reused=True,
            animation_length_value=length,
            max_delta=delta,
        )
        unreal.log(
            "PASS45_REMINGTON870_PRODUCTION_IMPORT_PASS reused_verified=1 "
            f"engine={engine} skeletal={SKELETAL_ASSET} rigid={RIGID_ASSET} "
            f"pump_animation={PUMP_ANIMATION_ASSET} production_source_ready=1 "
            "runtime_acceptance=0 item16_checked=0"
        )
        return

    preexisting = unreal.EditorAssetLibrary.list_assets(DESTINATION, recursive=True, include_folder=False)
    if preexisting:
        fail(
            "production_destination_not_clean_and_not_canonical=1 destructive_cleanup_refused=1 "
            f"asset_count={len(preexisting)}"
        )

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(production_source))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("save", False)
    task.set_editor_property("async_", False)
    task.set_editor_property("options", configure_pipeline())

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    objects = imported_objects(task)
    if not objects:
        fail("production_import_no_objects=1")

    counts: dict[str, int] = {}
    for obj in objects:
        counts[class_name(obj)] = counts.get(class_name(obj), 0) + 1
    unreal.log(f"PASS45_REMINGTON870_PRODUCTION_IMPORT_CLASSES {json.dumps(counts, sort_keys=True)}")

    static_meshes = [obj for obj in objects if class_name(obj) == "StaticMesh"]
    skeletal_meshes = [obj for obj in objects if class_name(obj) == "SkeletalMesh"]
    animations = [obj for obj in objects if class_name(obj) == "AnimSequence"]
    pump_meshes = [obj for obj in skeletal_meshes if mesh_has_bone(obj, PUMP_BONE)]
    pump_animations = []
    for animation in animations:
        length = animation_length(animation)
        moved, _delta = animation_motion(animation)
        if (
            animation_has_bone(animation, PUMP_BONE)
            and moved
            and abs(length - PUMP_DURATION) <= DURATION_TOLERANCE
        ):
            pump_animations.append(animation)

    if len(static_meshes) != 1:
        fail(f"combined_rigid_mesh_count_invalid expected=1 actual={len(static_meshes)}")
    if len(skeletal_meshes) != 1 or len(pump_meshes) != 1:
        fail(
            "combined_skeletal_mesh_count_invalid "
            f"skeletal={len(skeletal_meshes)} pump_meshes={len(pump_meshes)}"
        )
    if len(pump_animations) != 1:
        fail(f"standalone_pump_animation_count_invalid expected=1 actual={len(pump_animations)}")

    skeletal = skeletal_meshes[0]
    rigid = static_meshes[0]
    animation = pump_animations[0]
    imported_skeleton = skeleton_path(skeletal)
    if not imported_skeleton:
        fail("imported_skeleton_missing=1")
    skeleton_obj = unreal.EditorAssetLibrary.load_asset(imported_skeleton.split(".", 1)[0])
    if skeleton_obj is None or class_name(skeleton_obj) != "Skeleton":
        fail(f"imported_skeleton_asset_missing path={imported_skeleton}")

    skeleton_obj = rename_asset(skeleton_obj, SKELETON_ASSET)
    skeletal = rename_asset(skeletal, SKELETAL_ASSET)
    rigid = rename_asset(rigid, RIGID_ASSET)
    animation = rename_asset(animation, PUMP_ANIMATION_ASSET)

    unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)

    skeletal = unreal.EditorAssetLibrary.load_asset(SKELETAL_ASSET)
    rigid = unreal.EditorAssetLibrary.load_asset(RIGID_ASSET)
    animation = unreal.EditorAssetLibrary.load_asset(PUMP_ANIMATION_ASSET)
    if skeletal is None or rigid is None or animation is None:
        fail("canonical_assets_missing_after_save=1")
    length, delta = validate_assets(skeletal, rigid, animation)

    write_sentinel(
        cache,
        derived_sha=derived_sha,
        production_sha=production_sha,
        reused=False,
        animation_length_value=length,
        max_delta=delta,
    )
    unreal.log(
        "PASS45_REMINGTON870_PRODUCTION_IMPORT_PASS reused_verified=0 "
        f"engine={engine} skeletal={SKELETAL_ASSET} rigid={RIGID_ASSET} "
        f"pump_animation={PUMP_ANIMATION_ASSET} pump_bone={PUMP_BONE} "
        f"play_length={length:.6f} max_translation_delta={delta:.6f} "
        "combined_rigid_and_skeletal_assembly=1 production_source_ready=1 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
