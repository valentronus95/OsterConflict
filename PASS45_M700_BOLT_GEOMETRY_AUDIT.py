#!/usr/bin/env python3
"""Fail-closed geometry ownership audit for the Stein CC0 M700 bolt joints.

This is a narrow PASS45 item-16 gate layered on top of the existing source-motion
auditor. It does not create production assets or claim runtime acceptance.

Schema 4 answers one unresolved question from schema 3: whether BOLT_STOP is
actually a separate weighted geometry component or overlaps the geometry owned by
BOLT. It deliberately refuses to reinterpret a weighted BOLT_STOP joint as an
authored bolt-travel endpoint.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
from typing import Any

from PASS45_M700_SOURCE_MOTION_AUDIT import (
    EXPECTED_SHA256,
    EXPECTED_SIZE,
    EXPECTED_SOURCE,
    clean_name,
    fail,
    load_buffers,
    read_accessor,
)

WEIGHT_EPSILON = 1e-5
DOMINANT_EPSILON = 1e-8
TARGETS = ("BOLT", "BOLT_STOP")


def exact_node(node_names: list[str], name: str) -> int:
    matches = [index for index, value in enumerate(node_names) if value == name]
    if len(matches) != 1:
        fail(f"expected exactly one {name} node, got {matches}")
    return matches[0]


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
    stats["bounds_min"] = [
        min(float(stats["bounds_min"][axis]), xyz[axis]) for axis in range(3)
    ]
    stats["bounds_max"] = [
        max(float(stats["bounds_max"][axis]), xyz[axis]) for axis in range(3)
    ]


def finalize_stats(stats: dict[str, Any]) -> dict[str, Any]:
    count = int(stats["weighted_vertex_count"])
    total_weight = float(stats["total_weight"])
    bounds_min = stats["bounds_min"]
    bounds_max = stats["bounds_max"]
    extent = None
    volume = None
    if bounds_min is not None and bounds_max is not None:
        extent = [
            float(bounds_max[axis]) - float(bounds_min[axis]) for axis in range(3)
        ]
        volume = math.prod(extent)

    weighted_centroid = None
    if total_weight > WEIGHT_EPSILON:
        weighted_centroid = [
            float(value) / total_weight
            for value in stats["weighted_centroid_accumulator"]
        ]

    vertex_centroid = None
    if count > 0:
        vertex_centroid = [
            float(value) / count for value in stats["vertex_centroid_accumulator"]
        ]

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


def distance(a: list[float] | None, b: list[float] | None) -> float | None:
    if a is None or b is None:
        return None
    return math.sqrt(sum((float(a[i]) - float(b[i])) ** 2 for i in range(3)))


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
    materials = doc.get("materials") or []
    if not isinstance(nodes, list) or not isinstance(meshes, list) or not isinstance(skins, list):
        fail("inspection glTF has invalid nodes/meshes/skins payload")
    if not nodes or not meshes or not skins:
        fail("inspection glTF lacks required skeletal geometry")

    buffers = load_buffers(doc, gltf_path)
    node_names = [
        clean_name(node.get("name"), f"<node:{index}>")
        for index, node in enumerate(nodes)
    ]
    mesh_names = [
        clean_name(mesh.get("name"), f"<mesh:{index}>")
        for index, mesh in enumerate(meshes)
    ]
    material_names = [
        clean_name(material.get("name"), f"<material:{index}>")
        for index, material in enumerate(materials)
    ]

    target_nodes = {name: exact_node(node_names, name) for name in TARGETS}
    raw_stats = {name: empty_stats() for name in TARGETS}
    shared_vertices: set[str] = set()
    primitive_rows: list[dict[str, Any]] = []

    for node_index, node in enumerate(nodes):
        skin_index = node.get("skin")
        mesh_index = node.get("mesh")
        if not isinstance(skin_index, int) or not isinstance(mesh_index, int):
            continue
        if not (0 <= skin_index < len(skins) and 0 <= mesh_index < len(meshes)):
            fail(f"skinned node {node_index} references invalid skin/mesh")

        skin_joints = skins[skin_index].get("joints") or []
        slots = {
            target: skin_joints.index(target_node)
            if target_node in skin_joints
            else None
            for target, target_node in target_nodes.items()
        }
        if all(slot is None for slot in slots.values()):
            continue

        primitives = meshes[mesh_index].get("primitives") or []
        for primitive_index, primitive in enumerate(primitives):
            attrs = primitive.get("attributes") or {}
            pos_accessor = attrs.get("POSITION")
            if not isinstance(pos_accessor, int):
                fail(f"mesh {mesh_index} primitive {primitive_index} lacks POSITION")
            positions = read_accessor(doc, buffers, pos_accessor)

            influence_sets: list[
                tuple[list[tuple[float | int, ...]], list[tuple[float | int, ...]]]
            ] = []
            set_index = 0
            while f"JOINTS_{set_index}" in attrs or f"WEIGHTS_{set_index}" in attrs:
                joints_accessor = attrs.get(f"JOINTS_{set_index}")
                weights_accessor = attrs.get(f"WEIGHTS_{set_index}")
                if not isinstance(joints_accessor, int) or not isinstance(weights_accessor, int):
                    fail(
                        f"mesh {mesh_index} primitive {primitive_index} has incomplete "
                        f"skin influence set {set_index}"
                    )
                joints_rows = read_accessor(doc, buffers, joints_accessor)
                weights_rows = read_accessor(doc, buffers, weights_accessor)
                if len(joints_rows) != len(positions) or len(weights_rows) != len(positions):
                    fail(
                        f"mesh {mesh_index} primitive {primitive_index} skin accessor "
                        "count mismatch"
                    )
                influence_sets.append((joints_rows, weights_rows))
                set_index += 1
            if not influence_sets:
                fail(
                    f"skinned mesh {mesh_index} primitive {primitive_index} "
                    "lacks JOINTS/WEIGHTS"
                )

            material_index = primitive.get("material")
            material_name = (
                material_names[material_index]
                if isinstance(material_index, int) and 0 <= material_index < len(material_names)
                else None
            )
            primitive_target_counts = {target: 0 for target in TARGETS}
            primitive_shared_count = 0

            for vertex_index, position in enumerate(positions):
                if len(position) < 3:
                    fail("POSITION accessor is not VEC3")
                xyz = [float(position[0]), float(position[1]), float(position[2])]
                if not all(math.isfinite(value) for value in xyz):
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
                    target: (
                        slot_weights.get(int(slot), 0.0)
                        if slot is not None
                        else 0.0
                    )
                    for target, slot in slots.items()
                }
                positive_targets = [
                    target
                    for target, weight in target_weights.items()
                    if weight > WEIGHT_EPSILON
                ]
                if not positive_targets:
                    continue

                vertex_key = f"{node_index}:{mesh_index}:{primitive_index}:{vertex_index}"
                max_weight = max(slot_weights.values()) if slot_weights else 0.0

                if len(positive_targets) == len(TARGETS):
                    shared_vertices.add(vertex_key)
                    primitive_shared_count += 1

                for target in positive_targets:
                    weight = target_weights[target]
                    stats = raw_stats[target]
                    if vertex_key in stats["weighted_vertices"]:
                        fail(f"duplicate target vertex identity encountered: {vertex_key}")
                    stats["weighted_vertices"].add(vertex_key)
                    stats["weighted_vertex_count"] += 1
                    stats["total_weight"] += weight
                    primitive_target_counts[target] += 1
                    if abs(weight - max_weight) <= DOMINANT_EPSILON:
                        stats["dominant_vertex_count"] += 1
                    if weight >= 1.0 - WEIGHT_EPSILON:
                        stats["full_weight_vertex_count"] += 1
                    update_bounds(stats, xyz)
                    for axis in range(3):
                        stats["weighted_centroid_accumulator"][axis] += xyz[axis] * weight
                        stats["vertex_centroid_accumulator"][axis] += xyz[axis]

            if any(primitive_target_counts.values()) or primitive_shared_count:
                primitive_rows.append(
                    {
                        "node_index": node_index,
                        "node_name": node_names[node_index],
                        "skin_index": skin_index,
                        "mesh_index": mesh_index,
                        "mesh_name": mesh_names[mesh_index],
                        "primitive_index": primitive_index,
                        "material_index": material_index
                        if isinstance(material_index, int)
                        else None,
                        "material_name": material_name,
                        "weighted_vertex_count": primitive_target_counts,
                        "shared_target_vertex_count": primitive_shared_count,
                    }
                )

    finalized = {target: finalize_stats(raw_stats[target]) for target in TARGETS}
    bolt_count = int(finalized["BOLT"]["weighted_vertex_count"])
    stop_count = int(finalized["BOLT_STOP"]["weighted_vertex_count"])
    shared_count = len(shared_vertices)

    if bolt_count <= 0:
        fail("BOLT does not weight any inspected geometry")
    if stop_count <= 0:
        fail("BOLT_STOP no longer weights geometry; source/converter evidence drifted")

    bolt_volume = finalized["BOLT"]["aabb_volume"]
    stop_volume = finalized["BOLT_STOP"]["aabb_volume"]
    volume_ratio = (
        float(stop_volume) / float(bolt_volume)
        if isinstance(stop_volume, (int, float))
        and isinstance(bolt_volume, (int, float))
        and float(bolt_volume) > 0.0
        else None
    )
    vertex_ratio = float(stop_count) / float(bolt_count)
    weighted_centroid_distance = distance(
        finalized["BOLT"]["weighted_centroid"],
        finalized["BOLT_STOP"]["weighted_centroid"],
    )

    if shared_count == 0:
        classification = "DISTINCT_WEIGHTED_COMPONENTS_NO_SHARED_VERTICES"
    else:
        classification = "OVERLAPPING_WEIGHTED_COMPONENTS_REVIEW_REQUIRED"

    report = {
        "schema": 4,
        "status": "SOURCE_GEOMETRY_OWNERSHIP_EVIDENCE_ONLY",
        "source": EXPECTED_SOURCE,
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": digest,
        "source_size": len(payload),
        "inspection_converter": "Assimp -> glTF2 (CI inspection only)",
        "target_nodes": target_nodes,
        "joint_geometry": finalized,
        "shared_weighted_vertex_count": shared_count,
        "target_weighted_vertices_disjoint": shared_count == 0,
        "bolt_stop_to_bolt_weighted_vertex_ratio": vertex_ratio,
        "bolt_stop_to_bolt_aabb_volume_ratio": volume_ratio,
        "weighted_centroid_distance": weighted_centroid_distance,
        "primitive_material_ownership": primitive_rows,
        "classification": classification,
        "source_authored_stop_delta_safe_as_bolt_travel": False,
        "source_authored_stop_delta_rejection_reason": (
            "BOLT_STOP_IS_WEIGHTED_GEOMETRY_COMPONENT_NOT_PROVEN_TRAVEL_ENDPOINT"
        ),
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(
        "PASS45 M700 BOLT GEOMETRY AUDIT: COMPLETE "
        f"bolt_vertices={bolt_count} bolt_stop_vertices={stop_count} "
        f"shared_vertices={shared_count} vertex_ratio={vertex_ratio:.6f} "
        f"volume_ratio={volume_ratio if volume_ratio is not None else 'NA'} "
        f"classification={classification} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
