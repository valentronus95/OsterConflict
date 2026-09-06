#!/usr/bin/env python3
"""Topology audit for the exact pinned Remington 870 Pmag_061 geometry.

The preceding bind-pose visual audit showed that Pmag_061 influences visibly
separated geometry. This audit answers the next narrower question: how many
connected mesh components are actually carried by that one animated joint, and
where do those components sit relative to the authored R870 body?

It is evidence only. It does not call any component a pump, mutate Unreal
content, create production assets, or close PASS45 item 16.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
from pathlib import Path

import PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT as visual
import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire
import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
TARGET = "Pmag_061"
WEIGHT_THRESHOLD = 0.5
MAX_POINTS_PER_COMPONENT = 1800


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 PMAG COMPONENT AUDIT: FAIL\n[FAIL] {message}")


class UnionFind:
    def __init__(self, count: int) -> None:
        self.parent = list(range(count))
        self.rank = [0] * count

    def find(self, value: int) -> int:
        while self.parent[value] != value:
            self.parent[value] = self.parent[self.parent[value]]
            value = self.parent[value]
        return value

    def union(self, left: int, right: int) -> None:
        a = self.find(left)
        b = self.find(right)
        if a == b:
            return
        if self.rank[a] < self.rank[b]:
            a, b = b, a
        self.parent[b] = a
        if self.rank[a] == self.rank[b]:
            self.rank[a] += 1


def index_values(doc: dict, binary_payload: bytes, primitive: dict, vertex_count: int) -> list[int]:
    accessor = primitive.get("indices")
    if accessor is None:
        return list(range(vertex_count))
    if not isinstance(accessor, int):
        fail("primitive indices is not an accessor index")
    raw = structure.raw_accessor_values(doc, binary_payload, accessor)
    result: list[int] = []
    for row in raw:
        if len(row) != 1:
            fail("index accessor is not SCALAR")
        value = int(round(row[0]))
        if value < 0 or value >= vertex_count:
            fail(f"index {value} outside vertex range 0..{vertex_count - 1}")
        result.append(value)
    return result


def target_primitive(doc: dict, target_index: int) -> tuple[int, int, int, dict]:
    nodes = doc.get("nodes") or []
    skins = doc.get("skins") or []
    meshes = doc.get("meshes") or []
    candidates: list[tuple[int, int, int, dict]] = []
    for skin_index, skin in enumerate(skins):
        joints = skin.get("joints") or []
        if target_index not in joints:
            continue
        for node_index, node in enumerate(nodes):
            if node.get("skin") != skin_index or not isinstance(node.get("mesh"), int):
                continue
            mesh_index = int(node["mesh"])
            mesh = meshes[mesh_index]
            for primitive_index, primitive in enumerate(mesh.get("primitives") or []):
                attrs = primitive.get("attributes") or {}
                position_accessor = attrs.get("POSITION")
                if not isinstance(position_accessor, int):
                    continue
                positions = structure.raw_accessor_values(doc, acquire.glb_binary_chunk(_DATA), position_accessor)
                joint_slot = joints.index(target_index)
                target_weight_seen = False
                for suffix in sorted(key.split("_", 1)[1] for key in attrs if key.startswith("JOINTS_")):
                    ja = attrs.get(f"JOINTS_{suffix}")
                    wa = attrs.get(f"WEIGHTS_{suffix}")
                    if not isinstance(ja, int) or not isinstance(wa, int):
                        fail(f"unpaired JOINTS/WEIGHTS set {suffix}")
                    joint_rows = structure.raw_accessor_values(doc, acquire.glb_binary_chunk(_DATA), ja)
                    weight_rows = structure.raw_accessor_values(doc, acquire.glb_binary_chunk(_DATA), wa)
                    if len(joint_rows) != len(positions) or len(weight_rows) != len(positions):
                        fail("influence row count mismatch")
                    for joints_row, weights_row in zip(joint_rows, weight_rows):
                        if any(int(round(j)) == joint_slot and float(w) >= WEIGHT_THRESHOLD for j, w in zip(joints_row, weights_row)):
                            target_weight_seen = True
                            break
                    if target_weight_seen:
                        break
                if target_weight_seen:
                    candidates.append((skin_index, node_index, primitive_index, primitive))
    if len(candidates) != 1:
        fail(f"expected one materially {TARGET}-influenced primitive, found {len(candidates)}")
    return candidates[0]


def component_rows(
    world_points: list[tuple[float, float, float]],
    local_points: list[tuple[float, ...]],
    indices: list[int],
    context_bounds: dict[str, object],
) -> tuple[list[dict[str, object]], dict[int, list[int]]]:
    if len(indices) % 3 != 0:
        fail(f"triangle index count {len(indices)} is not divisible by 3")
    uf = UnionFind(len(world_points))
    used: set[int] = set()
    triangle_counts: dict[int, int] = {}
    for offset in range(0, len(indices), 3):
        a, b, c = indices[offset : offset + 3]
        used.update((a, b, c))
        uf.union(a, b)
        uf.union(b, c)
        uf.union(c, a)
    groups: dict[int, list[int]] = {}
    for vertex in sorted(used):
        root = uf.find(vertex)
        groups.setdefault(root, []).append(vertex)
    for offset in range(0, len(indices), 3):
        root = uf.find(indices[offset])
        triangle_counts[root] = triangle_counts.get(root, 0) + 1

    ordered_groups = sorted(groups.values(), key=lambda members: (-len(members), members[0]))
    rows: list[dict[str, object]] = []
    remapped: dict[int, list[int]] = {}
    for component_index, members in enumerate(ordered_groups):
        world = [world_points[i] for i in members]
        local = [tuple(float(v) for v in local_points[i]) for i in members]
        world_bounds = visual.bounds(world)
        local_bounds = visual.bounds(local)  # type: ignore[arg-type]
        root = uf.find(members[0])
        row = {
            "component_index": component_index,
            "vertex_count": len(members),
            "triangle_count": triangle_counts.get(root, 0),
            "world_bounds": world_bounds,
            "local_bounds": local_bounds,
            "normalized_to_r870_context": visual.normalized_to_context(world_bounds, context_bounds),
            "world_extent_sorted": sorted(
                (("X", float(world_bounds["extent"][0])), ("Y", float(world_bounds["extent"][1])), ("Z", float(world_bounds["extent"][2]))),
                key=lambda item: item[1],
                reverse=True,
            ),
        }
        rows.append(row)
        remapped[component_index] = members
    return rows, remapped


def sample_members(members: list[int]) -> list[int]:
    if len(members) <= MAX_POINTS_PER_COMPONENT:
        return members
    step = max(1, math.ceil(len(members) / MAX_POINTS_PER_COMPONENT))
    return members[::step][:MAX_POINTS_PER_COMPONENT]


def component_svg(
    world_points: list[tuple[float, float, float]],
    components: dict[int, list[int]],
    context: list[tuple[float, float, float]],
    identity: dict[str, object],
    output: Path,
) -> None:
    width, height = 1560, 650
    margin = 55
    top = 165
    gap = 28
    panel_w = (width - 2 * margin - 2 * gap) / 3
    panel_h = 390
    projections = (("X", "Y", 0, 1), ("X", "Z", 0, 2), ("Y", "Z", 1, 2))
    palette = (
        "#d73027", "#4575b4", "#1a9850", "#984ea3", "#ff7f00", "#a65628",
        "#e7298a", "#66a61e", "#7570b3", "#e6ab02", "#1b9e77", "#666666",
    )
    all_points = context + world_points
    context_sample = visual.sample_points(context, 4800)

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#fff"/>',
        '<style>text{font-family:Arial,sans-serif;fill:#222}.small{font-size:14px}.title{font-size:22px;font-weight:700}.panel{font-size:17px;font-weight:700}.label{font-size:13px;font-weight:700}</style>',
        '<text x="55" y="40" class="title">PASS45 Remington 870 Pmag_061 connected components</text>',
        f'<text x="55" y="68" class="small">exact donor SHA-256: {html.escape(str(identity["sha256"]))}</text>',
        '<text x="55" y="94" class="small">Gray = R870 context. Each Pmag_061 connected component gets a separate color/label.</text>',
        '<text x="55" y="118" class="small">Topology evidence only. A component is not called the pump without separate physical/semantic review.</text>',
    ]

    for panel_index, (la, lb, aa, ab) in enumerate(projections):
        x0 = margin + panel_index * (panel_w + gap)
        y0 = top
        mn_a = min(p[aa] for p in all_points)
        mx_a = max(p[aa] for p in all_points)
        mn_b = min(p[ab] for p in all_points)
        mx_b = max(p[ab] for p in all_points)
        range_a = max(mx_a - mn_a, 1e-9)
        range_b = max(mx_b - mn_b, 1e-9)
        scale = min((panel_w - 28) / range_a, (panel_h - 48) / range_b)
        used_w = range_a * scale
        used_h = range_b * scale
        ox = x0 + (panel_w - used_w) / 2
        oy = y0 + 25 + (panel_h - 48 - used_h) / 2

        def project(point: tuple[float, float, float]) -> tuple[float, float]:
            return (
                ox + (point[aa] - mn_a) * scale,
                oy + used_h - (point[ab] - mn_b) * scale,
            )

        lines.append(f'<rect x="{x0:.2f}" y="{y0:.2f}" width="{panel_w:.2f}" height="{panel_h:.2f}" fill="none" stroke="#555"/>')
        lines.append(f'<text x="{x0 + 12:.2f}" y="{y0 + 22:.2f}" class="panel">{la}{lb} bind pose</text>')
        for point in context_sample:
            px, py = project(point)
            lines.append(f'<circle cx="{px:.2f}" cy="{py:.2f}" r="0.6" fill="#aaa" opacity="0.22"/>')
        for component_index, members in sorted(components.items()):
            color = palette[component_index % len(palette)]
            sampled = sample_members(members)
            for vertex in sampled:
                px, py = project(world_points[vertex])
                lines.append(f'<circle cx="{px:.2f}" cy="{py:.2f}" r="1.0" fill="{color}" opacity="0.82"/>')
            centroid = tuple(
                sum(world_points[v][axis] for v in members) / len(members)
                for axis in range(3)
            )
            px, py = project(centroid)
            lines.append(f'<text x="{px + 4:.2f}" y="{py - 4:.2f}" class="label" fill="{color}">C{component_index}</text>')
        lines.append(f'<text x="{x0 + 10:.2f}" y="{y0 + panel_h - 9:.2f}" class="small">{la}[{mn_a:.3f},{mx_a:.3f}] {lb}[{mn_b:.3f},{mx_b:.3f}]</text>')

    lines.append('<text x="55" y="600" class="small">Status: COMPONENT_TOPOLOGY_EVIDENCE_ONLY / pump_node_identity=UNPROVEN / production_cutover=0 / item16_checked=0</text>')
    lines.append('</svg>')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-json", required=True)
    parser.add_argument("--output-svg", required=True)
    args = parser.parse_args()

    global _DATA
    _DATA = remote.fetch_bytes()
    identity = remote.verify_pinned_bytes(_DATA)
    if identity.get("sha256") != EXPECTED_SHA256:
        fail(f"unexpected source SHA-256 {identity.get('sha256')}")
    doc = remote.parse_glb_json(_DATA)
    remote.require_animation_contract(doc)
    remote.require_skin(doc)
    binary_payload = acquire.glb_binary_chunk(_DATA)
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []
    target_index = structure.node_index_by_name(nodes, TARGET)
    globals_ = visual.global_matrices(nodes)

    context, context_sources = visual.context_world_points(doc, binary_payload, globals_)
    context_bounds = visual.bounds(context)
    world_points, target_sources = visual.target_world_points(doc, binary_payload, globals_, TARGET)

    skin_index, mesh_node_index, primitive_index, primitive = target_primitive(doc, target_index)
    mesh_index = int(nodes[mesh_node_index]["mesh"])
    mesh = meshes[mesh_index]
    if primitive.get("mode", 4) != 4:
        fail(f"target primitive mode {primitive.get('mode')} is not TRIANGLES")
    position_accessor = (primitive.get("attributes") or {}).get("POSITION")
    if not isinstance(position_accessor, int):
        fail("target primitive missing POSITION accessor")
    local_points = structure.raw_accessor_values(doc, binary_payload, position_accessor)
    if len(local_points) != len(world_points):
        fail(
            f"selected target vertex count {len(world_points)} != primitive vertex count {len(local_points)}; "
            "component audit requires the pinned all-weight-one Pmag geometry"
        )
    indices = index_values(doc, binary_payload, primitive, len(local_points))
    rows, components = component_rows(world_points, local_points, indices, context_bounds)
    if len(rows) < 2:
        fail(f"expected visibly separated Pmag geometry, topology found only {len(rows)} component")

    report = {
        "schema": 1,
        "audit": "PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "source_git_blob_sha1": identity["git_blob_sha1"],
        "source_transport_repo": remote.REPO,
        "source_transport_commit": remote.COMMIT,
        "source_transport_path": remote.PATH,
        "target_node": TARGET,
        "skin_index": skin_index,
        "mesh_node_index": mesh_node_index,
        "mesh_node_name": structure.clean_name(nodes[mesh_node_index].get("name"), f"<node:{mesh_node_index}>"),
        "mesh_index": mesh_index,
        "mesh_name": structure.clean_name(mesh.get("name"), f"<mesh:{mesh_index}>"),
        "primitive_index": primitive_index,
        "material_name": visual.material_name(doc, primitive),
        "vertex_count": len(local_points),
        "index_count": len(indices),
        "triangle_count": len(indices) // 3,
        "component_count": len(rows),
        "components": rows,
        "context_bounds": context_bounds,
        "context_sources": context_sources,
        "target_sources": target_sources,
        "interpretation": "one animated Pmag_061 joint carries multiple disconnected authored geometry components; component semantics remain review-required",
        "status": "COMPONENT_TOPOLOGY_EVIDENCE_ONLY",
        "pump_node_identity": "UNPROVEN",
        "standalone_pump_clip": "UNPROVEN",
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    serialized = json.dumps(report, indent=2, sort_keys=True) + "\n"
    json_path = Path(args.output_json)
    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(serialized, encoding="utf-8")
    component_svg(world_points, components, context, identity, Path(args.output_svg))
    fingerprint = hashlib.sha256(serialized.encode("utf-8")).hexdigest()
    print(
        "PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT: PASS "
        f"source_sha256={identity['sha256']} components={len(rows)} vertices={len(local_points)} "
        f"triangles={len(indices)//3} report_sha256={fingerprint} "
        "pump_node_identity=UNPROVEN standalone_pump_clip=UNPROVEN "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


_DATA = b""

if __name__ == "__main__":
    main()
