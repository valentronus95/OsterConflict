#!/usr/bin/env python3
"""Fail-closed structural, motion and joint-geometry audit for Stein CC0 Lever Action.

The audit proves only what the pinned source actually contains. It does not close
PASS45 item 16, does not invent a lever pivot/range, and does not claim UE runtime
acceptance. GitHub Actions materializes only the exact LFS payload and converts it
to inspection-only glTF2 with Assimp. Mechanical name matches are discovery hints,
not acceptance evidence unless source hierarchy/skin/geometry/animation facts
support them.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path
from typing import Any

EXPECTED_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
EXPECTED_SIZE = 570332
EXPECTED_SOURCE = (
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/"
    "WeaponsPack/LeverAction/SKM_LeverAction.fbx"
)
MECHANICAL_TERMS = ("lever", "action", "bolt", "breech", "hammer", "handle")
IDENTITY_TERMS = ("leveraction", "rifle", "trigger", "mag", "barrel", "stock", "receiver")
GEOMETRY_TARGETS = ("LEVER", "HAMMER", "BOLT")
COMPONENT_FORMATS = {5120: "b", 5121: "B", 5122: "h", 5123: "H", 5125: "I", 5126: "f"}
TYPE_COMPONENTS = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}
WEIGHT_EPSILON = 1e-5
DOMINANT_EPSILON = 1e-8


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 LEVER ACTION SOURCE MOTION AUDIT: FAIL\n[FAIL] {message}")


def clean_name(value: object, fallback: str) -> str:
    text = str(value or "").strip()
    return text or fallback


def finite_vec(value: object, width: int) -> list[float] | None:
    if not isinstance(value, list) or len(value) != width:
        return None
    try:
        result = [float(v) for v in value]
    except (TypeError, ValueError):
        return None
    return result if all(math.isfinite(v) for v in result) else None


def node_transform(node: dict[str, Any]) -> dict[str, object]:
    matrix = finite_vec(node.get("matrix"), 16)
    if matrix is not None:
        return {
            "uses_matrix": True,
            "matrix": matrix,
            "translation": [matrix[12], matrix[13], matrix[14]],
        }
    return {
        "uses_matrix": False,
        "translation": finite_vec(node.get("translation"), 3) or [0.0, 0.0, 0.0],
        "rotation": finite_vec(node.get("rotation"), 4) or [0.0, 0.0, 0.0, 1.0],
        "scale": finite_vec(node.get("scale"), 3) or [1.0, 1.0, 1.0],
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
    if not 0 <= accessor_index < len(accessors):
        fail(f"invalid accessor index {accessor_index}")
    accessor = accessors[accessor_index]
    if accessor.get("sparse") is not None:
        fail(f"sparse accessor {accessor_index} is unsupported")
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or not 0 <= view_index < len(views):
        fail(f"accessor {accessor_index} lacks a valid bufferView")
    view = views[view_index]
    buffer_index = view.get("buffer", 0)
    if not isinstance(buffer_index, int) or not 0 <= buffer_index < len(buffers):
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


def empty_stats() -> dict[str, Any]:
    return {
        "weighted_vertices": set(),
        "weighted_vertex_count": 0,
        "dominant_vertex_count": 0,
        "full_weight_vertex_count": 0,
        "total_weight": 0.0,
        "bounds_min": None,
        "bounds_max": None,
        "weighted_centroid_accumulator": [0.0, 0.0, 0.0],
        "vertex_centroid_accumulator": [0.0, 0.0, 0.0],
    }


def update_bounds(stats: dict[str, Any], xyz: list[float]) -> None:
    if stats["bounds_min"] is None:
        stats["bounds_min"] = xyz[:]
        stats["bounds_max"] = xyz[:]
        return
    stats["bounds_min"] = [min(float(stats["bounds_min"][i]), xyz[i]) for i in range(3)]
    stats["bounds_max"] = [max(float(stats["bounds_max"][i]), xyz[i]) for i in range(3)]


def finalize_stats(stats: dict[str, Any]) -> dict[str, Any]:
    count = int(stats["weighted_vertex_count"])
    total_weight = float(stats["total_weight"])
    bounds_min = stats["bounds_min"]
    bounds_max = stats["bounds_max"]
    extent = None
    volume = None
    if bounds_min is not None and bounds_max is not None:
        extent = [float(bounds_max[i]) - float(bounds_min[i]) for i in range(3)]
        volume = math.prod(extent)
    weighted_centroid = None
    if total_weight > WEIGHT_EPSILON:
        weighted_centroid = [float(v) / total_weight for v in stats["weighted_centroid_accumulator"]]
    vertex_centroid = None
    if count > 0:
        vertex_centroid = [float(v) / count for v in stats["vertex_centroid_accumulator"]]
    return {
        "weighted_vertex_count": count,
        "dominant_vertex_count": int(stats["dominant_vertex_count"]),
        "full_weight_vertex_count": int(stats["full_weight_vertex_count"]),
        "total_weight": total_weight,
        "bounds_min": bounds_min,
        "bounds_max": bounds_max,
        "aabb_extent": extent,
        "aabb_volume": volume,
        "weighted_centroid": weighted_centroid,
        "vertex_centroid": vertex_centroid,
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
        fail("Lever Action LFS payload was not materialized; refusing pointer text")
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
    materials = doc.get("materials") or []
    if not isinstance(nodes, list) or not isinstance(meshes, list) or not nodes or not meshes:
        fail("inspection glTF lacks inspectable nodes/meshes")
    if not isinstance(skins, list) or not isinstance(animations, list) or not isinstance(materials, list):
        fail("inspection glTF has malformed skins/animations/materials")

    node_names = [clean_name(node.get("name"), f"<node:{i}>") for i, node in enumerate(nodes)]
    mesh_names = [clean_name(mesh.get("name"), f"<mesh:{i}>") for i, mesh in enumerate(meshes)]
    material_names = [clean_name(material.get("name"), f"<material:{i}>") for i, material in enumerate(materials)]
    buffers = load_buffers(doc, gltf_path)

    parent_by_child: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child in node.get("children") or []:
            if not isinstance(child, int) or not 0 <= child < len(nodes):
                fail(f"node {parent_index} has invalid child {child!r}")
            if child in parent_by_child:
                fail(f"node {child} has multiple parents")
            parent_by_child[child] = parent_index

    def matching(terms: tuple[str, ...]) -> list[dict[str, object]]:
        rows: list[dict[str, object]] = []
        for index, name in enumerate(node_names):
            lower = name.lower()
            hits = [term for term in terms if term in lower]
            if hits:
                parent = parent_by_child.get(index)
                rows.append({
                    "node_index": index,
                    "node_name": name,
                    "matched_terms": hits,
                    "parent_index": parent,
                    "parent_name": node_names[parent] if parent is not None else None,
                    "children": [node_names[c] for c in (nodes[index].get("children") or [])],
                    "local_transform": node_transform(nodes[index]),
                })
        return rows

    mechanical_nodes = matching(MECHANICAL_TERMS)
    identity_nodes = matching(IDENTITY_TERMS)

    joint_indices: set[int] = set()
    skin_rows: list[dict[str, object]] = []
    for skin_index, skin in enumerate(skins):
        joints = skin.get("joints") or []
        if not isinstance(joints, list):
            fail(f"skin {skin_index} joints are malformed")
        names: list[str] = []
        for joint in joints:
            if not isinstance(joint, int) or not 0 <= joint < len(nodes):
                fail(f"skin {skin_index} contains invalid joint {joint!r}")
            joint_indices.add(joint)
            names.append(node_names[joint])
        skin_rows.append({"skin_index": skin_index, "joint_count": len(joints), "joint_names": names})

    mechanical_joint_names = [
        node_names[i] for i in sorted(joint_indices)
        if any(term in node_names[i].lower() for term in MECHANICAL_TERMS)
    ]

    animated_indices: set[int] = set()
    animation_rows: list[dict[str, object]] = []
    mechanical_animation_targets: list[dict[str, object]] = []
    for animation_index, animation in enumerate(animations):
        channels = animation.get("channels") or []
        if not isinstance(channels, list):
            fail(f"animation {animation_index} channels are malformed")
        targets: list[dict[str, object]] = []
        for channel_index, channel in enumerate(channels):
            target = channel.get("target") or {}
            node_index = target.get("node")
            path = target.get("path")
            if not isinstance(node_index, int) or not 0 <= node_index < len(nodes):
                continue
            animated_indices.add(node_index)
            row = {"channel_index": channel_index, "node_index": node_index, "node_name": node_names[node_index], "path": path}
            targets.append(row)
            if any(term in node_names[node_index].lower() for term in MECHANICAL_TERMS):
                mechanical_animation_targets.append({"animation_index": animation_index, **row})
        animation_rows.append({
            "animation_index": animation_index,
            "animation_name": clean_name(animation.get("name"), f"<animation:{animation_index}>"),
            "channel_count": len(channels),
            "targets": targets,
        })

    target_nodes: dict[str, int] = {}
    for target in GEOMETRY_TARGETS:
        matches = [i for i, name in enumerate(node_names) if name == target]
        if len(matches) != 1:
            fail(f"expected exactly one {target} node, got {matches}")
        target_nodes[target] = matches[0]

    raw_stats = {target: empty_stats() for target in GEOMETRY_TARGETS}
    shared_pairs: dict[str, set[str]] = {
        f"{a}+{b}": set()
        for i, a in enumerate(GEOMETRY_TARGETS)
        for b in GEOMETRY_TARGETS[i + 1:]
    }
    primitive_rows: list[dict[str, Any]] = []

    for node_index, node in enumerate(nodes):
        skin_index = node.get("skin")
        mesh_index = node.get("mesh")
        if not isinstance(skin_index, int) or not isinstance(mesh_index, int):
            continue
        if not 0 <= skin_index < len(skins) or not 0 <= mesh_index < len(meshes):
            fail(f"skinned node {node_index} references invalid skin/mesh")
        skin_joints = skins[skin_index].get("joints") or []
        slots = {
            target: skin_joints.index(target_node) if target_node in skin_joints else None
            for target, target_node in target_nodes.items()
        }
        if all(slot is None for slot in slots.values()):
            continue

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

            material_index = primitive.get("material")
            material_name = material_names[material_index] if isinstance(material_index, int) and 0 <= material_index < len(material_names) else None
            primitive_counts = {target: 0 for target in GEOMETRY_TARGETS}
            primitive_shared = {key: 0 for key in shared_pairs}

            for vertex_index, position in enumerate(positions):
                if len(position) < 3:
                    fail("POSITION accessor is not VEC3")
                xyz = [float(position[0]), float(position[1]), float(position[2])]
                if not all(math.isfinite(v) for v in xyz):
                    fail("POSITION accessor contains non-finite value")

                slot_weights: dict[int, float] = {}
                for joints_rows, weights_rows in influence_sets:
                    joints = joints_rows[vertex_index]
                    weights = weights_rows[vertex_index]
                    if len(joints) != len(weights):
                        fail("JOINTS/WEIGHTS tuple width mismatch")
                    for joint_value, weight_value in zip(joints, weights):
                        slot = int(joint_value)
                        weight = float(weight_value)
                        if weight > WEIGHT_EPSILON:
                            slot_weights[slot] = slot_weights.get(slot, 0.0) + weight

                target_weights = {
                    target: slot_weights.get(int(slot), 0.0) if slot is not None else 0.0
                    for target, slot in slots.items()
                }
                positive = [target for target, weight in target_weights.items() if weight > WEIGHT_EPSILON]
                if not positive:
                    continue
                vertex_key = f"{node_index}:{mesh_index}:{primitive_index}:{vertex_index}"
                max_weight = max(slot_weights.values()) if slot_weights else 0.0

                for pair_key, pair_vertices in shared_pairs.items():
                    a, b = pair_key.split("+")
                    if a in positive and b in positive:
                        pair_vertices.add(vertex_key)
                        primitive_shared[pair_key] += 1

                for target in positive:
                    weight = target_weights[target]
                    stats = raw_stats[target]
                    if vertex_key in stats["weighted_vertices"]:
                        fail(f"duplicate target vertex identity encountered: {vertex_key}")
                    stats["weighted_vertices"].add(vertex_key)
                    stats["weighted_vertex_count"] += 1
                    stats["total_weight"] += weight
                    primitive_counts[target] += 1
                    if abs(weight - max_weight) <= DOMINANT_EPSILON:
                        stats["dominant_vertex_count"] += 1
                    if weight >= 1.0 - WEIGHT_EPSILON:
                        stats["full_weight_vertex_count"] += 1
                    update_bounds(stats, xyz)
                    for axis in range(3):
                        stats["weighted_centroid_accumulator"][axis] += xyz[axis] * weight
                        stats["vertex_centroid_accumulator"][axis] += xyz[axis]

            if any(primitive_counts.values()):
                primitive_rows.append({
                    "node_index": node_index,
                    "node_name": node_names[node_index],
                    "skin_index": skin_index,
                    "mesh_index": mesh_index,
                    "mesh_name": mesh_names[mesh_index],
                    "primitive_index": primitive_index,
                    "material_index": material_index if isinstance(material_index, int) else None,
                    "material_name": material_name,
                    "weighted_vertex_count": primitive_counts,
                    "shared_target_vertex_count": primitive_shared,
                })

    finalized = {target: finalize_stats(raw_stats[target]) for target in GEOMETRY_TARGETS}
    lever_count = int(finalized["LEVER"]["weighted_vertex_count"])
    if lever_count > 0:
        geometry_classification = "LEVER_WEIGHTED_GEOMETRY_DERIVATIVE_CANDIDATE"
    else:
        geometry_classification = "LEVER_JOINT_WITHOUT_WEIGHTED_GEOMETRY_REVIEW_REQUIRED"

    if mechanical_animation_targets:
        motion_classification = "DIRECT_AUTHORED_MECHANICAL_MOTION_EVIDENCE"
    elif mechanical_joint_names:
        motion_classification = "MECHANICAL_JOINT_DERIVATIVE_CANDIDATE_NO_EMBEDDED_MOTION"
    elif mechanical_nodes:
        motion_classification = "MECHANICAL_NODE_DERIVATIVE_CANDIDATE_NO_EMBEDDED_MOTION"
    elif skins or animations:
        motion_classification = "SKELETAL_OR_ANIMATED_SOURCE_MECHANICAL_IDENTITY_UNPROVEN"
    elif len(nodes) > 1 or len(meshes) > 1:
        motion_classification = "PARTITIONED_SOURCE_MECHANICAL_IDENTITY_UNPROVEN"
    else:
        motion_classification = "NO_DIRECT_MECHANICAL_MOTION_OR_PART_IDENTITY_EVIDENCE"

    report: dict[str, Any] = {
        "schema": 2,
        "status": "SOURCE_STRUCTURE_MOTION_AND_GEOMETRY_EVIDENCE_ONLY",
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
        "mechanical_named_candidates": mechanical_nodes,
        "identity_named_candidates": identity_nodes,
        "skins": skin_rows,
        "mechanical_joint_names": mechanical_joint_names,
        "animated_node_names": [node_names[i] for i in sorted(animated_indices)],
        "mechanical_animation_targets": mechanical_animation_targets,
        "animations": animation_rows,
        "motion_classification": motion_classification,
        "target_nodes": target_nodes,
        "joint_geometry": finalized,
        "shared_weighted_vertices": {key: len(vertices) for key, vertices in shared_pairs.items()},
        "primitive_material_ownership": primitive_rows,
        "lever_joint_has_weighted_geometry": lever_count > 0,
        "geometry_classification": geometry_classification,
        "source_authored_lever_angle_or_endpoint": False,
        "derived_motion_parameters_authored": False,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(
        "PASS45 LEVER ACTION SOURCE MOTION AUDIT: COMPLETE "
        f"nodes={len(nodes)} meshes={len(meshes)} skins={len(skins)} animations={len(animations)} "
        f"lever_vertices={lever_count} hammer_vertices={finalized['HAMMER']['weighted_vertex_count']} "
        f"bolt_vertices={finalized['BOLT']['weighted_vertex_count']} "
        f"motion={motion_classification} geometry={geometry_classification} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
