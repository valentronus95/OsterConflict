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
        values = acquire.accessor_values(doc, binary_payload, accessor_index)
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
        "schema": 1,
        "audit": "PASS45_REMINGTON870_STRUCTURE_AUDIT",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "source_git_blob_sha1": identity["git_blob_sha1"],
        "source_transport_repo": remote.REPO,
        "source_transport_commit": remote.COMMIT,
        "source_transport_path": remote.PATH,
        "status": "STRUCTURAL_GEOMETRY_EVIDENCE_ONLY",
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
        f"source_sha256={identity['sha256']} targets={len(targets)} "
        "pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN "
        "ue58_import_pending=1 visual_inspection_required=1 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
