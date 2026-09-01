#!/usr/bin/env python3
"""Isolated UE 5.8 editor import probe for the pinned Remington 870 donor.

Run inside UnrealEditor-Cmd, not normal CPython. This pilot deliberately does
not save imported packages, does not touch the production Remington path, and
does not alter gameplay/action authority. Its only job is to prove or reject
that the exact acquired GLB can become a skeletal mesh plus animation assets in
UE 5.8 using the engine-native import path.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

import unreal

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_BYTES = 20621580
EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
SOURCE_REL = Path("SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb")
MANIFEST_REL = Path("SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")
PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870_147aa6a0"


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_UE58_IMPORT_PILOT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def load_and_validate_source(root: Path) -> Path:
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
        "runtime_ready": False,
        "ue58_import_pending": True,
        "item16_checked": False,
    }
    for key, expected in required_manifest.items():
        actual = manifest.get(key)
        if actual != expected:
            fail(f"manifest_drift key={key} expected={expected!r} actual={actual!r}")

    return source


def imported_objects_from_destination() -> list[object]:
    objects: list[object] = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    ):
        obj = unreal.EditorAssetLibrary.load_asset(asset_path)
        if obj is not None:
            objects.append(obj)
    return objects


def main() -> None:
    engine_version = unreal.SystemLibrary.get_engine_version()
    if not engine_version.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine_version}")

    root = repo_root()
    source = load_and_validate_source(root)

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
    for obj in imported:
        class_name = obj.get_class().get_name()
        class_counts[class_name] = class_counts.get(class_name, 0) + 1
        asset_rows.append((class_name, obj.get_path_name()))

    skeletal_count = class_counts.get("SkeletalMesh", 0)
    animation_count = class_counts.get("AnimSequence", 0)
    if skeletal_count < 1:
        fail(
            "skeletal_mesh_missing=1 "
            f"imported_classes={json.dumps(class_counts, sort_keys=True)}"
        )
    if animation_count < 1:
        fail(
            "anim_sequence_missing=1 "
            f"imported_classes={json.dumps(class_counts, sort_keys=True)}"
        )

    for class_name, asset_path in sorted(asset_rows):
        unreal.log(
            "PASS45_REMINGTON870_UE58_IMPORT_PILOT_ASSET "
            f"class={class_name} path={asset_path}"
        )

    unreal.log(
        "PASS45_REMINGTON870_UE58_IMPORT_PILOT_PASS "
        f"engine={engine_version} source_sha256={EXPECTED_SOURCE_SHA256} "
        f"imported_objects={len(imported)} skeletal_meshes={skeletal_count} "
        f"anim_sequences={animation_count} destination={PILOT_DESTINATION} "
        "saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
