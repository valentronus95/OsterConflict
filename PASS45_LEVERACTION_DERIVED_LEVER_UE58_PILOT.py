#!/usr/bin/env python3
"""Isolated UE 5.8 moving-part pilot for the committed Stein Lever Action FBX.

This script imports the exact pinned Stein CC0 Lever Action source as an unsaved
SkeletalMesh, reads the factual UE reference transform of the existing LEVER
bone, creates an unsaved standalone AnimSequence, and adds a bounded local-X
calibration motion to that bone through UE's AnimationDataController.

The 0.85 s duration matches Oster's existing authoritative LeverAction gameplay
cycle. The -45 degree excursion is a calibration pilot only: the source FBX has
no authored animation channel or authored lever endpoint. Nothing here changes
the production weapon profile, saves a package, closes PASS45 item 16, or claims
runtime/visual acceptance.
"""
from __future__ import annotations

import hashlib
import json
import math
from pathlib import Path

try:
    import unreal
except ImportError as exc:  # pragma: no cover - must run inside Unreal Editor Python
    raise SystemExit("PASS45 LEVER ACTION UE58 PILOT: run inside Unreal Editor 5.8 Python") from exc

EXPECTED_ENGINE_PREFIX = "5.8"
EXPECTED_SOURCE_REL = Path(
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/"
    "LeverAction/SKM_LeverAction.fbx"
)
EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
EXPECTED_SOURCE_BYTES = 570332
LEVER_BONE = "LEVER"
PILOT_DESTINATION = "/Game/PASS45/ImportPilots/LeverActionDerivedCycle"
PILOT_SEQUENCE_NAME = "AN_PASS45_LeverAction_Cycle_Pilot"
CYCLE_DURATION = 0.85
FRAME_RATE = 20
FRAME_COUNT = 17
KEY_COUNT = FRAME_COUNT + 1
PILOT_MAX_ANGLE_DEG = -45.0
MOTION_ANCHORS = (
    (0.00, 0.00),
    (0.20, PILOT_MAX_ANGLE_DEG * 0.55),
    (0.42, PILOT_MAX_ANGLE_DEG),
    (0.66, PILOT_MAX_ANGLE_DEG * 0.35),
    (CYCLE_DURATION, 0.00),
)
SAMPLE_TIMES = (0.0, 0.20, 0.42, 0.66, 0.849)
MIN_NONTRIVIAL_ROTATION_DEG = 20.0
MAX_END_RETURN_ERROR_DEG = 4.0
EVIDENCE_REL = Path(
    "OsterConflict/Saved/PASS45/PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT.json"
)
LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1"


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_FAIL {message}")
    raise RuntimeError(message)


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


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
        PILOT_DESTINATION,
        recursive=True,
        include_folder=False,
    )
    if existing:
        fail(
            "pilot_destination_not_clean=1 destructive_cleanup_refused=1 "
            f"destination={PILOT_DESTINATION} asset_count={len(existing)}"
        )


def configure_fbx_import() -> object:
    if not hasattr(unreal, "FbxImportUI"):
        fail("fbx_import_ui_unavailable=1")
    options = unreal.FbxImportUI()
    properties = {
        "automated_import_should_detect_type": False,
        "import_mesh": True,
        "import_as_skeletal": True,
        "import_animations": False,
        "import_materials": False,
        "import_textures": False,
        "create_physics_asset": False,
    }
    for name, value in properties.items():
        try:
            options.set_editor_property(name, value)
        except Exception as exc:
            fail(f"fbx_import_option_unavailable property={name} error={exc}")

    enum_type = getattr(unreal, "FBXImportType", None)
    if enum_type is None:
        fail("fbx_import_type_enum_unavailable=1")
    skeletal_enum = getattr(enum_type, "FBXIT_SKELETAL_MESH", None)
    if skeletal_enum is None:
        fail("fbx_skeletal_import_enum_unavailable=1")
    try:
        options.set_editor_property("mesh_type_to_import", skeletal_enum)
    except Exception as exc:
        fail(f"fbx_mesh_type_assignment_failed error={exc}")
    return options


def import_exact_source(source: Path) -> list[object]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", PILOT_DESTINATION)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("replace_existing_settings", False)
    task.set_editor_property("options", configure_fbx_import())

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.get_objects())
    if not imported:
        for path in unreal.EditorAssetLibrary.list_assets(
            PILOT_DESTINATION,
            recursive=True,
            include_folder=False,
        ):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if asset is not None:
                imported.append(asset)
    if not imported:
        fail("exact_fbx_import_no_objects=1")
    return imported


def find_single_skeletal_mesh(imported: list[object]) -> object:
    meshes = [obj for obj in imported if class_name(obj) == "SkeletalMesh"]
    if len(meshes) != 1:
        fail(
            "exact_fbx_skeletal_mesh_count_invalid=1 "
            f"count={len(meshes)} classes={json.dumps([class_name(obj) for obj in imported])}"
        )
    return meshes[0]


def get_skeleton(mesh: object) -> object:
    getter = getattr(mesh, "get_skeleton", None)
    if callable(getter):
        try:
            skeleton = getter()
            if skeleton is not None:
                return skeleton
        except Exception:
            pass
    try:
        skeleton = mesh.get_editor_property("skeleton")
        if skeleton is not None:
            return skeleton
    except Exception:
        pass
    fail("imported_skeletal_mesh_has_no_skeleton=1")


def make_mesh_component(mesh: object) -> object:
    component = unreal.SkeletalMeshComponent()
    setter = getattr(component, "set_skeletal_mesh", None)
    if callable(setter):
        try:
            setter(mesh, True)
            return component
        except TypeError:
            try:
                setter(mesh)
                return component
            except Exception:
                pass
        except Exception:
            pass
    for property_name in ("skeletal_mesh_asset", "skeletal_mesh"):
        try:
            component.set_editor_property(property_name, mesh)
            return component
        except Exception:
            continue
    fail("cannot_bind_imported_mesh_to_transient_component=1")


def ref_pose_for_bone(mesh: object, bone_name: str) -> tuple[int, object]:
    component = make_mesh_component(mesh)
    bone_index = int(component.get_bone_index(unreal.Name(bone_name)))
    if bone_index < 0:
        fail(f"required_bone_missing bone={bone_name}")
    getter = getattr(component, "get_ref_pose_transform", None)
    if not callable(getter):
        fail("get_ref_pose_transform_unavailable=1")
    transform = getter(bone_index)
    if transform is None:
        fail(f"ref_pose_transform_missing bone={bone_name} index={bone_index}")
    return bone_index, transform


def copy_vector(value: object) -> object:
    return unreal.Vector(float(value.x), float(value.y), float(value.z))


def copy_quat(value: object) -> object:
    return unreal.Quat(float(value.x), float(value.y), float(value.z), float(value.w))


def quat_mul(a: object, b: object) -> object:
    ax, ay, az, aw = float(a.x), float(a.y), float(a.z), float(a.w)
    bx, by, bz, bw = float(b.x), float(b.y), float(b.z), float(b.w)
    return unreal.Quat(
        aw * bx + ax * bw + ay * bz - az * by,
        aw * by - ax * bz + ay * bw + az * bx,
        aw * bz + ax * by - ay * bx + az * bw,
        aw * bw - ax * bx - ay * by - az * bz,
    )


def quat_x(degrees: float) -> object:
    half = math.radians(degrees) * 0.5
    return unreal.Quat(math.sin(half), 0.0, 0.0, math.cos(half))


def normalized_quat(value: object) -> object:
    magnitude = math.sqrt(
        float(value.x) ** 2
        + float(value.y) ** 2
        + float(value.z) ** 2
        + float(value.w) ** 2
    )
    if magnitude <= 1e-8:
        fail("zero_length_quaternion=1")
    return unreal.Quat(
        float(value.x) / magnitude,
        float(value.y) / magnitude,
        float(value.z) / magnitude,
        float(value.w) / magnitude,
    )


def angle_for_time(seconds: float) -> float:
    time_value = max(0.0, min(float(seconds), CYCLE_DURATION))
    for (t0, a0), (t1, a1) in zip(MOTION_ANCHORS, MOTION_ANCHORS[1:]):
        if time_value <= t1 + 1e-9:
            if t1 <= t0:
                fail("invalid_motion_anchor_order=1")
            alpha = (time_value - t0) / (t1 - t0)
            return a0 + (a1 - a0) * alpha
    return float(MOTION_ANCHORS[-1][1])


def animation_controller(sequence: object) -> object:
    try:
        controller = sequence.get_editor_property("controller")
    except Exception:
        controller = getattr(sequence, "controller", None)
    if controller is None:
        getter = getattr(sequence, "get_controller", None)
        if callable(getter):
            try:
                controller = getter()
            except Exception:
                controller = None
    if controller is None:
        fail("animation_data_controller_unavailable=1")
    return controller


def create_unsaved_sequence(skeleton: object, mesh: object) -> object:
    factory = unreal.AnimSequenceFactory()
    factory.set_editor_property("target_skeleton", skeleton)
    try:
        factory.set_editor_property("preview_skeletal_mesh", mesh)
    except Exception:
        pass
    sequence = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        PILOT_SEQUENCE_NAME,
        PILOT_DESTINATION,
        unreal.AnimSequence,
        factory,
    )
    if sequence is None:
        fail("pilot_anim_sequence_creation_failed=1")
    if class_name(sequence) != "AnimSequence":
        fail(f"pilot_sequence_wrong_class class={class_name(sequence)}")
    return sequence


def configure_sequence(sequence: object, ref_transform: object) -> dict[str, object]:
    controller = animation_controller(sequence)
    frame_rate = unreal.FrameRate(numerator=FRAME_RATE, denominator=1)
    controller.set_frame_rate(frame_rate, False)
    controller.set_number_of_frames(unreal.FrameNumber(value=FRAME_COUNT), False)

    track_index = int(controller.add_bone_track(unreal.Name(LEVER_BONE), False))
    if track_index < 0:
        fail(f"lever_bone_track_creation_failed track_index={track_index}")

    bind_translation = copy_vector(ref_transform.translation)
    bind_rotation = normalized_quat(copy_quat(ref_transform.rotation))
    bind_scale = copy_vector(ref_transform.scale3d)

    positions = []
    rotations = []
    scales = []
    key_rows: list[dict[str, object]] = []
    for frame in range(KEY_COUNT):
        seconds = frame / float(FRAME_RATE)
        angle = angle_for_time(seconds)
        delta = quat_x(angle)
        local_rotation = normalized_quat(quat_mul(bind_rotation, delta))
        positions.append(copy_vector(bind_translation))
        rotations.append(local_rotation)
        scales.append(copy_vector(bind_scale))
        key_rows.append({
            "frame": frame,
            "time": round(seconds, 6),
            "pilot_angle_deg": round(angle, 6),
        })

    if not controller.set_bone_track_keys(
        unreal.Name(LEVER_BONE),
        positions,
        rotations,
        scales,
        False,
    ):
        fail("lever_bone_track_key_write_failed=1")

    return {
        "track_index": track_index,
        "frame_rate": FRAME_RATE,
        "frame_count": FRAME_COUNT,
        "key_count": KEY_COUNT,
        "bind_translation": [
            float(bind_translation.x),
            float(bind_translation.y),
            float(bind_translation.z),
        ],
        "bind_rotation": [
            float(bind_rotation.x),
            float(bind_rotation.y),
            float(bind_rotation.z),
            float(bind_rotation.w),
        ],
        "bind_scale": [
            float(bind_scale.x),
            float(bind_scale.y),
            float(bind_scale.z),
        ],
        "keys": key_rows,
    }


def animation_length(sequence: object) -> float:
    getter = getattr(sequence, "get_play_length", None)
    if callable(getter):
        try:
            return float(getter())
        except Exception:
            pass
    for name in ("sequence_length", "play_length"):
        try:
            return float(sequence.get_editor_property(name))
        except Exception:
            continue
    return -1.0


def quat_angle_deg(a: object, b: object) -> float:
    qa = normalized_quat(a)
    qb = normalized_quat(b)
    dot = abs(
        float(qa.x) * float(qb.x)
        + float(qa.y) * float(qb.y)
        + float(qa.z) * float(qb.z)
        + float(qa.w) * float(qb.w)
    )
    dot = max(-1.0, min(1.0, dot))
    return math.degrees(2.0 * math.acos(dot))


def sample_motion(sequence: object) -> tuple[float, float, list[dict[str, object]]]:
    if not hasattr(unreal, "AnimationLibrary"):
        fail("animation_library_unavailable=1")
    if not unreal.AnimationLibrary.does_bone_name_exist(sequence, unreal.Name(LEVER_BONE)):
        fail("lever_bone_not_addressable_in_pilot_sequence=1")

    poses = []
    rows: list[dict[str, object]] = []
    for seconds in SAMPLE_TIMES:
        pose = unreal.AnimationLibrary.get_bone_pose_for_time(
            sequence,
            unreal.Name(LEVER_BONE),
            min(seconds, CYCLE_DURATION - 0.0001),
            False,
        )
        poses.append(pose)
        rows.append({
            "time": seconds,
            "translation": [
                float(pose.translation.x),
                float(pose.translation.y),
                float(pose.translation.z),
            ],
            "rotation": [
                float(pose.rotation.x),
                float(pose.rotation.y),
                float(pose.rotation.z),
                float(pose.rotation.w),
            ],
            "scale": [
                float(pose.scale3d.x),
                float(pose.scale3d.y),
                float(pose.scale3d.z),
            ],
        })

    first = poses[0]
    max_rotation = max(quat_angle_deg(first.rotation, pose.rotation) for pose in poses[1:])
    end_return_error = quat_angle_deg(first.rotation, poses[-1].rotation)
    return max_rotation, end_return_error, rows


def write_evidence(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> None:
    engine_version = str(unreal.SystemLibrary.get_engine_version())
    if not engine_version.startswith(EXPECTED_ENGINE_PREFIX):
        fail(f"wrong_engine expected_prefix={EXPECTED_ENGINE_PREFIX} actual={engine_version}")

    root = repo_root()
    source = root / EXPECTED_SOURCE_REL
    identity = source_identity(source)
    ensure_clean_destination()

    imported = import_exact_source(source)
    mesh = find_single_skeletal_mesh(imported)
    skeleton = get_skeleton(mesh)
    bone_index, ref_transform = ref_pose_for_bone(mesh, LEVER_BONE)
    sequence = create_unsaved_sequence(skeleton, mesh)
    controller_evidence = configure_sequence(sequence, ref_transform)

    play_length = animation_length(sequence)
    if abs(play_length - CYCLE_DURATION) > 0.011:
        fail(
            "pilot_sequence_duration_mismatch=1 "
            f"expected={CYCLE_DURATION} actual={play_length}"
        )

    max_rotation, end_return_error, samples = sample_motion(sequence)
    if max_rotation < MIN_NONTRIVIAL_ROTATION_DEG:
        fail(
            "pilot_lever_motion_trivial=1 "
            f"max_rotation_deg={max_rotation:.6f}"
        )
    if end_return_error > MAX_END_RETURN_ERROR_DEG:
        fail(
            "pilot_lever_does_not_return_to_bind=1 "
            f"end_return_error_deg={end_return_error:.6f}"
        )

    evidence = {
        "schema": 1,
        "status": "LEVERACTION_DERIVED_UE58_MOTION_PROOF_ONLY",
        "engine_version": engine_version,
        "source": EXPECTED_SOURCE_REL.as_posix(),
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["bytes"],
        "imported_objects": [object_path(obj) for obj in imported],
        "skeletal_mesh": object_path(mesh),
        "skeleton": object_path(skeleton),
        "lever_bone": LEVER_BONE,
        "lever_bone_index": bone_index,
        "pilot_sequence": object_path(sequence),
        "cycle_duration_seconds": CYCLE_DURATION,
        "play_length_seconds": play_length,
        "pilot_axis": "LEVER local X after factual UE-imported bind rotation",
        "pilot_max_angle_deg": PILOT_MAX_ANGLE_DEG,
        "pilot_angle_accepted": False,
        "source_authored_animation": False,
        "source_authored_endpoint": False,
        "controller": controller_evidence,
        "samples": samples,
        "max_sampled_rotation_delta_deg": max_rotation,
        "end_return_error_deg": end_return_error,
        "lever_bone_addressable": True,
        "lever_motion_nontrivial": True,
        "returns_near_bind_pose": True,
        "saved_packages": False,
        "production_profile_changed": False,
        "production_cutover": False,
        "runtime_visual_acceptance": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    evidence_path = root / EVIDENCE_REL
    write_evidence(evidence_path, evidence)

    unreal.log(
        "PASS45_LEVERACTION_DERIVED_LEVER_UE58_PILOT_PASS "
        f"engine={engine_version} source_sha256={EXPECTED_SOURCE_SHA256} "
        f"skeletal_mesh={object_path(mesh)} skeleton={object_path(skeleton)} "
        f"lever_bone={LEVER_BONE} lever_bone_index={bone_index} "
        f"sequence={object_path(sequence)} play_length={play_length:.6f} "
        f"max_rotation_delta_deg={max_rotation:.6f} "
        f"end_return_error_deg={end_return_error:.6f} "
        "source_authored_endpoint=0 pilot_angle_accepted=0 saved_packages=0 "
        "production_profile_changed=0 production_cutover=0 "
        "runtime_visual_acceptance=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
