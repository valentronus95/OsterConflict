#!/usr/bin/env python3
"""Deterministic structural/geometry audit for the exact pinned Remington 870 donor.

This is source evidence only. It reuses the already-pinned remote donor audit and
binary accessor helpers, does not import anything into Unreal, does not create a
production asset, and does not identify a node as the physical pump unless the
recorded hierarchy/geometry can support that conclusion separately.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
TARGET_NODES = ("Rif_059", "Trigger_060", "PBody_058", "Pmag_061")
ACTION_CLIPS = ((2, "fire"), (3, "easy_reload"), (4, "full_reload"))


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 STRUCTURE AUDIT: FAIL\n[FAIL] {message}")


def clean_name(value: object, fallback: str) -> str:
    text = str(value or "").strip()
    return text or fallback


def build_parent_map(nodes: list[dict]) -> dict[int, int]:
    parents: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child in node.get("children") or []:
            if not isinstance(child, int) or child < 0 or child >= len(nodes):
                fail(f"node {parent_index} has invalid child {child!r}")
            if child in parents:
                fail(f"node {child} has multiple parents ({parents[child]}, {parent_index})")
            parents[child] = parent_index
    return parents


def node_index_by_name(nodes: list[dict], name: str) -> int:
    matches = [i for i, node in enumerate(nodes) if str(node.get("name") or "") == name]
    if len(matches) != 1:
        fail(f"expected exactly one node named {name}, found {matches}")
    return matches[0]


def descendant_indices(nodes: list[dict], start: int) -> list[int]:
    ordered: list[int] = []
    stack = list(reversed(nodes[start].get("children") or []))
    seen: set[int] = set()
    while stack:
        index = stack.pop()
        if index in seen:
            fail(f"cycle/repeated descendant detected under node {start}: {index}")
        seen.add(index)
        ordered.append(index)
        children = nodes[index].get("children") or []
        stack.extend(reversed(children))
    return ordered


def ancestor_rows(nodes: list[dict], parents: dict[int, int], start: int) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    current = start
    seen: set[int] = set()
    while current in parents:
        parent = parents[current]
        if parent in seen:
            fail(f"ancestor cycle detected for node {start}")
        seen.add(parent)
        rows.append({
            "node_index": parent,
            "node_name": clean_name(nodes[parent].get("name"), f"<unnamed:{parent}>")
        })
        current = parent
    return rows


def raw_accessor_values(doc: dict, binary_payload: bytes, accessor_index: int) -> list[tuple[float, ...]]:
    accessors = doc.get("accessors") or []
    buffer_views = doc.get("bufferViews") or []
    if accessor_index < 0 or accessor_index >= len(accessors):
        fail(f"invalid accessor {accessor_index}")
    accessor = accessors[accessor_index]
    if accessor.get("sparse"):
        fail(f"sparse accessor {accessor_index} unsupported in fail-closed geometry audit")
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or view_index < 0 or view_index >= len(buffer_views):
        fail(f"accessor {accessor_index} missing valid bufferView")
    view = buffer_views[view_index]
    if view.get("buffer", 0) != 0:
        fail(f"accessor {accessor_index} references nonzero buffer")

    component_formats = {
        5120: ("b", 1, -128, 127),
        5121: ("B", 1, 0, 255),
        5122: ("h", 2, -32768, 32767),
        5123: ("H", 2, 0, 65535),
        5125: ("I", 4, 0, 4294967295),
        5126: ("f", 4, None, None),
    }
    type_components = {
        "SCALAR": 1,
        "VEC2": 2,
        "VEC3": 3,
        "VEC4": 4,
    }
    component_type = accessor.get("componentType")
    accessor_type = accessor.get("type")
    if component_type not in component_formats:
        fail(f"accessor {accessor_index} unsupported componentType {component_type!r}")
    component_count = type_components.get(accessor_type)
    if component_count is None:
        fail(f"accessor {accessor_index} unsupported type {accessor_type!r}")
    count = accessor.get("count")
    if not isinstance(count, int) or count < 1:
        fail(f"accessor {accessor_index} invalid count {count!r}")

    fmt_char, component_size, signed_min, signed_max = component_formats[component_type]
    packed_size = component_count * component_size
    stride = view.get("byteStride", packed_size)
    if not isinstance(stride, int) or stride < packed_size:
        fail(f"accessor {accessor_index} invalid stride {stride!r}")
    base_offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    fmt = "<" + (fmt_char * component_count)
    normalized = bool(accessor.get("normalized"))
    values: list[tuple[float, ...]] = []

    for item_index in range(count):
        item_offset = base_offset + item_index * stride
        if item_offset < 0 or item_offset + packed_size > len(binary_payload):
            fail(f"accessor {accessor_index} exceeds BIN chunk")
        raw = struct.unpack_from(fmt, binary_payload, item_offset)
        converted: list[float] = []
        for value in raw:
            if component_type == 5126 or not normalized:
                converted.append(float(value))
            elif signed_min is not None and signed_min < 0:
                denominator = float(signed_max)
                converted.append(max(-1.0, float(value) / denominator))
            else:
                converted.append(float(value) / float(signed_max))
        values.append(tuple(converted))
    return values


def accessor_bounds(doc: dict, binary_payload: bytes, accessor_index: int) -> dict[str, object]:
    accessors = doc.get("accessors") or []
    if accessor_index < 0 or accessor_index >= len(accessors):
        fail(f"invalid POSITION accessor {accessor_index}")
    accessor = accessors[accessor_index]
    count = accessor.get("count")
    minimum = accessor.get("min")
    maximum = accessor.get("max")
    if (
        isinstance(minimum, list) and isinstance(maximum, list)
        and len(minimum) == 3 and len(maximum) == 3
    ):
        mn = [float(v) for v in minimum]
        mx = [float(v) for v in maximum]
    else:
        values = raw_accessor_values(doc, binary_payload, accessor_index)
        if not values or len(values[0]) != 3:
            fail(f"POSITION accessor {accessor_index} is not VEC3")
        mn = [min(v[axis] for v in values) for axis in range(3)]
        mx = [max(v[axis] for v in values) for axis in range(3)]
    extent = [mx[i] - mn[i] for i in range(3)]
    return {
        "accessor_index": accessor_index,
        "vertex_count": count,
        "min": [round(v, 6) for v in mn],
        "max": [round(v, 6) for v in mx],
        "extent": [round(v, 6) for v in extent],
    }


def mesh_rows_for_subtree(
    doc: dict,
    binary_payload: bytes,
    nodes: list[dict],
    subtree: list[int],
) -> list[dict[str, object]]:
    meshes = doc.get("meshes") or []
    materials = doc.get("materials") or []
    rows: list[dict[str, object]] = []
    for node_index in subtree:
        node = nodes[node_index]
        mesh_index = node.get("mesh")
        if not isinstance(mesh_index, int):
            continue
        if mesh_index < 0 or mesh_index >= len(meshes):
            fail(f"node {node_index} references invalid mesh {mesh_index}")
        mesh = meshes[mesh_index]
        primitive_rows: list[dict[str, object]] = []
        for primitive_index, primitive in enumerate(mesh.get("primitives") or []):
            attributes = primitive.get("attributes") or {}
            position_accessor = attributes.get("POSITION")
            if not isinstance(position_accessor, int):
                fail(f"mesh {mesh_index} primitive {primitive_index} has no POSITION accessor")
            material_index = primitive.get("material")
            material_name = "NONE"
            if isinstance(material_index, int):
                if material_index < 0 or material_index >= len(materials):
                    fail(f"mesh {mesh_index} primitive {primitive_index} has invalid material {material_index}")
                material_name = clean_name(materials[material_index].get("name"), f"<material:{material_index}>")
            primitive_rows.append({
                "primitive_index": primitive_index,
                "material_index": material_index,
                "material_name": material_name,
                "position": accessor_bounds(doc, binary_payload, position_accessor),
            })
        rows.append({
            "node_index": node_index,
            "node_name": clean_name(node.get("name"), f"<unnamed:{node_index}>"),
            "mesh_index": mesh_index,
            "mesh_name": clean_name(mesh.get("name"), f"<mesh:{mesh_index}>"),
            "skin_index": node.get("skin"),
            "primitives": primitive_rows,
        })
    return rows


def translation_delta_summary(doc: dict, binary_payload: bytes, node_index: int) -> dict[str, object]:
    animations = doc.get("animations") or []
    result: dict[str, object] = {}
    for animation_index, semantic in ACTION_CLIPS:
        if animation_index >= len(animations):
            fail(f"missing animation {animation_index} ({semantic})")
        animation = animations[animation_index]
        samplers = animation.get("samplers") or []
        translation_values: list[tuple[float, ...]] | None = None
        for channel in animation.get("channels") or []:
            target = channel.get("target") or {}
            if target.get("node") != node_index or target.get("path") != "translation":
                continue
            sampler_index = channel.get("sampler")
            if not isinstance(sampler_index, int) or sampler_index < 0 or sampler_index >= len(samplers):
                fail(f"invalid translation sampler for node {node_index} in animation {animation_index}")
            translation_values = acquire.sampled_animation_output(doc, binary_payload, samplers[sampler_index])
            break

        if not translation_values:
            result[f"{semantic}_index_{animation_index}"] = {"translation_channel": False}
            continue
        if len(translation_values[0]) != 3:
            fail(f"translation output for node {node_index} is not VEC3")
        origin = translation_values[0]
        deltas = [tuple(sample[i] - origin[i] for i in range(3)) for sample in translation_values]
        delta_min = [min(v[i] for v in deltas) for i in range(3)]
        delta_max = [max(v[i] for v in deltas) for i in range(3)]
        peak = max(math.sqrt(sum(component * component for component in value)) for value in deltas)
        axis_ranges = [delta_max[i] - delta_min[i] for i in range(3)]
        dominant_axis = max(range(3), key=lambda i: axis_ranges[i])
        result[f"{semantic}_index_{animation_index}"] = {
            "translation_channel": True,
            "sample_count": len(translation_values),
            "delta_min": [round(v, 6) for v in delta_min],
            "delta_max": [round(v, 6) for v in delta_max],
            "axis_range": [round(v, 6) for v in axis_ranges],
            "dominant_axis": ("X", "Y", "Z")[dominant_axis],
            "peak_displacement": round(peak, 6),
        }
    return result


def influence_bounds(positions: list[tuple[float, ...]], weights: list[float], threshold: float) -> dict[str, object] | None:
    selected = [(position, weight) for position, weight in zip(positions, weights) if weight >= threshold]
    if not selected:
        return None
    mn = [min(position[axis] for position, _ in selected) for axis in range(3)]
    mx = [max(position[axis] for position, _ in selected) for axis in range(3)]
    extent = [mx[i] - mn[i] for i in range(3)]
    weight_sum = sum(weight for _, weight in selected)
    centroid = [
        sum(position[axis] * weight for position, weight in selected) / weight_sum
        for axis in range(3)
    ] if weight_sum > 1e-12 else [0.0, 0.0, 0.0]
    return {
        "vertex_count": len(selected),
        "weight_sum": round(weight_sum, 6),
        "min": [round(v, 6) for v in mn],
        "max": [round(v, 6) for v in mx],
        "extent": [round(v, 6) for v in extent],
        "weighted_centroid": [round(v, 6) for v in centroid],
    }


def skin_influence_summary(doc: dict, binary_payload: bytes, target_node_index: int) -> list[dict[str, object]]:
    nodes = doc.get("nodes") or []
    skins = doc.get("skins") or []
    meshes = doc.get("meshes") or []
    materials = doc.get("materials") or []
    rows: list[dict[str, object]] = []

    for skin_index, skin in enumerate(skins):
        joints = skin.get("joints") or []
        slots = [slot for slot, joint_node in enumerate(joints) if joint_node == target_node_index]
        if not slots:
            continue
        if len(slots) != 1:
            fail(f"target node {target_node_index} appears multiple times in skin {skin_index}: {slots}")
        joint_slot = slots[0]
        mesh_nodes = [
            node_index for node_index, node in enumerate(nodes)
            if node.get("skin") == skin_index and isinstance(node.get("mesh"), int)
        ]
        skin_row: dict[str, object] = {
            "skin_index": skin_index,
            "skin_name": clean_name(skin.get("name"), f"<skin:{skin_index}>"),
            "joint_slot": joint_slot,
            "joint_count": len(joints),
            "skinned_meshes": [],
        }

        for node_index in mesh_nodes:
            node = nodes[node_index]
            mesh_index = int(node["mesh"])
            if mesh_index < 0 or mesh_index >= len(meshes):
                fail(f"skinned node {node_index} references invalid mesh {mesh_index}")
            mesh = meshes[mesh_index]
            primitive_rows: list[dict[str, object]] = []
            for primitive_index, primitive in enumerate(mesh.get("primitives") or []):
                attributes = primitive.get("attributes") or {}
                position_accessor = attributes.get("POSITION")
                if not isinstance(position_accessor, int):
                    fail(f"skinned mesh {mesh_index} primitive {primitive_index} lacks POSITION")
                positions = raw_accessor_values(doc, binary_payload, position_accessor)
                if not positions or len(positions[0]) != 3:
                    fail(f"skinned mesh {mesh_index} primitive {primitive_index} POSITION is not VEC3")
                vertex_weights = [0.0 for _ in positions]
                influence_sets = sorted(
                    suffix for key in attributes
                    if key.startswith("JOINTS_")
                    for suffix in [key.split("_", 1)[1]]
                )
                if not influence_sets:
                    continue
                for suffix in influence_sets:
                    joints_accessor = attributes.get(f"JOINTS_{suffix}")
                    weights_accessor = attributes.get(f"WEIGHTS_{suffix}")
                    if not isinstance(joints_accessor, int) or not isinstance(weights_accessor, int):
                        fail(f"skinned mesh {mesh_index} primitive {primitive_index} has unpaired JOINTS/WEIGHTS set {suffix}")
                    joint_values = raw_accessor_values(doc, binary_payload, joints_accessor)
                    weight_values = raw_accessor_values(doc, binary_payload, weights_accessor)
                    if len(joint_values) != len(positions) or len(weight_values) != len(positions):
                        fail(f"skinned mesh {mesh_index} primitive {primitive_index} influence count mismatch")
                    for vertex_index, (joint_tuple, weight_tuple) in enumerate(zip(joint_values, weight_values)):
                        if len(joint_tuple) != len(weight_tuple):
                            fail(f"skinned mesh {mesh_index} primitive {primitive_index} JOINTS/WEIGHTS width mismatch")
                        for joint_value, weight in zip(joint_tuple, weight_tuple):
                            if int(round(joint_value)) == joint_slot:
                                vertex_weights[vertex_index] += float(weight)

                influenced = [weight for weight in vertex_weights if weight > 1e-6]
                if not influenced:
                    continue
                full_bounds = accessor_bounds(doc, binary_payload, position_accessor)
                any_bounds = influence_bounds(positions, vertex_weights, 1e-6)
                dominant_bounds = influence_bounds(positions, vertex_weights, 0.5)
                if any_bounds is None:
                    fail("influence bookkeeping lost nonzero weighted vertices")
                full_min = [float(v) for v in full_bounds["min"]]
                full_extent = [float(v) for v in full_bounds["extent"]]
                centroid = [float(v) for v in any_bounds["weighted_centroid"]]
                normalized_centroid = [
                    round((centroid[i] - full_min[i]) / full_extent[i], 6) if abs(full_extent[i]) > 1e-12 else 0.0
                    for i in range(3)
                ]
                material_index = primitive.get("material")
                material_name = "NONE"
                if isinstance(material_index, int):
                    if material_index < 0 or material_index >= len(materials):
                        fail(f"skinned mesh {mesh_index} primitive {primitive_index} invalid material {material_index}")
                    material_name = clean_name(materials[material_index].get("name"), f"<material:{material_index}>")
                primitive_rows.append({
                    "primitive_index": primitive_index,
                    "material_index": material_index,
                    "material_name": material_name,
                    "full_position_bounds": full_bounds,
                    "influenced_vertex_count": len(influenced),
                    "max_vertex_weight": round(max(influenced), 6),
                    "mean_nonzero_vertex_weight": round(sum(influenced) / len(influenced), 6),
                    "influence_any_weight": any_bounds,
                    "influence_weight_ge_0_5": dominant_bounds,
                    "weighted_centroid_normalized_in_full_bounds": normalized_centroid,
                })

            if primitive_rows:
                skin_row["skinned_meshes"].append({
                    "node_index": node_index,
                    "node_name": clean_name(node.get("name"), f"<unnamed:{node_index}>"),
                    "mesh_index": mesh_index,
                    "mesh_name": clean_name(mesh.get("name"), f"<mesh:{mesh_index}>"),
                    "primitives": primitive_rows,
                })
        rows.append(skin_row)
    return rows


def target_summary(doc: dict, binary_payload: bytes, target_name: str) -> dict[str, object]:
    nodes = doc.get("nodes") or []
    if not isinstance(nodes, list):
        fail("GLB nodes collection missing")
    parents = build_parent_map(nodes)
    index = node_index_by_name(nodes, target_name)
    node = nodes[index]
    descendants = descendant_indices(nodes, index)
    subtree = [index, *descendants]
    child_rows = [
        {
            "node_index": child,
            "node_name": clean_name(nodes[child].get("name"), f"<unnamed:{child}>")
        }
        for child in node.get("children") or []
    ]
    row: dict[str, object] = {
        "node_index": index,
        "node_name": target_name,
        "parent": None,
        "children": child_rows,
        "ancestors_nearest_first": ancestor_rows(nodes, parents, index),
        "descendant_count": len(descendants),
        "descendant_names": [clean_name(nodes[i].get("name"), f"<unnamed:{i}>") for i in descendants],
        "local_translation": [round(float(v), 6) for v in (node.get("translation") or [0.0, 0.0, 0.0])],
        "local_rotation": [round(float(v), 6) for v in (node.get("rotation") or [0.0, 0.0, 0.0, 1.0])],
        "local_scale": [round(float(v), 6) for v in (node.get("scale") or [1.0, 1.0, 1.0])],
        "mesh_bindings_in_subtree": mesh_rows_for_subtree(doc, binary_payload, nodes, subtree),
        "skin_influence": skin_influence_summary(doc, binary_payload, index),
        "action_translation": translation_delta_summary(doc, binary_payload, index),
    }
    if index in parents:
        parent = parents[index]
        row["parent"] = {
            "node_index": parent,
            "node_name": clean_name(nodes[parent].get("name"), f"<unnamed:{parent}>")
        }
    fingerprint_payload = json.dumps(row, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")
    row["structure_fingerprint_sha256"] = hashlib.sha256(fingerprint_payload).hexdigest()
    return row


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    data = remote.fetch_bytes()
    identity = remote.verify_pinned_bytes(data)
    if identity.get("sha256") != EXPECTED_SHA256:
        fail(f"unexpected pinned SHA-256 {identity.get('sha256')}")
    doc = remote.parse_glb_json(data)
    remote.require_animation_contract(doc)
    remote.require_skin(doc)
    binary_payload = acquire.glb_binary_chunk(data)

    targets = {name: target_summary(doc, binary_payload, name) for name in TARGET_NODES}
    report = {
        "schema": 2,
        "audit": "PASS45_REMINGTON870_STRUCTURE_AUDIT",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "source_git_blob_sha1": identity["git_blob_sha1"],
        "source_transport_repo": remote.REPO,
        "source_transport_commit": remote.COMMIT,
        "source_transport_path": remote.PATH,
        "status": "STRUCTURAL_GEOMETRY_SKIN_EVIDENCE_ONLY",
        "targets": targets,
        "pump_node_identity": "UNPROVEN",
        "standalone_pump_clip": "UNPROVEN",
        "ue58_import_pending": True,
        "visual_inspection_required": True,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(
        "PASS45_REMINGTON870_STRUCTURE_AUDIT: PASS "
        f"source_sha256={identity['sha256']} targets={len(targets)} skin_influence=1 "
        "pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN "
        "ue58_import_pending=1 visual_inspection_required=1 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
