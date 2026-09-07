#!/usr/bin/env python3
"""Generate bind-pose visual evidence for the exact pinned Remington 870 donor.

This is a semantic inspection aid, not an acceptance oracle. It reuses the pinned
remote donor identity plus the existing GLB/accessor audit helpers, reconstructs
bind-pose world-space geometry, highlights vertices materially influenced by
PBody_058 and Pmag_061, and emits one deterministic SVG + JSON report.

The script must not rename Pmag_061 to "pump", create production assets, or close
PASS45 item 16. A visual reviewer still has to identify the physical fore-end.
"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import math
import struct
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire
import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
TARGETS = ("PBody_058", "Pmag_061")
TARGET_WEIGHT_THRESHOLD = 0.5
CONTEXT_MATERIAL = "R870"
MAX_CONTEXT_POINTS = 5500
MAX_TARGET_POINTS = 3000


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 PUMP IDENTITY VISUAL AUDIT: FAIL\n[FAIL] {message}")


def mat_identity() -> list[list[float]]:
    return [
        [1.0, 0.0, 0.0, 0.0],
        [0.0, 1.0, 0.0, 0.0],
        [0.0, 0.0, 1.0, 0.0],
        [0.0, 0.0, 0.0, 1.0],
    ]


def mat_mul(a: list[list[float]], b: list[list[float]]) -> list[list[float]]:
    return [
        [sum(a[row][k] * b[k][col] for k in range(4)) for col in range(4)]
        for row in range(4)
    ]


def mat_vec(matrix: list[list[float]], value: tuple[float, float, float]) -> tuple[float, float, float]:
    x, y, z = value
    result = [
        matrix[row][0] * x
        + matrix[row][1] * y
        + matrix[row][2] * z
        + matrix[row][3]
        for row in range(4)
    ]
    w = result[3]
    if abs(w) > 1e-12 and abs(w - 1.0) > 1e-12:
        return (result[0] / w, result[1] / w, result[2] / w)
    return (result[0], result[1], result[2])


def local_matrix(node: dict) -> list[list[float]]:
    matrix = node.get("matrix")
    if isinstance(matrix, list):
        if len(matrix) != 16:
            fail(f"node matrix has {len(matrix)} values, expected 16")
        # glTF stores matrices column-major.
        return [[float(matrix[col * 4 + row]) for col in range(4)] for row in range(4)]

    tx, ty, tz = (float(v) for v in (node.get("translation") or [0.0, 0.0, 0.0]))
    sx, sy, sz = (float(v) for v in (node.get("scale") or [1.0, 1.0, 1.0]))
    x, y, z, w = (float(v) for v in (node.get("rotation") or [0.0, 0.0, 0.0, 1.0]))
    norm = math.sqrt(x * x + y * y + z * z + w * w)
    if norm <= 1e-12:
        fail("node quaternion has zero length")
    x, y, z, w = x / norm, y / norm, z / norm, w / norm

    rotation = [
        [1.0 - 2.0 * (y * y + z * z), 2.0 * (x * y - z * w), 2.0 * (x * z + y * w), 0.0],
        [2.0 * (x * y + z * w), 1.0 - 2.0 * (x * x + z * z), 2.0 * (y * z - x * w), 0.0],
        [2.0 * (x * z - y * w), 2.0 * (y * z + x * w), 1.0 - 2.0 * (x * x + y * y), 0.0],
        [0.0, 0.0, 0.0, 1.0],
    ]
    scale = [
        [sx, 0.0, 0.0, 0.0],
        [0.0, sy, 0.0, 0.0],
        [0.0, 0.0, sz, 0.0],
        [0.0, 0.0, 0.0, 1.0],
    ]
    translate = mat_identity()
    translate[0][3], translate[1][3], translate[2][3] = tx, ty, tz
    return mat_mul(translate, mat_mul(rotation, scale))


def global_matrices(nodes: list[dict]) -> list[list[list[float]]]:
    parents = structure.build_parent_map(nodes)
    cache: dict[int, list[list[float]]] = {}

    def resolve(index: int) -> list[list[float]]:
        if index in cache:
            return cache[index]
        local = local_matrix(nodes[index])
        if index in parents:
            value = mat_mul(resolve(parents[index]), local)
        else:
            value = local
        cache[index] = value
        return value

    return [resolve(index) for index in range(len(nodes))]


def raw_mat4_accessor(doc: dict, binary_payload: bytes, accessor_index: int) -> list[list[list[float]]]:
    accessors = doc.get("accessors") or []
    views = doc.get("bufferViews") or []
    if accessor_index < 0 or accessor_index >= len(accessors):
        fail(f"invalid MAT4 accessor {accessor_index}")
    accessor = accessors[accessor_index]
    if accessor.get("sparse"):
        fail(f"sparse MAT4 accessor {accessor_index} unsupported")
    if accessor.get("componentType") != 5126 or accessor.get("type") != "MAT4":
        fail(f"accessor {accessor_index} is not FLOAT MAT4")
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or view_index < 0 or view_index >= len(views):
        fail(f"MAT4 accessor {accessor_index} missing bufferView")
    view = views[view_index]
    count = accessor.get("count")
    if not isinstance(count, int) or count < 1:
        fail(f"MAT4 accessor {accessor_index} invalid count")
    packed_size = 64
    stride = int(view.get("byteStride", packed_size))
    if stride < packed_size:
        fail(f"MAT4 accessor {accessor_index} invalid stride {stride}")
    base = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    result: list[list[list[float]]] = []
    for item in range(count):
        offset = base + item * stride
        if offset < 0 or offset + packed_size > len(binary_payload):
            fail(f"MAT4 accessor {accessor_index} exceeds BIN chunk")
        values = struct.unpack_from("<16f", binary_payload, offset)
        result.append([[float(values[col * 4 + row]) for col in range(4)] for row in range(4)])
    return result


def skin_inverse_binds(doc: dict, binary_payload: bytes, skin: dict, joint_count: int) -> list[list[list[float]]]:
    accessor = skin.get("inverseBindMatrices")
    if accessor is None:
        return [mat_identity() for _ in range(joint_count)]
    if not isinstance(accessor, int):
        fail("inverseBindMatrices is not an accessor index")
    matrices = raw_mat4_accessor(doc, binary_payload, accessor)
    if len(matrices) != joint_count:
        fail(f"inverse bind count {len(matrices)} != joint count {joint_count}")
    return matrices


def material_name(doc: dict, primitive: dict) -> str:
    materials = doc.get("materials") or []
    index = primitive.get("material")
    if not isinstance(index, int):
        return "NONE"
    if index < 0 or index >= len(materials):
        fail(f"invalid material index {index}")
    return structure.clean_name(materials[index].get("name"), f"<material:{index}>")


def target_world_points(
    doc: dict,
    binary_payload: bytes,
    globals_: list[list[list[float]]],
    target_name: str,
) -> tuple[list[tuple[float, float, float]], list[dict[str, object]]]:
    nodes = doc.get("nodes") or []
    skins = doc.get("skins") or []
    meshes = doc.get("meshes") or []
    target_index = structure.node_index_by_name(nodes, target_name)
    points: list[tuple[float, float, float]] = []
    sources: list[dict[str, object]] = []

    for skin_index, skin in enumerate(skins):
        joints = skin.get("joints") or []
        target_slots = [slot for slot, joint_node in enumerate(joints) if joint_node == target_index]
        if not target_slots:
            continue
        if len(target_slots) != 1:
            fail(f"{target_name} appears multiple times in skin {skin_index}")
        target_slot = target_slots[0]
        inverse_binds = skin_inverse_binds(doc, binary_payload, skin, len(joints))
        joint_world = [globals_[joint_node] for joint_node in joints]
        skin_matrices = [
            mat_mul(joint_world[slot], inverse_binds[slot])
            for slot in range(len(joints))
        ]

        for mesh_node_index, node in enumerate(nodes):
            if node.get("skin") != skin_index or not isinstance(node.get("mesh"), int):
                continue
            mesh_index = int(node["mesh"])
            if mesh_index < 0 or mesh_index >= len(meshes):
                fail(f"node {mesh_node_index} references invalid mesh {mesh_index}")
            mesh = meshes[mesh_index]
            for primitive_index, primitive in enumerate(mesh.get("primitives") or []):
                attributes = primitive.get("attributes") or {}
                position_accessor = attributes.get("POSITION")
                if not isinstance(position_accessor, int):
                    continue
                positions = structure.raw_accessor_values(doc, binary_payload, position_accessor)
                influence_sets = sorted(
                    key.split("_", 1)[1]
                    for key in attributes
                    if key.startswith("JOINTS_")
                )
                if not influence_sets:
                    continue

                joint_sets: list[list[tuple[float, ...]]] = []
                weight_sets: list[list[tuple[float, ...]]] = []
                for suffix in influence_sets:
                    joints_accessor = attributes.get(f"JOINTS_{suffix}")
                    weights_accessor = attributes.get(f"WEIGHTS_{suffix}")
                    if not isinstance(joints_accessor, int) or not isinstance(weights_accessor, int):
                        fail(f"mesh {mesh_index} primitive {primitive_index} has unpaired influence set {suffix}")
                    joint_values = structure.raw_accessor_values(doc, binary_payload, joints_accessor)
                    weight_values = structure.raw_accessor_values(doc, binary_payload, weights_accessor)
                    if len(joint_values) != len(positions) or len(weight_values) != len(positions):
                        fail(f"mesh {mesh_index} primitive {primitive_index} influence count mismatch")
                    joint_sets.append(joint_values)
                    weight_sets.append(weight_values)

                selected_count = 0
                for vertex_index, raw_position in enumerate(positions):
                    if len(raw_position) != 3:
                        fail("POSITION accessor is not VEC3")
                    target_weight = 0.0
                    influences: list[tuple[int, float]] = []
                    for joint_values, weight_values in zip(joint_sets, weight_sets):
                        joint_tuple = joint_values[vertex_index]
                        weight_tuple = weight_values[vertex_index]
                        if len(joint_tuple) != len(weight_tuple):
                            fail("JOINTS/WEIGHTS width mismatch")
                        for raw_joint, raw_weight in zip(joint_tuple, weight_tuple):
                            slot = int(round(raw_joint))
                            weight = float(raw_weight)
                            if weight <= 1e-9:
                                continue
                            if slot < 0 or slot >= len(skin_matrices):
                                fail(f"joint slot {slot} out of range for skin {skin_index}")
                            influences.append((slot, weight))
                            if slot == target_slot:
                                target_weight += weight
                    if target_weight < TARGET_WEIGHT_THRESHOLD:
                        continue

                    total_weight = sum(weight for _, weight in influences)
                    if total_weight <= 1e-12:
                        fail("selected vertex has no nonzero skin weight")
                    world = [0.0, 0.0, 0.0]
                    for slot, weight in influences:
                        transformed = mat_vec(
                            skin_matrices[slot],
                            (float(raw_position[0]), float(raw_position[1]), float(raw_position[2])),
                        )
                        for axis in range(3):
                            world[axis] += transformed[axis] * weight
                    if abs(total_weight - 1.0) > 1e-5:
                        world = [value / total_weight for value in world]
                    points.append((world[0], world[1], world[2]))
                    selected_count += 1

                if selected_count:
                    sources.append(
                        {
                            "skin_index": skin_index,
                            "joint_slot": target_slot,
                            "mesh_node_index": mesh_node_index,
                            "mesh_node_name": structure.clean_name(
                                node.get("name"), f"<node:{mesh_node_index}>"
                            ),
                            "mesh_index": mesh_index,
                            "mesh_name": structure.clean_name(mesh.get("name"), f"<mesh:{mesh_index}>"),
                            "primitive_index": primitive_index,
                            "material_name": material_name(doc, primitive),
                            "selected_vertex_count": selected_count,
                        }
                    )

    if not points:
        fail(f"no bind-pose vertices selected for {target_name}")
    return points, sources


def context_world_points(
    doc: dict,
    binary_payload: bytes,
    globals_: list[list[list[float]]],
) -> tuple[list[tuple[float, float, float]], list[dict[str, object]]]:
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []
    points: list[tuple[float, float, float]] = []
    sources: list[dict[str, object]] = []
    for node_index, node in enumerate(nodes):
        mesh_index = node.get("mesh")
        if not isinstance(mesh_index, int) or node.get("skin") is not None:
            continue
        if mesh_index < 0 or mesh_index >= len(meshes):
            fail(f"context node {node_index} references invalid mesh {mesh_index}")
        mesh = meshes[mesh_index]
        for primitive_index, primitive in enumerate(mesh.get("primitives") or []):
            if material_name(doc, primitive) != CONTEXT_MATERIAL:
                continue
            position_accessor = (primitive.get("attributes") or {}).get("POSITION")
            if not isinstance(position_accessor, int):
                fail("R870 context primitive has no POSITION")
            raw_positions = structure.raw_accessor_values(doc, binary_payload, position_accessor)
            for raw_position in raw_positions:
                if len(raw_position) != 3:
                    fail("R870 context POSITION is not VEC3")
                points.append(
                    mat_vec(
                        globals_[node_index],
                        (float(raw_position[0]), float(raw_position[1]), float(raw_position[2])),
                    )
                )
            sources.append(
                {
                    "node_index": node_index,
                    "node_name": structure.clean_name(node.get("name"), f"<node:{node_index}>"),
                    "mesh_index": mesh_index,
                    "mesh_name": structure.clean_name(mesh.get("name"), f"<mesh:{mesh_index}>"),
                    "primitive_index": primitive_index,
                    "vertex_count": len(raw_positions),
                    "material_name": CONTEXT_MATERIAL,
                }
            )
    if not points:
        fail(f"no unskinned context geometry found with material {CONTEXT_MATERIAL}")
    return points, sources


def bounds(points: list[tuple[float, float, float]]) -> dict[str, object]:
    mn = [min(point[axis] for point in points) for axis in range(3)]
    mx = [max(point[axis] for point in points) for axis in range(3)]
    extent = [mx[axis] - mn[axis] for axis in range(3)]
    centroid = [sum(point[axis] for point in points) / len(points) for axis in range(3)]
    return {
        "min": [round(value, 6) for value in mn],
        "max": [round(value, 6) for value in mx],
        "extent": [round(value, 6) for value in extent],
        "centroid": [round(value, 6) for value in centroid],
        "vertex_count": len(points),
    }


def normalized_to_context(target_bounds: dict[str, object], context_bounds: dict[str, object]) -> dict[str, object]:
    context_min = [float(v) for v in context_bounds["min"]]
    context_extent = [float(v) for v in context_bounds["extent"]]
    result: dict[str, object] = {}
    for key in ("min", "max", "centroid"):
        values = [float(v) for v in target_bounds[key]]
        normalized = [
            (values[axis] - context_min[axis]) / context_extent[axis]
            if abs(context_extent[axis]) > 1e-12
            else 0.0
            for axis in range(3)
        ]
        result[key] = [round(value, 6) for value in normalized]
    return result


def sample_points(
    points: list[tuple[float, float, float]],
    maximum: int,
) -> list[tuple[float, float, float]]:
    if len(points) <= maximum:
        return points
    step = max(1, math.ceil(len(points) / maximum))
    sampled = points[::step]
    if points[-1] not in sampled:
        sampled.append(points[-1])
    return sampled[:maximum]


def escape(value: object) -> str:
    return html.escape(str(value), quote=True)


def svg_projection(
    context: list[tuple[float, float, float]],
    targets: dict[str, list[tuple[float, float, float]]],
    identity: dict[str, object],
    output: Path,
) -> None:
    width, height = 1560, 620
    margin = 55
    top = 155
    panel_gap = 30
    panel_width = (width - margin * 2 - panel_gap * 2) / 3
    panel_height = 390
    projections = (("X", "Y", 0, 1), ("X", "Z", 0, 2), ("Y", "Z", 1, 2))
    all_points = context + [point for values in targets.values() for point in values]
    context_sample = sample_points(context, MAX_CONTEXT_POINTS)
    target_samples = {name: sample_points(values, MAX_TARGET_POINTS) for name, values in targets.items()}
    palette = {"context": "#b4b4b4", "PBody_058": "#2456a6", "Pmag_061": "#d13b31"}

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        '<style>text{font-family:Arial,sans-serif;fill:#222}.small{font-size:14px}.title{font-size:22px;font-weight:700}.panel{font-size:17px;font-weight:700}</style>',
        '<text x="55" y="42" class="title">PASS45 Remington 870 bind-pose pump identity evidence</text>',
        f'<text x="55" y="70" class="small">exact donor SHA-256: {escape(identity["sha256"])}</text>',
        '<text x="55" y="94" class="small">Gray = authored R870 context; blue = PBody_058-influenced geometry; red = Pmag_061-influenced geometry.</text>',
        '<text x="55" y="118" class="small">Evidence aid only. Bone names are not semantic acceptance; physical fore-end identity still requires visual review.</text>',
        '<circle cx="1030" cy="70" r="5" fill="#b4b4b4"/><text x="1042" y="75" class="small">R870 context</text>',
        '<circle cx="1180" cy="70" r="5" fill="#2456a6"/><text x="1192" y="75" class="small">PBody_058</text>',
        '<circle cx="1325" cy="70" r="5" fill="#d13b31"/><text x="1337" y="75" class="small">Pmag_061</text>',
    ]

    for panel_index, (label_a, label_b, axis_a, axis_b) in enumerate(projections):
        x0 = margin + panel_index * (panel_width + panel_gap)
        y0 = top
        panel_points = [(p[axis_a], p[axis_b]) for p in all_points]
        min_a = min(p[0] for p in panel_points)
        max_a = max(p[0] for p in panel_points)
        min_b = min(p[1] for p in panel_points)
        max_b = max(p[1] for p in panel_points)
        range_a = max(max_a - min_a, 1e-9)
        range_b = max(max_b - min_b, 1e-9)
        scale = min((panel_width - 30) / range_a, (panel_height - 45) / range_b)
        used_w = range_a * scale
        used_h = range_b * scale
        offset_x = x0 + (panel_width - used_w) / 2
        offset_y = y0 + 25 + (panel_height - 45 - used_h) / 2

        def project(point: tuple[float, float, float]) -> tuple[float, float]:
            px = offset_x + (point[axis_a] - min_a) * scale
            py = offset_y + used_h - (point[axis_b] - min_b) * scale
            return px, py

        lines.append(
            f'<rect x="{x0:.2f}" y="{y0:.2f}" width="{panel_width:.2f}" height="{panel_height:.2f}" fill="none" stroke="#555" stroke-width="1"/>'
        )
        lines.append(
            f'<text x="{x0 + 12:.2f}" y="{y0 + 22:.2f}" class="panel">{label_a}{label_b} orthographic bind pose</text>'
        )
        for point in context_sample:
            px, py = project(point)
            lines.append(f'<circle cx="{px:.2f}" cy="{py:.2f}" r="0.65" fill="{palette["context"]}" opacity="0.28"/>')
        for target_name in TARGETS:
            for point in target_samples[target_name]:
                px, py = project(point)
                radius = "1.05" if target_name == "Pmag_061" else "0.9"
                opacity = "0.90" if target_name == "Pmag_061" else "0.72"
                lines.append(
                    f'<circle cx="{px:.2f}" cy="{py:.2f}" r="{radius}" fill="{palette[target_name]}" opacity="{opacity}"/>'
                )
        lines.append(
            f'<text x="{x0 + 10:.2f}" y="{y0 + panel_height - 9:.2f}" class="small">'
            f'{label_a} [{min_a:.4f}, {max_a:.4f}]  {label_b} [{min_b:.4f}, {max_b:.4f}]</text>'
        )

    lines.append('<text x="55" y="585" class="small">Status: VISUAL_SEMANTIC_REVIEW_REQUIRED / production_cutover=0 / runtime_acceptance=0 / item16_checked=0</text>')
    lines.append("</svg>")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-json", required=True)
    parser.add_argument("--output-svg", required=True)
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
    if not nodes:
        fail("GLB nodes collection missing")
    globals_ = global_matrices(nodes)

    context, context_sources = context_world_points(doc, binary_payload, globals_)
    target_points: dict[str, list[tuple[float, float, float]]] = {}
    target_sources: dict[str, list[dict[str, object]]] = {}
    for target_name in TARGETS:
        points, sources = target_world_points(doc, binary_payload, globals_, target_name)
        target_points[target_name] = points
        target_sources[target_name] = sources

    context_bounds = bounds(context)
    target_rows: dict[str, object] = {}
    for target_name in TARGETS:
        target_bounds = bounds(target_points[target_name])
        target_rows[target_name] = {
            "bind_pose_world_bounds": target_bounds,
            "normalized_to_r870_context": normalized_to_context(target_bounds, context_bounds),
            "sources": target_sources[target_name],
        }

    report = {
        "schema": 1,
        "audit": "PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT",
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "source_git_blob_sha1": identity["git_blob_sha1"],
        "source_transport_repo": remote.REPO,
        "source_transport_commit": remote.COMMIT,
        "source_transport_path": remote.PATH,
        "projection_space": "glTF bind-pose world coordinates reconstructed from node globals and skin inverse-bind matrices",
        "context_material": CONTEXT_MATERIAL,
        "context_bounds": context_bounds,
        "context_sources": context_sources,
        "targets": target_rows,
        "status": "VISUAL_SEMANTIC_REVIEW_REQUIRED",
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
    svg_projection(context, target_points, identity, Path(args.output_svg))

    fingerprint = hashlib.sha256(serialized.encode("utf-8")).hexdigest()
    print(
        "PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT: PASS "
        f"source_sha256={identity['sha256']} context_vertices={len(context)} "
        f"pbody_vertices={len(target_points['PBody_058'])} "
        f"pmag_vertices={len(target_points['Pmag_061'])} "
        f"report_sha256={fingerprint} "
        "pump_node_identity=UNPROVEN visual_semantic_review_required=1 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
