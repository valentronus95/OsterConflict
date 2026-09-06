#!/usr/bin/env python3
"""Isolated UE 5.8 import/motion pilot for the derived Remington 870 pump source.

Run inside UnrealEditor-Cmd using the isolated Pass45 commandlet project. The
script rebuilds the deterministic CC-BY derivative from the exact registered
8sianDude donor, imports it without saving packages, and proves that UE 5.8
preserves the new PASS45_PumpForeEnd bone plus a non-trivial standalone pump
AnimSequence on a compatible skeleton.

This is DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY. It does not touch the production
Remington asset path, gameplay profile, runtime authority, PR merge state, or
PASS45 item-16 acceptance.
"""
from __future__ import annotations

import hashlib
import json
import sys
from pathlib import Path

import unreal

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
EXPECTED_SOURCE_BYTES = 20621580
EXPECTED_FORE_END_VERTICES = 1170
EXPECTED_SIDE_SADDLE_VERTICES = 3241
EXPECTED_PUMP_DURATION = 0.55
DURATION_TOLERANCE = 0.08
MIN_TRANSLATION_DELTA = 0.01
PUMP_BONE = "PASS45_PumpForeEnd"
PUMP_ANIMATION = "PASS45_Remington870_PumpCycle"
SOURCE_REL = Path("SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb")
DERIVED_WORK_REL = Path("OsterConflict/Saved/PASS45/Remington870DerivedPumpUE58")
EVIDENCE_REL = Path("OsterConflict/Saved/PASS45/PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.json")
PILOT_DESTINATION = "/Game/PASS45/ImportPilots/Remington870DerivedPump"
SAMPLE_TIMES = (0.0, 0.18, 0.28, 0.549)


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def ensure_repo_importable(root: Path) -> None:
    root_text = str(root)
    if root_text not in sys.path:
        sys.path.insert(0, root_text)


def source_identity(source: Path) -> dict[str, object]:
    if not source.is_file():
        fail(f"source_missing path={source}")
    prefix = source.read_bytes()[:160]
    if prefix.startswith(b"version https://git-lfs.github.com/spec/v1"):
        fail("git_lfs_pointer_only=1 run_normal_lfs_fetch_before_ue_pilot=1")
    size = source.stat().st_size
    if size != EXPECTED_SOURCE_BYTES:
        fail(f"source_size_mismatch expected={EXPECTED_SOURCE_BYTES} actual={size}")
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    if digest != EXPECTED_SOURCE_SHA256:
        fail(f"source_sha256_mismatch expected={EXPECTED_SOURCE_SHA256} actual={digest}")
    return {"bytes": size, "sha256": digest}


def class_name(obj: object) -> str:
    return obj.get_class().get_name()


def imported_objects() -> list[object]:
    objects: list[object] = []
    for asset_path in unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    ):
        obj = unreal.EditorAssetLibrary.load_asset(asset_path)
        if obj is not None:
            objects.append(obj)
    return objects


def animation_length(animation: object) -> float:
    getter = getattr(animation, "get_play_length", None)
    if callable(getter):
        try:
            return float(getter())
        except Exception:
            pass
    for property_name in ("sequence_length", "play_length"):
        try:
            return float(animation.get_editor_property(property_name))
        except Exception:
            continue
    return -1.0


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


def bone_exists_in_animation(animation: object, bone_name: str) -> bool:
    if not hasattr(unreal, "AnimationLibrary"):
        fail("animation_library_unavailable=1")
    return bool(
        unreal.AnimationLibrary.does_bone_name_exist(
            animation,
            unreal.Name(bone_name),
        )
    )


def mesh_has_bone(mesh: object, bone_name: str) -> bool:
    get_bone_index = getattr(mesh, "get_bone_index", None)
    if callable(get_bone_index):
        try:
            index = int(get_bone_index(unreal.Name(bone_name)))
            return index >= 0
        except Exception:
            pass
    get_bone_parent = getattr(mesh, "get_bone_parent", None)
    if callable(get_bone_parent):
        try:
            parent = str(get_bone_parent(unreal.Name(bone_name)))
            return parent not in ("", "None")
        except Exception:
            pass
    return False


def sample_pump_motion(animation: object, play_length: float) -> tuple[bool, float, list[dict[str, object]]]:
    if not bone_exists_in_animation(animation, PUMP_BONE):
        return False, 0.0, []
    if play_length <= 0.0:
        return False, 0.0, []

    samples: list[dict[str, object]] = []
    poses = []
    for requested_time in SAMPLE_TIMES:
        sample_time = min(requested_time, max(play_length - 0.0001, 0.0))
        pose = unreal.AnimationLibrary.get_bone_pose_for_time(
            animation,
            unreal.Name(PUMP_BONE),
            sample_time,
            False,
        )
        poses.append(pose)
        translation = pose.translation
        samples.append({
            "time": round(sample_time, 6),
            "translation": [float(translation.x), float(translation.y), float(translation.z)],
            "rotation": str(pose.rotation),
        })

    first = poses[0]
    max_delta = 0.0
    for pose in poses[1:]:
        delta = pose.translation - first.translation
        magnitude = float(delta.length())
        max_delta = max(max_delta, magnitude)
    return max_delta > MIN_TRANSLATION_DELTA, max_delta, samples


def write_evidence(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> None:
    engine_version = unreal.SystemLibrary.get_engine_version()
    if not engine_version.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine_version}")

    root = repo_root()
    ensure_repo_importable(root)
    try:
        import PASS45_REMINGTON870_DERIVED_PUMP_SOURCE as derived
    except Exception as exc:
        fail(f"derived_builder_import_failed error={exc}")

    source = root / SOURCE_REL
    identity = source_identity(source)
    derived_dir = root / DERIVED_WORK_REL
    derived_dir.mkdir(parents=True, exist_ok=True)

    try:
        glb_bytes, manifest = derived.build_derived(source.read_bytes())
    except Exception as exc:
        fail(f"derived_source_build_failed error={exc}")

    if manifest.get("source_sha256") != EXPECTED_SOURCE_SHA256:
        fail("derived_manifest_source_identity_drift=1")
    if manifest.get("low_y_vertex_count") != EXPECTED_FORE_END_VERTICES:
        fail(f"derived_fore_end_vertex_count_drift actual={manifest.get('low_y_vertex_count')}")
    if manifest.get("high_y_vertex_count") != EXPECTED_SIDE_SADDLE_VERTICES:
        fail(f"derived_side_saddle_vertex_count_drift actual={manifest.get('high_y_vertex_count')}")
    if manifest.get("derived_joint") != PUMP_BONE:
        fail(f"derived_joint_drift actual={manifest.get('derived_joint')!r}")
    if manifest.get("derived_animation") != PUMP_ANIMATION:
        fail(f"derived_animation_drift actual={manifest.get('derived_animation')!r}")
    if manifest.get("production_cutover") is not False or manifest.get("item16_checked") is not False:
        fail("derived_source_false_acceptance=1")

    derived_source = derived_dir / derived.DERIVED_SOURCE_NAME
    derived_source.write_bytes(glb_bytes)
    derived_digest = hashlib.sha256(glb_bytes).hexdigest()
    if derived_digest != manifest.get("derived_sha256"):
        fail("derived_sha256_mismatch=1")

    preexisting = unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    )
    if preexisting:
        fail(
            "pilot_destination_not_clean=1 destructive_cleanup_refused=1 "
            f"destination={PILOT_DESTINATION} asset_count={len(preexisting)}"
        )

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(derived_source))
    task.set_editor_property("destination_path", PILOT_DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("replace_existing_settings", False)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])

    imported = list(task.get_objects())
    if not imported:
        imported = imported_objects()
    if not imported:
        fail("derived_import_no_objects=1")

    skeletal_meshes = [obj for obj in imported if class_name(obj) == "SkeletalMesh"]
    animations = [obj for obj in imported if class_name(obj) == "AnimSequence"]
    if not skeletal_meshes:
        fail("derived_import_skeletal_mesh_missing=1")
    if not animations:
        fail("derived_import_animation_missing=1")

    pump_meshes = [mesh for mesh in skeletal_meshes if mesh_has_bone(mesh, PUMP_BONE)]
    if not pump_meshes:
        fail("derived_pump_bone_not_addressable_on_skeletal_mesh=1")

    mesh_skeletons = {skeleton_path(mesh) for mesh in pump_meshes if skeleton_path(mesh)}
    moving_sequences: list[dict[str, object]] = []
    animation_rows: list[dict[str, object]] = []
    for animation in animations:
        play_length = animation_length(animation)
        addressable = bone_exists_in_animation(animation, PUMP_BONE)
        moved, max_delta, samples = sample_pump_motion(animation, play_length)
        row = {
            "path": animation.get_path_name(),
            "name": animation.get_name(),
            "skeleton": skeleton_path(animation),
            "play_length": play_length,
            "pump_bone_addressable": addressable,
            "pump_bone_moved": moved,
            "max_translation_delta": max_delta,
            "samples": samples,
        }
        animation_rows.append(row)
        unreal.log(
            "PASS45_REMINGTON870_DERIVED_PUMP_UE58_SEQUENCE "
            f"path={row['path']} play_length={play_length:.6f} "
            f"pump_bone_addressable={int(addressable)} pump_bone_moved={int(moved)} "
            f"max_translation_delta={max_delta:.6f}"
        )
        if moved:
            moving_sequences.append(row)

    if not moving_sequences:
        fail("derived_pump_nontrivial_motion_not_preserved=1")

    compatible_moving = [
        row for row in moving_sequences
        if row.get("skeleton") in mesh_skeletons
    ]
    if not compatible_moving:
        fail(
            "derived_pump_shared_skeleton_missing=1 "
            f"mesh_skeletons={json.dumps(sorted(mesh_skeletons))}"
        )

    duration_candidates = [
        row for row in compatible_moving
        if abs(float(row.get("play_length", -1.0)) - EXPECTED_PUMP_DURATION) <= DURATION_TOLERANCE
    ]
    if len(duration_candidates) != 1:
        fail(
            "derived_standalone_pump_sequence_not_unique=1 "
            f"expected_duration={EXPECTED_PUMP_DURATION} candidates={len(duration_candidates)} "
            f"moving_sequences={len(compatible_moving)}"
        )

    accepted = duration_candidates[0]
    accepted_name = str(accepted.get("name") or "")
    accepted_path = str(accepted.get("path") or "")
    if PUMP_ANIMATION.lower() not in (accepted_name + " " + accepted_path).lower():
        fail(
            "derived_standalone_pump_sequence_identity_not_preserved=1 "
            f"expected_name={PUMP_ANIMATION} actual={accepted_name} path={accepted_path}"
        )

    evidence = {
        "schema": 1,
        "status": "DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY",
        "engine_version": engine_version,
        "source_sha256": identity["sha256"],
        "source_bytes": identity["bytes"],
        "derived_sha256": derived_digest,
        "derived_joint": PUMP_BONE,
        "derived_animation": PUMP_ANIMATION,
        "fore_end_vertices": EXPECTED_FORE_END_VERTICES,
        "side_saddle_vertices": EXPECTED_SIDE_SADDLE_VERTICES,
        "imported_skeletal_meshes": [mesh.get_path_name() for mesh in skeletal_meshes],
        "pump_bone_meshes": [mesh.get_path_name() for mesh in pump_meshes],
        "animations": animation_rows,
        "accepted_pump_sequence": accepted,
        "pump_bone_addressable": True,
        "pump_motion_preserved": True,
        "shared_skeleton_preserved": True,
        "standalone_pump_sequence_preserved": True,
        "source_partition_verified": True,
        "saved_packages": False,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    evidence_path = root / EVIDENCE_REL
    write_evidence(evidence_path, evidence)

    unreal.log(
        "PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT_PASS "
        f"engine={engine_version} source_sha256={EXPECTED_SOURCE_SHA256} "
        f"derived_sha256={derived_digest} pump_bone={PUMP_BONE} "
        f"fore_end_vertices={EXPECTED_FORE_END_VERTICES} side_saddle_vertices={EXPECTED_SIDE_SADDLE_VERTICES} "
        f"accepted_sequence={accepted_path} play_length={float(accepted['play_length']):.6f} "
        f"max_translation_delta={float(accepted['max_translation_delta']):.6f} "
        "pump_bone_addressable=1 pump_motion_preserved=1 shared_skeleton_preserved=1 "
        "standalone_pump_sequence_preserved=1 source_partition_verified=1 "
        "classification=DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY "
        "saved_packages=0 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
