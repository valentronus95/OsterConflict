#!/usr/bin/env python3
"""Partition exact-donor Pmag_061 connected components into spatial groups.

This follows the exact-donor topology audit. It does not name either group as the
physical fore-end. It detects the strongest centroid separation deterministically
so the next UE/source visual-isolation proof can inspect one bounded group instead
of treating all Pmag_061-weighted geometry as one pump object.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT as component
import PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT as visual
import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire
import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
TARGET = "Pmag_061"
MIN_COMPONENTS_PER_SIDE = 4
MIN_SEPARATION = 0.25


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 PMAG SPATIAL PARTITION AUDIT: FAIL\n[FAIL] {message}")


def group_bounds(rows: list[dict]) -> dict[str, list[float] | int]:
    mins = [min(float(r["world_bounds"]["min"][axis]) for r in rows) for axis in range(3)]
    maxs = [max(float(r["world_bounds"]["max"][axis]) for r in rows) for axis in range(3)]
    return {
        "component_count": len(rows),
        "vertex_count_sum": sum(int(r["vertex_count"]) for r in rows),
        "min": [round(v, 6) for v in mins],
        "max": [round(v, 6) for v in maxs],
        "extent": [round(maxs[i] - mins[i], 6) for i in range(3)],
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    data = remote.fetch_bytes()
    identity = remote.verify_pinned_bytes(data)
    if identity.get("sha256") != EXPECTED_SHA256:
        fail(f"unexpected source SHA-256 {identity.get('sha256')}")

    doc = remote.parse_glb_json(data)
    remote.require_animation_contract(doc)
    remote.require_skin(doc)
    binary_payload = acquire.glb_binary_chunk(data)
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []
    target_index = structure.node_index_by_name(nodes, TARGET)
    globals_ = visual.global_matrices(nodes)
    context_points, _context_sources = visual.context_world_points(doc, binary_payload, globals_)
    context_bounds = visual.bounds(context_points)
    world_points, _target_sources = visual.target_world_points(doc, binary_payload, globals_, TARGET)

    component._DATA = data
    _skin_index, mesh_node_index, _primitive_index, primitive = component.target_primitive(doc, target_index)
    mesh_index = int(nodes[mesh_node_index]["mesh"])
    if mesh_index < 0 or mesh_index >= len(meshes):
        fail(f"invalid target mesh index {mesh_index}")
    position_accessor = (primitive.get("attributes") or {}).get("POSITION")
    if not isinstance(position_accessor, int):
        fail("target primitive missing POSITION accessor")
    local_points = structure.raw_accessor_values(doc, binary_payload, position_accessor)
    if len(local_points) != len(world_points):
        fail(f"target vertex mismatch local={len(local_points)} world={len(world_points)}")
    indices = component.index_values(doc, binary_payload, primitive, len(local_points))
    rows, _members = component.component_rows(world_points, local_points, indices, context_bounds)
    if len(rows) < MIN_COMPONENTS_PER_SIDE * 2:
        fail(f"insufficient component count for spatial partition: {len(rows)}")

    ordered = sorted(rows, key=lambda row: float(row["world_bounds"]["centroid"][1]))
    candidates: list[tuple[float, int]] = []
    for split in range(MIN_COMPONENTS_PER_SIDE, len(ordered) - MIN_COMPONENTS_PER_SIDE + 1):
        left_y = float(ordered[split - 1]["world_bounds"]["centroid"][1])
        right_y = float(ordered[split]["world_bounds"]["centroid"][1])
        candidates.append((right_y - left_y, split))
    gap, split = max(candidates, key=lambda item: item[0])
    if gap < MIN_SEPARATION:
        fail(f"no stable spatial split: largest centroid gap={gap:.6f}")

    low_y = ordered[:split]
    high_y = ordered[split:]
    low_edge = float(low_y[-1]["world_bounds"]["centroid"][1])
    high_edge = float(high_y[0]["world_bounds"]["centroid"][1])

    report = {
        "schema": 1,
        "audit": "PASS45_REMINGTON870_PMAG_SPATIAL_PARTITION_AUDIT",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "target_node": TARGET,
        "input_component_count": len(rows),
        "partition_axis": "Y",
        "largest_centroid_gap": round(gap, 6),
        "split_low_edge": round(low_edge, 6),
        "split_high_edge": round(high_edge, 6),
        "low_y_group": {
            **group_bounds(low_y),
            "component_indices": [int(r["component_index"]) for r in low_y],
        },
        "high_y_group": {
            **group_bounds(high_y),
            "component_indices": [int(r["component_index"]) for r in high_y],
        },
        "status": "SPATIAL_PARTITION_EVIDENCE_ONLY",
        "interpretation": "Pmag_061 contains at least two strongly separated spatial component groups; physical fore-end identity still requires direct semantic/visual isolation proof",
        "direct_pmag_as_pump_mapping": "REJECTED",
        "fore_end_group_identity": "UNPROVEN",
        "standalone_pump_clip": "UNPROVEN",
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(
        "PASS45_REMINGTON870_PMAG_SPATIAL_PARTITION_AUDIT: PASS "
        f"components={len(rows)} gap={gap:.6f} low={len(low_y)} high={len(high_y)} "
        "direct_pmag_as_pump_mapping=REJECTED fore_end_group_identity=UNPROVEN "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
