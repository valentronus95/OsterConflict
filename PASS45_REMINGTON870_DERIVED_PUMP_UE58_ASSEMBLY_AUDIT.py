#!/usr/bin/env python3
"""Classify the imported UE 5.8 asset assembly for the derived Remington 870 pump.

This runs the already fail-closed derived-pump UE 5.8 pilot in the same editor
process, then inventories the unsaved imported objects. Its purpose is to answer
a production-wiring question the source GLB alone cannot answer: whether Unreal
creates one complete skeletal weapon visual or a multi-part/mixed assembly that
must be wired explicitly.

No packages are saved, no /Game/Production path is touched, and no runtime or
PASS45 item-16 acceptance is granted here.
"""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path

import unreal

BASE_PILOT = "PASS45_REMINGTON870_DERIVED_PUMP_UE58_PILOT.py"
EVIDENCE_REL = Path(
    "OsterConflict/Saved/PASS45/PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT.json"
)
PUMP_BONE = "PASS45_PumpForeEnd"
PUMP_ANIMATION = "PASS45_Remington870_PumpCycle"


def fail(message: str) -> None:
    unreal.log_error(f"PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_FAIL {message}")
    raise RuntimeError(message)


def load_base_pilot():
    root = Path(__file__).resolve().parent
    path = root / BASE_PILOT
    if not path.is_file():
        fail(f"base_pilot_missing path={path}")
    spec = importlib.util.spec_from_file_location(
        "pass45_remington870_derived_pump_ue58_pilot", path
    )
    if spec is None or spec.loader is None:
        fail("base_pilot_module_spec_failed=1")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def repo_root() -> Path:
    project_dir = Path(
        unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
    ).resolve()
    return project_dir.parent


def classify_assembly(skeletal_count: int, static_count: int) -> str:
    if skeletal_count == 1 and static_count == 0:
        return "SINGLE_SKELETAL_IMPORT_CANDIDATE"
    if skeletal_count > 1 and static_count == 0:
        return "MULTI_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN"
    if skeletal_count >= 1 and static_count > 0:
        return "MIXED_STATIC_SKELETAL_IMPORT_REQUIRES_ASSEMBLY_PLAN"
    return "INVALID_NO_SKELETAL_ASSEMBLY"


def main() -> None:
    pilot = load_base_pilot()
    pilot.main()

    imported = pilot.imported_objects()
    if not imported:
        fail("base_pilot_import_objects_missing=1")

    class_counts: dict[str, int] = {}
    asset_rows: list[dict[str, object]] = []
    skeletal_meshes: list[object] = []
    static_meshes: list[object] = []
    animations: list[object] = []

    for obj in imported:
        class_name = pilot.class_name(obj)
        class_counts[class_name] = class_counts.get(class_name, 0) + 1
        if class_name == "SkeletalMesh":
            skeletal_meshes.append(obj)
        elif class_name == "StaticMesh":
            static_meshes.append(obj)
        elif class_name == "AnimSequence":
            animations.append(obj)

    if not skeletal_meshes:
        fail("assembly_skeletal_mesh_missing=1")

    pump_meshes = [mesh for mesh in skeletal_meshes if pilot.mesh_has_bone(mesh, PUMP_BONE)]
    if not pump_meshes:
        fail("assembly_pump_mesh_missing=1")

    skeletal_skeletons = {
        pilot.skeleton_path(mesh) for mesh in skeletal_meshes if pilot.skeleton_path(mesh)
    }
    pump_skeletons = {
        pilot.skeleton_path(mesh) for mesh in pump_meshes if pilot.skeleton_path(mesh)
    }

    pump_sequences: list[object] = []
    for animation in animations:
        play_length = pilot.animation_length(animation)
        name_match = PUMP_ANIMATION.lower() in (
            animation.get_name() + " " + animation.get_path_name()
        ).lower()
        bone_addressable = pilot.bone_exists_in_animation(animation, PUMP_BONE)
        moved, max_delta, _samples = pilot.sample_pump_motion(animation, play_length)
        shared = pilot.skeleton_path(animation) in pump_skeletons
        if name_match and bone_addressable and moved and shared:
            pump_sequences.append(animation)
        unreal.log(
            "PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_ANIMATION "
            f"path={animation.get_path_name()} play_length={play_length:.6f} "
            f"name_match={int(name_match)} bone_addressable={int(bone_addressable)} "
            f"moved={int(moved)} shared_pump_skeleton={int(shared)} "
            f"max_translation_delta={max_delta:.6f}"
        )

    if len(pump_sequences) != 1:
        fail(
            "assembly_unique_pump_sequence_missing=1 "
            f"expected=1 actual={len(pump_sequences)}"
        )

    for obj in imported:
        class_name = pilot.class_name(obj)
        row: dict[str, object] = {
            "class": class_name,
            "name": obj.get_name(),
            "path": obj.get_path_name(),
        }
        if class_name in ("SkeletalMesh", "AnimSequence"):
            row["skeleton"] = pilot.skeleton_path(obj)
        if class_name == "SkeletalMesh":
            row["pump_bone_present"] = pilot.mesh_has_bone(obj, PUMP_BONE)
        asset_rows.append(row)
        unreal.log(
            "PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_ASSET "
            f"class={class_name} path={obj.get_path_name()} "
            f"skeleton={row.get('skeleton', 'NONE')} "
            f"pump_bone_present={int(bool(row.get('pump_bone_present', False)))}"
        )

    classification = classify_assembly(len(skeletal_meshes), len(static_meshes))
    if classification == "INVALID_NO_SKELETAL_ASSEMBLY":
        fail("assembly_classification_invalid=1")

    evidence = {
        "schema": 1,
        "status": "DERIVED_PUMP_UE58_ASSEMBLY_EVIDENCE_ONLY",
        "engine_version": unreal.SystemLibrary.get_engine_version(),
        "pilot_status": "DERIVED_PUMP_UE58_IMPORT_PROOF_ONLY",
        "pump_bone": PUMP_BONE,
        "pump_animation": PUMP_ANIMATION,
        "class_counts": class_counts,
        "imported_asset_count": len(imported),
        "skeletal_mesh_count": len(skeletal_meshes),
        "static_mesh_count": len(static_meshes),
        "animation_count": len(animations),
        "pump_mesh_count": len(pump_meshes),
        "pump_sequence_count": len(pump_sequences),
        "skeletal_skeleton_count": len(skeletal_skeletons),
        "pump_skeleton_count": len(pump_skeletons),
        "skeletal_skeletons": sorted(skeletal_skeletons),
        "pump_skeletons": sorted(pump_skeletons),
        "assembly_classification": classification,
        "production_visual_completeness": "UNPROVEN",
        "asset_rows": sorted(asset_rows, key=lambda row: (str(row["class"]), str(row["path"]))),
        "saved_packages": False,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    evidence_path = repo_root() / EVIDENCE_REL
    evidence_path.parent.mkdir(parents=True, exist_ok=True)
    evidence_path.write_text(
        json.dumps(evidence, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )

    unreal.log(
        "PASS45_REMINGTON870_DERIVED_PUMP_UE58_ASSEMBLY_AUDIT_PASS "
        f"classification={classification} imported_assets={len(imported)} "
        f"skeletal_meshes={len(skeletal_meshes)} static_meshes={len(static_meshes)} "
        f"animations={len(animations)} pump_meshes={len(pump_meshes)} "
        f"pump_sequences={len(pump_sequences)} skeletal_skeletons={len(skeletal_skeletons)} "
        "production_visual_completeness=UNPROVEN saved_packages=0 production_cutover=0 "
        "runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
