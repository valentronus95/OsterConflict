#!/usr/bin/env python3
"""Fail-closed structural/motion audit for the already-committed Stein CC0 M700 FBX.

This audit does not create a production asset and cannot close PASS45 item 16.
GitHub Actions materializes only the exact M700 LFS payload, Assimp converts it to
an inspection-only glTF, and this script records what the source actually contains.

Schema 2 additionally records the exact BOLT/BOLT_STOP hierarchy and local bind
transforms. If both marker joints are siblings, their source-authored local delta
is emitted as a derivative-candidate stroke vector rather than inventing motion.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path

EXPECTED_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
EXPECTED_SIZE = 638732
EXPECTED_SOURCE = (
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/"
    "WeaponsPack/M700/SKM_M700.fbx"
)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 M700 SOURCE MOTION AUDIT: FAIL\n[FAIL] {message}")


def clean_name(value: object, fallback: str) -> str:
    text = str(value or "").strip()
    return text or fallback


def vec3(value: object, default: tuple[float, float, float]) -> list[float]:
    if not isinstance(value, list) or len(value) != 3:
        return [float(v) for v in default]
    try:
        result = [float(v) for v in value]
    except (TypeError, ValueError):
        fail(f"invalid VEC3 transform payload: {value!r}")
    if not all(math.isfinite(v) for v in result):
        fail(f"non-finite VEC3 transform payload: {value!r}")
    return result


def quat4(value: object) -> list[float]:
    if not isinstance(value, list) or len(value) != 4:
        return [0.0, 0.0, 0.0, 1.0]
    try:
        result = [float(v) for v in value]
    except (TypeError, ValueError):
        fail(f"invalid quaternion transform payload: {value!r}")
    if not all(math.isfinite(v) for v in result):
        fail(f"non-finite quaternion transform payload: {value!r}")
    return result


def node_local_transform(node: dict[str, object]) -> dict[str, object]:
    matrix = node.get("matrix")
    if isinstance(matrix, list):
        if len(matrix) != 16:
            fail(f"invalid node matrix length: {len(matrix)}")
        try:
            values = [float(v) for v in matrix]
        except (TypeError, ValueError):
            fail(f"invalid node matrix payload: {matrix!r}")
        if not all(math.isfinite(v) for v in values):
            fail("non-finite node matrix payload")
        return {"matrix": values, "uses_matrix": True}
    return {
        "translation": vec3(node.get("translation"), (0.0, 0.0, 0.0)),
        "rotation": quat4(node.get("rotation")),
        "scale": vec3(node.get("scale"), (1.0, 1.0, 1.0)),
        "uses_matrix": False,
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", default=EXPECTED_SOURCE)
    parser.add_argument("--gltf", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    source = Path(args.source)
    gltf_path = Path(args.gltf)
    output = Path(args.output)

    if source.as_posix() != EXPECTED_SOURCE:
        fail(f"unexpected source path: {source.as_posix()}")
    if not source.is_file():
        fail(f"missing source FBX: {source}")

    payload = source.read_bytes()
    if payload.startswith(b"version https://git-lfs.github.com/spec/v1"):
        fail("M700 LFS payload was not materialized; refusing to audit pointer text")
    if len(payload) != EXPECTED_SIZE:
        fail(f"source size drifted: expected {EXPECTED_SIZE}, got {len(payload)}")
    digest = hashlib.sha256(payload).hexdigest()
    if digest != EXPECTED_SHA256:
        fail(f"source SHA-256 drifted: expected {EXPECTED_SHA256}, got {digest}")

    if not gltf_path.is_file():
        fail(f"inspection glTF missing: {gltf_path}")
    try:
        doc = json.loads(gltf_path.read_text(encoding="utf-8"))
    except Exception as exc:
        fail(f"invalid inspection glTF JSON: {exc}")

    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []
    skins = doc.get("skins") or []
    animations = doc.get("animations") or []
    if not isinstance(nodes, list) or not isinstance(meshes, list):
        fail("inspection glTF lacks nodes/meshes arrays")
    if not nodes or not meshes:
        fail("inspection glTF contains no inspectable M700 nodes/meshes")

    node_names = [clean_name(node.get("name"), f"<node:{i}>") for i, node in enumerate(nodes)]
    mesh_names = [clean_name(mesh.get("name"), f"<mesh:{i}>") for i, mesh in enumerate(meshes)]

    parent_by_child: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child in node.get("children") or []:
            if not isinstance(child, int) or child < 0 or child >= len(nodes):
                fail(f"node {parent_index} contains invalid child {child!r}")
            if child in parent_by_child:
                fail(f"node {child} has multiple parents")
            parent_by_child[child] = parent_index

    def names_matching(needles: tuple[str, ...]) -> list[str]:
        out: list[str] = []
        seen: set[str] = set()
        for name in node_names + mesh_names:
            lower = name.lower()
            if any(needle in lower for needle in needles) and name not in seen:
                seen.add(name)
                out.append(name)
        return out

    def exact_node(name: str) -> int | None:
        matches = [i for i, value in enumerate(node_names) if value == name]
        if len(matches) > 1:
            fail(f"node identity {name} is not unique: {matches}")
        return matches[0] if matches else None

    def joint_row(index: int) -> dict[str, object]:
        parent = parent_by_child.get(index)
        return {
            "node_index": index,
            "node_name": node_names[index],
            "parent_index": parent,
            "parent_name": node_names[parent] if parent is not None else None,
            "children": [
                {"node_index": child, "node_name": node_names[child]}
                for child in (nodes[index].get("children") or [])
            ],
            "local_transform": node_local_transform(nodes[index]),
        }

    bolt_named = names_matching(("bolt",))
    mechanical_named = names_matching(("bolt", "breech", "action", "handle", "receiver"))
    identity_named = names_matching(("m700", "rifle", "trigger", "mag", "scope", "barrel", "stock"))

    animated_node_indices: set[int] = set()
    animation_rows: list[dict[str, object]] = []
    for animation_index, animation in enumerate(animations):
        channels = animation.get("channels") or []
        targeted: list[dict[str, object]] = []
        for channel in channels:
            target = channel.get("target") or {}
            node_index = target.get("node")
            path = target.get("path")
            if isinstance(node_index, int) and 0 <= node_index < len(nodes):
                animated_node_indices.add(node_index)
                targeted.append({
                    "node_index": node_index,
                    "node_name": node_names[node_index],
                    "path": path,
                })
        animation_rows.append({
            "animation_index": animation_index,
            "animation_name": clean_name(animation.get("name"), f"<animation:{animation_index}>"),
            "channel_count": len(channels),
            "targets": targeted,
        })

    animated_node_names = [node_names[i] for i in sorted(animated_node_indices)]
    animated_bolt_nodes = [name for name in animated_node_names if "bolt" in name.lower()]

    skinned_node_rows: list[dict[str, object]] = []
    for node_index, node in enumerate(nodes):
        skin_index = node.get("skin")
        if isinstance(skin_index, int):
            skinned_node_rows.append({
                "node_index": node_index,
                "node_name": node_names[node_index],
                "skin_index": skin_index,
                "mesh_index": node.get("mesh"),
            })

    joint_names: list[str] = []
    for skin_index, skin in enumerate(skins):
        for joint in skin.get("joints") or []:
            if isinstance(joint, int) and 0 <= joint < len(nodes):
                joint_names.append(node_names[joint])
            else:
                fail(f"skin {skin_index} contains invalid joint index {joint!r}")
    joint_names = list(dict.fromkeys(joint_names))
    bolt_joint_names = [name for name in joint_names if "bolt" in name.lower()]

    bolt_index = exact_node("BOLT")
    bolt_stop_index = exact_node("BOLT_STOP")
    bolt_hierarchy = [joint_row(i) for i in (bolt_index, bolt_stop_index) if i is not None]

    source_authored_stop_delta: dict[str, object] = {
        "usable_as_sibling_local_translation_delta": False,
        "reason": "BOLT_OR_BOLT_STOP_MISSING",
    }
    if bolt_index is not None and bolt_stop_index is not None:
        bolt_parent = parent_by_child.get(bolt_index)
        stop_parent = parent_by_child.get(bolt_stop_index)
        bolt_transform = node_local_transform(nodes[bolt_index])
        stop_transform = node_local_transform(nodes[bolt_stop_index])
        if bolt_parent != stop_parent:
            source_authored_stop_delta = {
                "usable_as_sibling_local_translation_delta": False,
                "reason": "DIFFERENT_PARENTS",
                "bolt_parent": node_names[bolt_parent] if bolt_parent is not None else None,
                "bolt_stop_parent": node_names[stop_parent] if stop_parent is not None else None,
            }
        elif bolt_transform.get("uses_matrix") or stop_transform.get("uses_matrix"):
            source_authored_stop_delta = {
                "usable_as_sibling_local_translation_delta": False,
                "reason": "MATRIX_TRANSFORM_REQUIRES_WORLD_DECOMPOSITION",
                "shared_parent": node_names[bolt_parent] if bolt_parent is not None else None,
            }
        else:
            bolt_translation = bolt_transform["translation"]
            stop_translation = stop_transform["translation"]
            delta = [
                float(stop_translation[i]) - float(bolt_translation[i])
                for i in range(3)
            ]
            magnitude = math.sqrt(sum(value * value for value in delta))
            source_authored_stop_delta = {
                "usable_as_sibling_local_translation_delta": magnitude > 1e-8,
                "reason": "SOURCE_SIBLING_MARKER_DELTA" if magnitude > 1e-8 else "ZERO_DELTA",
                "shared_parent": node_names[bolt_parent] if bolt_parent is not None else None,
                "bolt_translation": bolt_translation,
                "bolt_stop_translation": stop_translation,
                "delta": delta,
                "magnitude": magnitude,
            }

    if animated_bolt_nodes:
        classification = "DIRECT_AUTHORED_BOLT_MOTION_EVIDENCE"
    elif bolt_joint_names:
        classification = "BOLT_JOINT_DERIVATIVE_CANDIDATE_NO_EMBEDDED_BOLT_MOTION"
    elif bolt_named:
        classification = "BOLT_GEOMETRY_DERIVATIVE_CANDIDATE_NO_EMBEDDED_BOLT_MOTION"
    elif skins or animations:
        classification = "SKELETAL_OR_ANIMATED_SOURCE_REVIEW_REQUIRED_BOLT_IDENTITY_UNPROVEN"
    elif len(nodes) > 1 or len(meshes) > 1:
        classification = "PARTITIONED_SOURCE_REVIEW_REQUIRED_BOLT_IDENTITY_UNPROVEN"
    else:
        classification = "NO_DIRECT_BOLT_MOTION_OR_PART_IDENTITY_EVIDENCE"

    report = {
        "schema": 2,
        "status": "SOURCE_STRUCTURE_MOTION_EVIDENCE_ONLY",
        "source": EXPECTED_SOURCE,
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": digest,
        "source_size": len(payload),
        "inspection_converter": "Assimp -> glTF2 (CI inspection only)",
        "node_count": len(nodes),
        "mesh_count": len(meshes),
        "skin_count": len(skins),
        "animation_count": len(animations),
        "node_names": node_names,
        "mesh_names": mesh_names,
        "bolt_named_candidates": bolt_named,
        "mechanical_named_candidates": mechanical_named,
        "identity_named_candidates": identity_named,
        "joint_names": joint_names,
        "bolt_joint_names": bolt_joint_names,
        "bolt_hierarchy": bolt_hierarchy,
        "source_authored_stop_delta": source_authored_stop_delta,
        "skinned_nodes": skinned_node_rows,
        "animated_node_names": animated_node_names,
        "animated_bolt_nodes": animated_bolt_nodes,
        "animations": animation_rows,
        "classification": classification,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    print(
        "PASS45 M700 SOURCE MOTION AUDIT: COMPLETE "
        f"nodes={len(nodes)} meshes={len(meshes)} skins={len(skins)} animations={len(animations)} "
        f"bolt_named={len(bolt_named)} bolt_joints={len(bolt_joint_names)} "
        f"animated_bolt_nodes={len(animated_bolt_nodes)} classification={classification} "
        f"source_stop_delta_usable={int(bool(source_authored_stop_delta.get('usable_as_sibling_local_translation_delta')))} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
