#!/usr/bin/env python3
"""Fail-closed structural/motion audit for the committed Stein CC0 M700 FBX.

This audit cannot close PASS45 item 16. GitHub Actions materializes only the exact
M700 LFS payload, Assimp converts it to an inspection-only glTF, and this script
records what the source actually contains.

Schema 3 proves three facts needed before an authored bolt derivative is allowed:
1. BOLT and BOLT_STOP share the same parent and expose a source-authored local
   endpoint delta;
2. BOLT is a real skin joint that influences weapon geometry;
3. BOLT_STOP is classified from actual skin influence rather than its name alone.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path
from typing import Any

EXPECTED_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
EXPECTED_SIZE = 638732
EXPECTED_SOURCE = (
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/"
    "WeaponsPack/M700/SKM_M700.fbx"
)
COMPONENT_FORMATS = {5120: "b", 5121: "B", 5122: "h", 5123: "H", 5125: "I", 5126: "f"}
TYPE_COMPONENTS = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}


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


def node_local_transform(node: dict[str, Any]) -> dict[str, object]:
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
        if any(abs(values[i]) > 1e-8 for i in (3, 7, 11)) or abs(values[15] - 1.0) > 1e-8:
            fail("non-affine node matrix is unsupported")
        return {
            "matrix": values,
            "translation": [values[12], values[13], values[14]],
            "uses_matrix": True,
        }
    return {
        "translation": vec3(node.get("translation"), (0.0, 0.0, 0.0)),
        "rotation": quat4(node.get("rotation")),
        "scale": vec3(node.get("scale"), (1.0, 1.0, 1.0)),
        "uses_matrix": False,
    }


def load_buffers(doc: dict[str, Any], gltf_path: Path) -> list[bytes]:
    result: list[bytes] = []
    for index, buffer in enumerate(doc.get("buffers") or []):
        uri = buffer.get("uri")
        if not isinstance(uri, str) or not uri or uri.startswith("data:"):
            fail(f"buffer {index} does not use a supported external URI")
        path = (gltf_path.parent / uri).resolve()
        if not path.is_file():
            fail(f"buffer {index} payload missing: {path}")
        payload = path.read_bytes()
        expected = buffer.get("byteLength")
        if isinstance(expected, int) and len(payload) < expected:
            fail(f"buffer {index} shorter than declared byteLength")
        result.append(payload)
    if not result:
        fail("inspection glTF contains no external buffers")
    return result


def normalized_value(value: int | float, component_type: int, normalized: bool) -> float | int:
    if not normalized or component_type == 5126:
        return value
    if component_type == 5120:
        return max(float(value) / 127.0, -1.0)
    if component_type == 5121:
        return float(value) / 255.0
    if component_type == 5122:
        return max(float(value) / 32767.0, -1.0)
    if component_type == 5123:
        return float(value) / 65535.0
    if component_type == 5125:
        return float(value) / 4294967295.0
    return value


def read_accessor(doc: dict[str, Any], buffers: list[bytes], accessor_index: int) -> list[tuple[float | int, ...]]:
    accessors = doc.get("accessors") or []
    views = doc.get("bufferViews") or []
    if not (0 <= accessor_index < len(accessors)):
        fail(f"invalid accessor index {accessor_index}")
    accessor = accessors[accessor_index]
    if accessor.get("sparse") is not None:
        fail(f"sparse accessor {accessor_index} is unsupported")
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or not (0 <= view_index < len(views)):
        fail(f"accessor {accessor_index} lacks a valid bufferView")
    view = views[view_index]
    buffer_index = view.get("buffer", 0)
    if not isinstance(buffer_index, int) or not (0 <= buffer_index < len(buffers)):
        fail(f"bufferView {view_index} has invalid buffer index")
    component_type = accessor.get("componentType")
    fmt = COMPONENT_FORMATS.get(component_type)
    type_name = accessor.get("type")
    components = TYPE_COMPONENTS.get(type_name)
    count = accessor.get("count")
    if fmt is None or components is None or not isinstance(count, int) or count < 0:
        fail(f"accessor {accessor_index} has unsupported metadata")
    component_size = struct.calcsize("<" + fmt)
    packed_size = component_size * components
    stride = view.get("byteStride", packed_size)
    if not isinstance(stride, int) or stride < packed_size:
        fail(f"accessor {accessor_index} has invalid byteStride {stride!r}")
    base = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    payload = buffers[buffer_index]
    normalized = bool(accessor.get("normalized", False))
    rows: list[tuple[float | int, ...]] = []
    unpack_fmt = "<" + fmt * components
    for row_index in range(count):
        offset = base + row_index * stride
        if offset + packed_size > len(payload):
            fail(f"accessor {accessor_index} overruns buffer")
        raw = struct.unpack_from(unpack_fmt, payload, offset)
        rows.append(tuple(normalized_value(v, component_type, normalized) for v in raw))
    return rows


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
    if not isinstance(nodes, list) or not isinstance(meshes, list) or not nodes or not meshes:
        fail("inspection glTF lacks inspectable nodes/meshes")
    buffers = load_buffers(doc, gltf_path)
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
                targeted.append({"node_index": node_index, "node_name": node_names[node_index], "path": path})
        animation_rows.append({
            "animation_index": animation_index,
            "animation_name": clean_name(animation.get("name"), f"<animation:{animation_index}>"),
            "channel_count": len(channels),
            "targets": targeted,
        })
    animated_node_names = [node_names[i] for i in sorted(animated_node_indices)]
    animated_bolt_nodes = [name for name in animated_node_names if "bolt" in name.lower()]

    skinned_node_rows: list[dict[str, object]] = []
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
        else:
            bolt_translation = [float(v) for v in bolt_transform["translation"]]
            stop_translation = [float(v) for v in stop_transform["translation"]]
            delta = [stop_translation[i] - bolt_translation[i] for i in range(3)]
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

    influence_stats: dict[str, dict[str, object]] = {
        "BOLT": {"weighted_vertex_count": 0, "total_weight": 0.0, "bounds_min": None, "bounds_max": None},
        "BOLT_STOP": {"weighted_vertex_count": 0, "total_weight": 0.0, "bounds_min": None, "bounds_max": None},
    }
    target_nodes = {"BOLT": bolt_index, "BOLT_STOP": bolt_stop_index}
    for node_index, node in enumerate(nodes):
        skin_index = node.get("skin")
        mesh_index = node.get("mesh")
        if not isinstance(skin_index, int) or not isinstance(mesh_index, int):
            continue
        if not (0 <= skin_index < len(skins) and 0 <= mesh_index < len(meshes)):
            fail(f"skinned node {node_index} references invalid skin/mesh")
        skin_joints = skins[skin_index].get("joints") or []
        slot_for_target = {
            name: skin_joints.index(target) if target is not None and target in skin_joints else None
            for name, target in target_nodes.items()
        }
        row = {
            "node_index": node_index,
            "node_name": node_names[node_index],
            "skin_index": skin_index,
            "mesh_index": mesh_index,
            "mesh_name": mesh_names[mesh_index],
            "target_joint_slots": slot_for_target,
        }
        skinned_node_rows.append(row)
        for primitive_index, primitive in enumerate(meshes[mesh_index].get("primitives") or []):
            attrs = primitive.get("attributes") or {}
            pos_accessor = attrs.get("POSITION")
            if not isinstance(pos_accessor, int):
                fail(f"mesh {mesh_index} primitive {primitive_index} lacks POSITION")
            positions = read_accessor(doc, buffers, pos_accessor)
            influence_sets: list[tuple[list[tuple[float | int, ...]], list[tuple[float | int, ...]]]] = []
            set_index = 0
            while f"JOINTS_{set_index}" in attrs or f"WEIGHTS_{set_index}" in attrs:
                joints_accessor = attrs.get(f"JOINTS_{set_index}")
                weights_accessor = attrs.get(f"WEIGHTS_{set_index}")
                if not isinstance(joints_accessor, int) or not isinstance(weights_accessor, int):
                    fail(f"mesh {mesh_index} primitive {primitive_index} has incomplete skin influence set {set_index}")
                joints_rows = read_accessor(doc, buffers, joints_accessor)
                weights_rows = read_accessor(doc, buffers, weights_accessor)
                if len(joints_rows) != len(positions) or len(weights_rows) != len(positions):
                    fail(f"mesh {mesh_index} primitive {primitive_index} skin accessor count mismatch")
                influence_sets.append((joints_rows, weights_rows))
                set_index += 1
            if not influence_sets:
                fail(f"skinned mesh {mesh_index} primitive {primitive_index} lacks JOINTS/WEIGHTS")
            for vertex_index, position in enumerate(positions):
                if len(position) < 3:
                    fail("POSITION accessor is not VEC3")
                for target_name, target_slot in slot_for_target.items():
                    if target_slot is None:
                        continue
                    weight = 0.0
                    for joints_rows, weights_rows in influence_sets:
                        joints = joints_rows[vertex_index]
                        weights = weights_rows[vertex_index]
                        if len(joints) != len(weights):
                            fail("JOINTS/WEIGHTS tuple width mismatch")
                        for joint_value, weight_value in zip(joints, weights):
                            if int(joint_value) == target_slot:
                                weight += float(weight_value)
                    if weight <= 1e-5:
                        continue
                    stats = influence_stats[target_name]
                    stats["weighted_vertex_count"] = int(stats["weighted_vertex_count"]) + 1
                    stats["total_weight"] = float(stats["total_weight"]) + weight
                    xyz = [float(position[0]), float(position[1]), float(position[2])]
                    if stats["bounds_min"] is None:
                        stats["bounds_min"] = xyz[:]
                        stats["bounds_max"] = xyz[:]
                    else:
                        stats["bounds_min"] = [min(float(stats["bounds_min"][i]), xyz[i]) for i in range(3)]
                        stats["bounds_max"] = [max(float(stats["bounds_max"][i]), xyz[i]) for i in range(3)]

    bolt_weighted = int(influence_stats["BOLT"]["weighted_vertex_count"])
    stop_weighted = int(influence_stats["BOLT_STOP"]["weighted_vertex_count"])
    endpoint_classification = "UNPROVEN"
    if bolt_weighted > 0 and stop_weighted == 0 and source_authored_stop_delta.get("usable_as_sibling_local_translation_delta"):
        endpoint_classification = "BOLT_GEOMETRY_PLUS_UNWEIGHTED_BOLT_STOP_ENDPOINT_MARKER"
    elif bolt_weighted > 0 and stop_weighted > 0:
        endpoint_classification = "BOLT_AND_BOLT_STOP_BOTH_WEIGHT_GEOMETRY_REVIEW_REQUIRED"
    elif bolt_weighted == 0:
        endpoint_classification = "BOLT_JOINT_DOES_NOT_WEIGHT_GEOMETRY_REVIEW_REQUIRED"

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
        "schema": 3,
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
        "bolt_skin_influence": influence_stats,
        "bolt_endpoint_classification": endpoint_classification,
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
        f"bolt_weighted_vertices={bolt_weighted} bolt_stop_weighted_vertices={stop_weighted} "
        f"endpoint={endpoint_classification} classification={classification} "
        f"source_stop_delta_usable={int(bool(source_authored_stop_delta.get('usable_as_sibling_local_translation_delta')))} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
