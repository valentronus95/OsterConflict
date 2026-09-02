from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "Remington870"
IMPORT_SENTINEL = CACHE_DIR / "remington870_import_success.txt"
FRESH_SENTINEL = CACHE_DIR / "remington870_fresh_load_success.txt"
IMPORT_CONTRACT_REVISION = "PASS45_REMINGTON870_DERIVED_PUMP_PROD_R1"
SKELETAL_ASSET = "/Game/Production/Weapons/Remington870/SKM_Remington870"
RIGID_ASSET = "/Game/Production/Weapons/Remington870/SM_Remington870_Rigid"
PUMP_ANIMATION_ASSET = "/Game/Production/Weapons/Remington870/AN_Remington870_PumpCycle"
PUMP_BONE = "PASS45_PumpForeEnd"
EXPECTED_DURATION = 0.55
DURATION_TOLERANCE = 0.08
MIN_TRANSLATION_DELTA = 0.01
SAMPLE_TIMES = (0.0, 0.18, 0.28, 0.549)


def fail(message):
    unreal.log_error(f"PASS45_REMINGTON870_FRESH_LOAD_FAIL {message}")
    raise RuntimeError(message)


def parse_lines(text):
    result = {}
    for raw in text.splitlines():
        line = raw.strip()
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        result.setdefault(key, []).append(value)
    return result


def skeleton_path(obj):
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
    return str(skeleton.get_path_name()) if skeleton is not None else ""


def mesh_has_bone(mesh, bone_name):
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


def animation_length(animation):
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


def pump_motion(animation):
    if not unreal.AnimationLibrary.does_bone_name_exist(animation, unreal.Name(PUMP_BONE)):
        return False, 0.0
    length = animation_length(animation)
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


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if FRESH_SENTINEL.exists():
        FRESH_SENTINEL.unlink()
    if not IMPORT_SENTINEL.is_file():
        fail(f"import_sentinel_missing path={IMPORT_SENTINEL}")

    parsed = parse_lines(IMPORT_SENTINEL.read_text(encoding="utf-8", errors="replace"))
    if parsed.get("IMPORT_CONTRACT_REVISION") != [IMPORT_CONTRACT_REVISION]:
        fail(
            f"import_revision_mismatch expected={IMPORT_CONTRACT_REVISION} "
            f"actual={parsed.get('IMPORT_CONTRACT_REVISION')}"
        )
    if parsed.get("PRODUCTION_SOURCE_READY") != ["1"]:
        fail("import_sentinel_missing_production_source_ready=1")
    if parsed.get("runtime_acceptance") != ["0"] or parsed.get("item16_checked") != ["0"]:
        fail("import_sentinel_false_acceptance=1")

    skeletal = unreal.load_asset(SKELETAL_ASSET)
    rigid = unreal.load_asset(RIGID_ASSET)
    animation = unreal.load_asset(PUMP_ANIMATION_ASSET)
    if skeletal is None or skeletal.get_class().get_name() != "SkeletalMesh":
        fail(f"skeletal_fresh_load_failed path={SKELETAL_ASSET}")
    if rigid is None or rigid.get_class().get_name() != "StaticMesh":
        fail(f"rigid_fresh_load_failed path={RIGID_ASSET}")
    if animation is None or animation.get_class().get_name() != "AnimSequence":
        fail(f"animation_fresh_load_failed path={PUMP_ANIMATION_ASSET}")

    if not mesh_has_bone(skeletal, PUMP_BONE):
        fail("fresh_skeletal_missing_pump_bone=1")
    if not unreal.AnimationLibrary.does_bone_name_exist(animation, unreal.Name(PUMP_BONE)):
        fail("fresh_animation_missing_pump_bone=1")

    mesh_skeleton = skeleton_path(skeletal)
    animation_skeleton = skeleton_path(animation)
    if not mesh_skeleton or mesh_skeleton != animation_skeleton:
        fail(
            "fresh_shared_skeleton_missing=1 "
            f"mesh={mesh_skeleton or 'NONE'} animation={animation_skeleton or 'NONE'}"
        )

    length = animation_length(animation)
    if abs(length - EXPECTED_DURATION) > DURATION_TOLERANCE:
        fail(f"fresh_pump_duration_invalid expected={EXPECTED_DURATION} actual={length}")
    moved, delta = pump_motion(animation)
    if not moved:
        fail(f"fresh_pump_motion_missing max_translation_delta={delta}")

    lines = [
        f"IMPORT_CONTRACT_REVISION={IMPORT_CONTRACT_REVISION}",
        f"FRESH_LOADED={SKELETAL_ASSET}",
        f"FRESH_LOADED={RIGID_ASSET}",
        f"FRESH_LOADED={PUMP_ANIMATION_ASSET}",
        f"PUMP_BONE={PUMP_BONE}",
        f"PUMP_PLAY_LENGTH={length:.6f}",
        f"PUMP_MAX_TRANSLATION_DELTA={delta:.6f}",
        "PUMP_BONE_ADDRESSABLE=1",
        "PUMP_MOTION_PRESERVED=1",
        "SHARED_SKELETON_PRESERVED=1",
        "COMBINED_RIGID_AND_SKELETAL_ASSEMBLY=1",
        "PRODUCTION_FRESH_LOAD_READY=1",
        "runtime_acceptance=0",
        "item16_checked=0",
    ]
    FRESH_SENTINEL.write_text("\n".join(lines) + "\n", encoding="utf-8")
    unreal.log(
        "PASS45_REMINGTON870_FRESH_LOAD_PASS "
        f"skeletal={SKELETAL_ASSET} rigid={RIGID_ASSET} animation={PUMP_ANIMATION_ASSET} "
        f"pump_bone={PUMP_BONE} play_length={length:.6f} max_translation_delta={delta:.6f} "
        "production_fresh_load_ready=1 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
