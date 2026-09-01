#!/usr/bin/env python3
"""Isolated UE 5.8 editor import probe for the pinned Remington 870 donor.

Run inside UnrealEditor-Cmd, not normal CPython. This pilot deliberately does
not save imported packages, does not touch the production Remington path, and
does not alter gameplay/action authority. Its only job is to prove or reject
that the exact acquired GLB preserves its pinned animated action set through
the engine-native UE 5.8 import path as skeletal + animation assets sharing a
usable skeleton.
"""
from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path

import unreal

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_BYTES = 20621580
EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
EXPECTED_DONOR_ANIMATIONS = 5
EXPECTED_DONOR_SKINS = 4
EXPECTED_DONOR_NODES = 109
EXPECTED_DONOR_MESHES = 6
EXPECTED_ACTION_CHANNELS = {
    2: ("fire", 71),
    3: ("easy_reload", 71),
    4: ("full_reload", 72),
}
SOURCE_REL = Path("SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb")
MANIFEST_REL = Path("SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")
PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870_147aa6a0"
GLB_JSON_CHUNK = 0x4E4F534A


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_UE58_IMPORT_PILOT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def parse_glb_json(source: Path) -> dict:
    with source.open("rb") as handle:
        header = handle.read(12)
        if len(header) != 12:
            fail("glb_header_truncated=1")
        magic, version, declared_length = struct.unpack("<4sII", header)
        if magic != b"glTF":
            fail(f"glb_magic_invalid actual={magic!r}")
        if version != 2:
            fail(f"glb_version_invalid expected=2 actual={version}")
        if declared_length != source.stat().st_size:
            fail(
                "glb_declared_length_mismatch=1 "
                f"declared={declared_length} actual={source.stat().st_size}"
            )

        while handle.tell() + 8 <= declared_length:
            chunk_header = handle.read(8)
            if len(chunk_header) != 8:
                fail("glb_chunk_header_truncated=1")
            chunk_length, chunk_type = struct.unpack("<II", chunk_header)
            payload = handle.read(chunk_length)
            if len(payload) != chunk_length:
                fail("glb_chunk_payload_truncated=1")
            if chunk_type == GLB_JSON_CHUNK:
                try:
                    return json.loads(payload.decode("utf-8").rstrip("\x00 \t\r\n"))
                except Exception as exc:
                    fail(f"glb_json_invalid error={exc}")

    fail("glb_json_chunk_missing=1")
    raise AssertionError("unreachable")


def validate_donor_action_set(source: Path, manifest: dict) -> dict[str, int]:
    doc = parse_glb_json(source)
    animations = doc.get("animations") or []
    skins = doc.get("skins") or []
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []

    expected_structure = {
        "animations": EXPECTED_DONOR_ANIMATIONS,
        "skins": EXPECTED_DONOR_SKINS,
        "nodes": EXPECTED_DONOR_NODES,
        "meshes": EXPECTED_DONOR_MESHES,
    }
    actual_structure = {
        "animations": len(animations),
        "skins": len(skins),
        "nodes": len(nodes),
        "meshes": len(meshes),
    }
    if actual_structure != expected_structure:
        fail(
            "donor_structure_drift=1 "
            f"expected={json.dumps(expected_structure, sort_keys=True)} "
            f"actual={json.dumps(actual_structure, sort_keys=True)}"
        )

    manifest_channels = manifest.get("proven_donor_action_channels") or {}
    for index, (semantic, expected_channels) in EXPECTED_ACTION_CHANNELS.items():
        clip = animations[index]
        channels = clip.get("channels") or []
        samplers = clip.get("samplers") or []
        if len(channels) != expected_channels:
            fail(
                "donor_action_channel_drift=1 "
                f"index={index} semantic={semantic} expected={expected_channels} actual={len(channels)}"
            )
        if not samplers:
            fail(f"donor_action_samplers_missing=1 index={index} semantic={semantic}")
        if any(not isinstance(channel.get("target"), dict) for channel in channels):
            fail(f"donor_action_target_missing=1 index={index} semantic={semantic}")

        manifest_key = f"{semantic}_index_{index}"
        if manifest_channels.get(manifest_key) != expected_channels:
            fail(
                "manifest_action_channel_drift=1 "
                f"key={manifest_key} expected={expected_channels} actual={manifest_channels.get(manifest_key)!r}"
            )

    return actual_structure


def load_and_validate_source(root: Path) -> tuple[Path, dict, dict[str, int]]:
    source = root / SOURCE_REL
    manifest_path = root / MANIFEST_REL
    if not source.is_file():
        fail(f"source_missing path={source}")
    if not manifest_path.is_file():
        fail(f"manifest_missing path={manifest_path}")

    with source.open("rb") as handle:
        prefix = handle.read(160)
    if prefix.startswith(b"version https://git-lfs.github.com/spec/v1"):
        fail("git_lfs_pointer_only=1 run_git_lfs_pull_before_ue_import=1")

    size = source.stat().st_size
    if size != EXPECTED_SOURCE_BYTES:
        fail(f"source_size_mismatch expected={EXPECTED_SOURCE_BYTES} actual={size}")

    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    if digest != EXPECTED_SOURCE_SHA256:
        fail(f"source_sha256_mismatch expected={EXPECTED_SOURCE_SHA256} actual={digest}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    required_manifest = {
        "weapon": "Remington870",
        "status": "APPROVED_FOR_UE_IMPORT",
        "source_sha256": EXPECTED_SOURCE_SHA256,
        "source_bytes": EXPECTED_SOURCE_BYTES,
        "rigged_or_articulated": True,
        "animation_capable": True,
        "donor_animation_count": EXPECTED_DONOR_ANIMATIONS,
        "donor_skin_count": EXPECTED_DONOR_SKINS,
        "donor_node_count": EXPECTED_DONOR_NODES,
        "donor_mesh_count": EXPECTED_DONOR_MESHES,
        "runtime_ready": False,
        "ue58_import_pending": True,
        "item16_checked": False,
    }
    for key, expected in required_manifest.items():
        actual = manifest.get(key)
        if actual != expected:
            fail(f"manifest_drift key={key} expected={expected!r} actual={actual!r}")

    structure = validate_donor_action_set(source, manifest)
    return source, manifest, structure


def imported_objects_from_destination() -> list[object]:
    objects: list[object] = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    ):
        obj = unreal.EditorAssetLibrary.load_asset(asset_path)
        if obj is not None:
            objects.append(obj)
    return objects


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
    try:
        return skeleton.get_path_name()
    except Exception:
        return str(skeleton)


def animation_play_length(obj: object) -> float:
    getter = getattr(obj, "get_play_length", None)
    if callable(getter):
        try:
            return float(getter())
        except Exception:
            pass
    for property_name in ("sequence_length", "play_length"):
        try:
            return float(obj.get_editor_property(property_name))
        except Exception:
            continue
    return -1.0


def main() -> None:
    engine_version = unreal.SystemLibrary.get_engine_version()
    if not engine_version.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine_version}")

    root = repo_root()
    source, _manifest, donor_structure = load_and_validate_source(root)

    preexisting = unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    )
    if preexisting:
        fail(
            "pilot_destination_not_clean=1 destructive_cleanup_refused=1 "
            f"destination={PILOT_DESTINATION} asset_count={len(preexisting)}"
        )

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", PILOT_DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("async", False)
    task.set_editor_property("save", False)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("replace_existing_settings", False)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    imported = list(task.get_objects())
    if not imported:
        imported = imported_objects_from_destination()
    if not imported:
        fail("no_imported_objects=1")

    class_counts: dict[str, int] = {}
    asset_rows: list[tuple[str, str]] = []
    skeletal_meshes: list[object] = []
    animation_assets: list[object] = []
    for obj in imported:
        class_name = obj.get_class().get_name()
        class_counts[class_name] = class_counts.get(class_name, 0) + 1
        asset_rows.append((class_name, obj.get_path_name()))
        if class_name == "SkeletalMesh":
            skeletal_meshes.append(obj)
        elif class_name == "AnimSequence":
            animation_assets.append(obj)

    if not skeletal_meshes:
        fail(
            "skeletal_mesh_missing=1 "
            f"imported_classes={json.dumps(class_counts, sort_keys=True)}"
        )
    if len(animation_assets) < EXPECTED_DONOR_ANIMATIONS:
        fail(
            "donor_animation_set_not_preserved=1 "
            f"expected_at_least={EXPECTED_DONOR_ANIMATIONS} actual={len(animation_assets)} "
            f"imported_classes={json.dumps(class_counts, sort_keys=True)}"
        )

    mesh_skeleton_paths = {path for path in (skeleton_path(obj) for obj in skeletal_meshes) if path}
    animation_skeleton_paths = {
        path for path in (skeleton_path(obj) for obj in animation_assets) if path
    }
    shared_skeleton_paths = mesh_skeleton_paths & animation_skeleton_paths
    if not shared_skeleton_paths:
        fail(
            "shared_skeleton_missing=1 "
            f"mesh_skeletons={json.dumps(sorted(mesh_skeleton_paths))} "
            f"animation_skeletons={json.dumps(sorted(animation_skeleton_paths))}"
        )

    positive_length_animations = 0
    for animation in animation_assets:
        length = animation_play_length(animation)
        if length > 0.0:
            positive_length_animations += 1
        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORT_PILOT_ANIMATION "
            f"path={animation.get_path_name()} skeleton={skeleton_path(animation) or 'NONE'} "
            f"play_length={length:.6f}"
        )
    if positive_length_animations < EXPECTED_DONOR_ANIMATIONS:
        fail(
            "nonempty_animation_set_not_preserved=1 "
            f"expected_at_least={EXPECTED_DONOR_ANIMATIONS} actual={positive_length_animations}"
        )

    for class_name, asset_path in sorted(asset_rows):
        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORT_PILOT_ASSET "
            f"class={class_name} path={asset_path}"
        )

    unreal.log(
        "PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS "
        f"engine={engine_version} source_sha256={EXPECTED_SOURCE_SHA256} "
        f"donor_structure={json.dumps(donor_structure, sort_keys=True)} "
        "donor_action_channels=71/71/72 "
        f"imported_objects={len(imported)} skeletal_meshes={len(skeletal_meshes)} "
        f"anim_sequences={len(animation_assets)} positive_length_animations={positive_length_animations} "
        f"shared_skeletons={len(shared_skeleton_paths)} imported_animation_set_preserved=1 "
        f"destination={PILOT_DESTINATION} "
        "saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
