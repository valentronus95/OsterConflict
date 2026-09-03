#!/usr/bin/env python3
"""Isolated UE 5.8 motion pilot for the Stein CC0 M700 BOLT joint.

The source FBX has a real weighted BOLT joint but no authored bolt animation. The
separate weighted BOLT_STOP joint is explicitly not accepted as a travel endpoint.
This pilot imports the exact FBX without saving packages, creates an unsaved
AnimSequence on the imported skeleton, and applies only the bounded translation
calibration channel defined by PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE.

This is UE motion proof only. It does not save production content, alter the M700
profile, accept the pilot travel, author the missing bolt rotation, close item 16,
or claim visual/runtime acceptance.
"""
from __future__ import annotations

import hashlib
import json
import math
import sys
from pathlib import Path

try:
    import unreal
except ImportError as exc:  # pragma: no cover
    raise SystemExit("PASS45 M700 UE58 PILOT: run inside Unreal Editor 5.8 Python") from exc

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_REL = Path(
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
)
EXPECTED_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
EXPECTED_SOURCE_BYTES = 638732
BOLT_BONE = "BOLT"
PILOT_DESTINATION = "/Game/PASS45/ImportPilots/M700DerivedBoltTranslation"
PILOT_SEQUENCE_NAME = "AN_PASS45_M700_BoltTranslation_Pilot"
FRAME_RATE = 20
FRAME_COUNT = 22
SAMPLE_TIMES = (0.0, 0.18, 0.48, 0.78, 1.099)
MIN_NONTRIVIAL_TRANSLATION = 0.01
MAX_END_RETURN_ERROR = 0.005
EVIDENCE_REL = Path(
    "OsterConflict/Saved/PASS45/PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT.json"
)
LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1"


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def ensure_repo_importable(root: Path) -> None:
    value = str(root)
    if value not in sys.path:
        sys.path.insert(0, value)


def source_identity(path: Path) -> dict[str, object]:
    if not path.is_file():
        fail(f"source_missing path={path}")
    with path.open("rb") as handle:
        prefix = handle.read(160)
    if prefix.startswith(LFS_POINTER_PREFIX):
        fail("git_lfs_pointer_only=1 run_normal_lfs_fetch_before_ue_pilot=1")
    size = path.stat().st_size
    if size != EXPECTED_SOURCE_BYTES:
        fail(f"source_size_mismatch expected={EXPECTED_SOURCE_BYTES} actual={size}")
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != EXPECTED_SOURCE_SHA256:
        fail(f"source_sha256_mismatch expected={EXPECTED_SOURCE_SHA256} actual={digest}")
    return {"bytes": size, "sha256": digest}


def class_name(obj: object) -> str:
    try:
        return str(obj.get_class().get_name())
    except Exception:
        return type(obj).__name__


def object_path(obj: object) -> str:
    try:
        return str(obj.get_path_name())
    except Exception:
        return str(obj)


def ensure_clean_destination() -> None:
    existing = unreal.EditorAssetLibrary.list_assets(
        PILOT_DESTINATION, recursive=True, include_folder=False
    )
    if existing:
        fail(
            "pilot_destination_not_clean=1 destructive_cleanup_refused=1 "
            f"destination={PILOT_DESTINATION} asset_count={len(existing)}"
        )


def fbx_options() -> object:
    options = unreal.FbxImportUI()
    settings = {
        "automated_import_should_detect_type": False,
        "import_mesh": True,
        "import_as_skeletal": True,
        "import_animations": False,
        "import_materials": False,
        "import_textures": False,
        "create_physics_asset": False,
    }
    for name, value in settings.items():
        try:
            options.set_editor_property(name, value)
        except Exception as exc:
            fail(f"fbx_import_option_unavailable property={name} error={exc}")
    enum_value = getattr(getattr(unreal, "FBXImportType", None), "FBXIT_SKELETAL_MESH", None)
    if enum_value is None:
        fail("fbx_skeletal_import_enum_unavailable=1")
    options.set_editor_property("mesh_type_to_import", enum_value)
    return options


def import_source(source: Path) -> list[object]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", PILOT_DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("options", fbx_options())
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.get_objects())
    if not imported:
        for path in unreal.EditorAssetLibrary.list_assets(
            PILOT_DESTINATION, recursive=True, include_folder=False
        ):
            obj = unreal.EditorAssetLibrary.load_asset(path)
            if obj is not None:
                imported.append(obj)
    if not imported:
        fail("exact_fbx_import_no_objects=1")
    return imported


def find_mesh(imported: list[object]) -> object:
    meshes = [obj for obj in imported if class_name(obj) == "SkeletalMesh"]
    if len(meshes) != 1:
        fail(f"exact_fbx_skeletal_mesh_count_invalid count={len(meshes)}")
    return meshes[0]


def get_skeleton(mesh: object) -> object:
    getter = getattr(mesh, "get_skeleton", None)
    if callable(getter):
        try:
            value = getter()
            if value is not None:
                return value
        except Exception:
            pass
    try:
        value = mesh.get_editor_property("skeleton")
        if value is not None:
            return value
    except Exception:
        pass
    fail("imported_skeletal_mesh_has_no_skeleton=1")


def ref_pose(mesh: object) -> tuple[int, object]:
    component = unreal.SkeletalMeshComponent()
    setter = getattr(component, "set_skeletal_mesh", None)
    if callable(setter):
        try:
            setter(mesh)
        except TypeError:
            setter(mesh, True)
    else:
        component.set_editor_property("skeletal_mesh_asset", mesh)
    bone_index = int(component.get_bone_index(unreal.Name(BOLT_BONE)))
    if bone_index < 0:
        fail(f"required_bone_missing bone={BOLT_BONE}")
    getter = getattr(component, "get_ref_pose_transform", None)
    if not callable(getter):
        fail("get_ref_pose_transform_unavailable=1")
    transform = getter(bone_index)
    if transform is None:
        fail("bolt_ref_pose_transform_missing=1")
    return bone_index, transform


def copy_vector(value: object) -> object:
    return unreal.Vector(float(value.x), float(value.y), float(value.z))


def copy_quat(value: object) -> object:
    return unreal.Quat(float(value.x), float(value.y), float(value.z), float(value.w))


def controller(sequence: object) -> object:
    try:
        value = sequence.get_editor_property("controller")
    except Exception:
        value = getattr(sequence, "controller", None)
    if value is None:
        getter = getattr(sequence, "get_controller", None)
        if callable(getter):
            value = getter()
    if value is None:
        fail("animation_data_controller_unavailable=1")
    return value


def interpolate_offset(contract: object, seconds: float) -> float:
    t = max(0.0, min(float(seconds), float(contract.CYCLE_DURATION)))
    times = tuple(float(v) for v in contract.KEY_TIMES)
    offsets = tuple(float(v) for v in contract.KEY_OFFSETS_Y)
    for t0, t1, y0, y1 in zip(times, times[1:], offsets, offsets[1:]):
        if t <= t1 + 1e-9:
            alpha = (t - t0) / (t1 - t0)
            return y0 + (y1 - y0) * alpha
    return offsets[-1]


def create_sequence(skeleton: object, mesh: object, ref_transform: object, contract: object) -> tuple[object, dict[str, object]]:
    factory = unreal.AnimSequenceFactory()
    factory.set_editor_property("target_skeleton", skeleton)
    try:
        factory.set_editor_property("preview_skeletal_mesh", mesh)
    except Exception:
        pass
    sequence = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        PILOT_SEQUENCE_NAME, PILOT_DESTINATION, unreal.AnimSequence, factory
    )
    if sequence is None or class_name(sequence) != "AnimSequence":
        fail("pilot_anim_sequence_creation_failed=1")

    data = controller(sequence)
    data.set_frame_rate(unreal.FrameRate(numerator=FRAME_RATE, denominator=1), False)
    data.set_number_of_frames(unreal.FrameNumber(value=FRAME_COUNT), False)
    track_index = int(data.add_bone_track(unreal.Name(BOLT_BONE), False))
    if track_index < 0:
        fail("bolt_bone_track_creation_failed=1")

    bind_t = copy_vector(ref_transform.translation)
    bind_r = copy_quat(ref_transform.rotation)
    bind_s = copy_vector(ref_transform.scale3d)
    positions = []
    rotations = []
    scales = []
    keys: list[dict[str, object]] = []
    for frame in range(FRAME_COUNT + 1):
        seconds = frame / float(FRAME_RATE)
        offset_y = interpolate_offset(contract, seconds)
        positions.append(unreal.Vector(float(bind_t.x), float(bind_t.y) + offset_y, float(bind_t.z)))
        rotations.append(copy_quat(bind_r))
        scales.append(copy_vector(bind_s))
        keys.append({"frame": frame, "time": round(seconds, 6), "offset_y": offset_y})
    if not data.set_bone_track_keys(unreal.Name(BOLT_BONE), positions, rotations, scales, False):
        fail("bolt_bone_track_key_write_failed=1")
    return sequence, {
        "track_index": track_index,
        "frame_rate": FRAME_RATE,
        "frame_count": FRAME_COUNT,
        "key_count": FRAME_COUNT + 1,
        "bind_translation": [float(bind_t.x), float(bind_t.y), float(bind_t.z)],
        "keys": keys,
    }


def play_length(sequence: object) -> float:
    getter = getattr(sequence, "get_play_length", None)
    if callable(getter):
        return float(getter())
    for name in ("sequence_length", "play_length"):
        try:
            return float(sequence.get_editor_property(name))
        except Exception:
            pass
    return -1.0


def sample(sequence: object) -> tuple[float, float, list[dict[str, object]]]:
    if not hasattr(unreal, "AnimationLibrary"):
        fail("animation_library_unavailable=1")
    if not unreal.AnimationLibrary.does_bone_name_exist(sequence, unreal.Name(BOLT_BONE)):
        fail("bolt_bone_not_addressable_in_pilot_sequence=1")
    poses = []
    rows = []
    for seconds in SAMPLE_TIMES:
        pose = unreal.AnimationLibrary.get_bone_pose_for_time(
            sequence, unreal.Name(BOLT_BONE), seconds, False
        )
        poses.append(pose)
        rows.append({
            "time": seconds,
            "translation": [float(pose.translation.x), float(pose.translation.y), float(pose.translation.z)],
        })
    first = poses[0].translation
    def distance(value: object) -> float:
        return math.sqrt(
            (float(value.x) - float(first.x)) ** 2
            + (float(value.y) - float(first.y)) ** 2
            + (float(value.z) - float(first.z)) ** 2
        )
    max_delta = max(distance(pose.translation) for pose in poses[1:])
    end_error = distance(poses[-1].translation)
    return max_delta, end_error, rows


def main() -> None:
    engine_version = str(unreal.SystemLibrary.get_engine_version())
    if not engine_version.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine_version}")
    root = repo_root()
    ensure_repo_importable(root)
    try:
        import PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE as contract
    except Exception as exc:
        fail(f"derived_source_contract_import_failed error={exc}")
    if contract.PILOT_TRAVEL_FRACTION_OF_Y_EXTENT != 0.20:
        fail("pilot_travel_contract_drifted=1")

    source = root / EXPECTED_SOURCE_REL
    identity = source_identity(source)
    ensure_clean_destination()
    imported = import_source(source)
    mesh = find_mesh(imported)
    skeleton = get_skeleton(mesh)
    bone_index, ref_transform = ref_pose(mesh)
    sequence, controller_evidence = create_sequence(skeleton, mesh, ref_transform, contract)

    length = play_length(sequence)
    if abs(length - float(contract.CYCLE_DURATION)) > 0.011:
        fail(f"pilot_sequence_duration_mismatch expected={contract.CYCLE_DURATION} actual={length}")
    max_delta, end_error, samples = sample(sequence)
    if max_delta < MIN_NONTRIVIAL_TRANSLATION:
        fail(f"pilot_bolt_motion_trivial max_translation_delta={max_delta:.6f}")
    if end_error > MAX_END_RETURN_ERROR:
        fail(f"pilot_bolt_does_not_return_to_bind end_return_error={end_error:.6f}")

    evidence = {
        "schema": 1,
        "status": "M700_BOLT_TRANSLATION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "engine_version": engine_version,
        "source": EXPECTED_SOURCE_REL.as_posix(),
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["bytes"],
        "skeletal_mesh": object_path(mesh),
        "skeleton": object_path(skeleton),
        "bolt_bone": BOLT_BONE,
        "bolt_bone_index": bone_index,
        "pilot_sequence": object_path(sequence),
        "cycle_duration_seconds": float(contract.CYCLE_DURATION),
        "play_length_seconds": length,
        "pilot_axis": "BOLT local Y after factual UE-imported bind transform",
        "pilot_max_travel": float(contract.PILOT_MAX_TRAVEL),
        "pilot_travel_accepted": False,
        "source_authored_animation": False,
        "source_authored_endpoint": False,
        "bolt_stop_used_as_endpoint": False,
        "rotation_channel_authored": False,
        "rotation_calibration_pending": True,
        "controller": controller_evidence,
        "samples": samples,
        "max_sampled_translation_delta": max_delta,
        "end_return_error": end_error,
        "bolt_bone_addressable": True,
        "bolt_motion_nontrivial": True,
        "returns_near_bind_pose": True,
        "saved_packages": False,
        "production_profile_changed": False,
        "production_cutover": False,
        "runtime_visual_acceptance": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    evidence_path = root / EVIDENCE_REL
    evidence_path.parent.mkdir(parents=True, exist_ok=True)
    evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(
        "PASS45_M700_DERIVED_BOLT_TRANSLATION_UE58_PILOT_PASS "
        f"engine={engine_version} source_sha256={EXPECTED_SOURCE_SHA256} "
        f"skeletal_mesh={object_path(mesh)} skeleton={object_path(skeleton)} "
        f"bolt_bone={BOLT_BONE} bolt_bone_index={bone_index} "
        f"sequence={object_path(sequence)} play_length={length:.6f} "
        f"max_translation_delta={max_delta:.6f} end_return_error={end_error:.6f} "
        "source_authored_endpoint=0 bolt_stop_used_as_endpoint=0 pilot_travel_accepted=0 "
        "rotation_calibration_pending=1 saved_packages=0 production_profile_changed=0 "
        "production_cutover=0 runtime_visual_acceptance=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
