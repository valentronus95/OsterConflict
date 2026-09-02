#!/usr/bin/env python3
"""Build a deterministic CC-BY-derived Remington 870 source with a clean pump joint.

The registered 8sianDude donor is articulated, but its Pmag_061 joint owns 106
separate connected components. Source/topology evidence shows that using that
joint directly as the production pump would also move unrelated side-saddle
geometry. This builder keeps the exact donor, reuses its geometry/materials,
and reassigns only the already-audited low-Y component partition to a new
PASS45_PumpForeEnd joint with the same bind transform as Pmag_061.

A standalone pump AnimSequence source channel is then authored on that new
joint. The stroke distance is not guessed from whole-weapon recoil: it reuses
the donor's measured easy-reload Pmag_061 dominant-Y travel magnitude
(0.537790 source units), while intentionally omitting the donor's unrelated
rotation/X/Z excursion.

This creates DERIVED SOURCE ONLY. It does not import/save UE assets, wire the
gameplay profile, claim visual/audio acceptance, close item 16, or merge PR #94.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import struct
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire
import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure

EXPECTED_SOURCE_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
TARGET_JOINT_NAME = "Pmag_061"
TARGET_MESH_NODE_NAME = "Object_95"
DERIVED_JOINT_NAME = "PASS45_PumpForeEnd"
DERIVED_ANIMATION_NAME = "PASS45_Remington870_PumpCycle"
DERIVED_SOURCE_NAME = "remington_870_pass45_derived_pump.glb"
DERIVED_MANIFEST_NAME = "PASS45_REMINGTON870_DERIVED_PUMP_MANIFEST.json"

# Exact component partition produced by PASS45_REMINGTON870_PMAG_SPATIAL_PARTITION_AUDIT.py
# for the pinned donor. It is topology-index based, so it remains deterministic as
# long as the pinned source SHA and component ordering contract remain unchanged.
LOW_Y_COMPONENTS = frozenset({
    8, 9, 17, 21, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35,
    43, 47, 51, 52, 53, 59, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77,
    78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93,
})
EXPECTED_COMPONENT_COUNT = 106
EXPECTED_LOW_COMPONENT_COUNT = 48
EXPECTED_HIGH_COMPONENT_COUNT = 58
EXPECTED_LOW_VERTEX_COUNT = 1170
EXPECTED_HIGH_VERTEX_COUNT = 3241
EXPECTED_TARGET_VERTEX_COUNT = 4411
EXPECTED_TARGET_TRIANGLES = 4982
PUMP_STROKE_Y = -0.537790
PUMP_DURATION = 0.55
PUMP_TIMES = (0.0, 0.18, 0.28, PUMP_DURATION)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 DERIVED PUMP SOURCE: FAIL\n[FAIL] {message}")


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


def parse_source(data: bytes) -> tuple[dict, bytearray]:
    identity = remote.verify_pinned_bytes(data)
    if identity.get("sha256") != EXPECTED_SOURCE_SHA256:
        fail(f"unexpected source SHA-256 {identity.get('sha256')}")
    doc = remote.parse_glb_json(data)
    remote.require_animation_contract(doc)
    remote.require_skin(doc)
    return copy.deepcopy(doc), bytearray(acquire.glb_binary_chunk(data))


def node_index(nodes: list[dict], name: str) -> int:
    matches = [i for i, node in enumerate(nodes) if str(node.get("name") or "") == name]
    if len(matches) != 1:
        fail(f"expected exactly one node named {name}, found {matches}")
    return matches[0]


def primitive_indices(doc: dict, binary_payload: bytes, primitive: dict, vertex_count: int) -> list[int]:
    accessor_index = primitive.get("indices")
    if accessor_index is None:
        values = list(range(vertex_count))
    elif isinstance(accessor_index, int):
        rows = structure.raw_accessor_values(doc, binary_payload, accessor_index)
        values = []
        for row in rows:
            if len(row) != 1:
                fail("index accessor is not SCALAR")
            values.append(int(round(row[0])))
    else:
        fail("primitive indices is not an accessor index")
    if len(values) % 3 != 0:
        fail(f"triangle index count {len(values)} is not divisible by 3")
    if any(value < 0 or value >= vertex_count for value in values):
        fail("primitive index outside vertex range")
    return values


def ordered_components(vertex_count: int, indices: list[int]) -> list[list[int]]:
    uf = UnionFind(vertex_count)
    used: set[int] = set()
    for offset in range(0, len(indices), 3):
        a, b, c = indices[offset: offset + 3]
        used.update((a, b, c))
        uf.union(a, b)
        uf.union(b, c)
        uf.union(c, a)
    groups: dict[int, list[int]] = {}
    for vertex in sorted(used):
        groups.setdefault(uf.find(vertex), []).append(vertex)
    return sorted(groups.values(), key=lambda members: (-len(members), members[0]))


def accessor_layout(doc: dict, accessor_index: int) -> tuple[dict, dict, int, int, int]:
    accessors = doc.get("accessors") or []
    views = doc.get("bufferViews") or []
    if accessor_index < 0 or accessor_index >= len(accessors):
        fail(f"invalid accessor {accessor_index}")
    accessor = accessors[accessor_index]
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or view_index < 0 or view_index >= len(views):
        fail(f"accessor {accessor_index} missing bufferView")
    view = views[view_index]
    if view.get("buffer", 0) != 0:
        fail(f"accessor {accessor_index} references nonzero buffer")
    component_type = accessor.get("componentType")
    component_sizes = {5120: 1, 5121: 1, 5122: 2, 5123: 2, 5125: 4, 5126: 4}
    component_size = component_sizes.get(component_type)
    type_counts = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}
    component_count = type_counts.get(accessor.get("type"))
    if component_size is None or component_count is None:
        fail(f"unsupported accessor layout for accessor {accessor_index}")
    packed_size = component_size * component_count
    stride = int(view.get("byteStride", packed_size))
    if stride < packed_size:
        fail(f"accessor {accessor_index} invalid stride {stride}")
    base = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    return accessor, view, base, stride, component_size


def write_joint_component(doc: dict, binary_payload: bytearray, accessor_index: int,
                          vertex: int, component: int, value: int) -> None:
    accessor, _view, base, stride, component_size = accessor_layout(doc, accessor_index)
    if accessor.get("type") != "VEC4":
        fail(f"joint accessor {accessor_index} is not VEC4")
    component_type = accessor.get("componentType")
    formats = {5121: ("B", 255), 5123: ("H", 65535)}
    layout = formats.get(component_type)
    if layout is None:
        fail(f"joint accessor {accessor_index} unsupported componentType {component_type}")
    fmt, maximum = layout
    if value < 0 or value > maximum:
        fail(f"joint slot {value} does not fit accessor componentType {component_type}")
    count = int(accessor.get("count", 0))
    if vertex < 0 or vertex >= count or component < 0 or component >= 4:
        fail("joint accessor write outside bounds")
    offset = base + vertex * stride + component * component_size
    struct.pack_into("<" + fmt, binary_payload, offset, value)


def append_aligned(binary_payload: bytearray, payload: bytes) -> tuple[int, int]:
    while len(binary_payload) % 4:
        binary_payload.append(0)
    offset = len(binary_payload)
    binary_payload.extend(payload)
    return offset, len(payload)


def append_accessor(doc: dict, binary_payload: bytearray, payload: bytes,
                    component_type: int, accessor_type: str, count: int,
                    *, minimum: list[float] | None = None,
                    maximum: list[float] | None = None) -> int:
    offset, length = append_aligned(binary_payload, payload)
    views = doc.setdefault("bufferViews", [])
    accessors = doc.setdefault("accessors", [])
    view_index = len(views)
    views.append({"buffer": 0, "byteOffset": offset, "byteLength": length})
    accessor: dict[str, object] = {
        "bufferView": view_index,
        "componentType": component_type,
        "count": count,
        "type": accessor_type,
    }
    if minimum is not None:
        accessor["min"] = minimum
    if maximum is not None:
        accessor["max"] = maximum
    accessor_index = len(accessors)
    accessors.append(accessor)
    return accessor_index


def read_inverse_bind_matrices(doc: dict, binary_payload: bytes, skin: dict) -> list[tuple[float, ...]]:
    accessor_index = skin.get("inverseBindMatrices")
    if not isinstance(accessor_index, int):
        fail("target skin lacks inverseBindMatrices accessor")
    accessor, _view, base, stride, _component_size = accessor_layout(doc, accessor_index)
    if accessor.get("componentType") != 5126 or accessor.get("type") != "MAT4":
        fail("target inverseBindMatrices accessor is not FLOAT MAT4")
    count = int(accessor.get("count", 0))
    matrices: list[tuple[float, ...]] = []
    for item in range(count):
        offset = base + item * stride
        if offset < 0 or offset + 64 > len(binary_payload):
            fail("inverseBindMatrices accessor exceeds BIN chunk")
        matrices.append(tuple(float(v) for v in struct.unpack_from("<16f", binary_payload, offset)))
    return matrices


def parent_map(nodes: list[dict]) -> dict[int, int]:
    result: dict[int, int] = {}
    for parent, node in enumerate(nodes):
        for child in node.get("children") or []:
            if not isinstance(child, int):
                fail(f"node {parent} has non-integer child")
            if child in result:
                fail(f"node {child} has multiple parents")
            result[child] = parent
    return result


def build_derived(data: bytes) -> tuple[bytes, dict[str, object]]:
    doc, binary_payload = parse_source(data)
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []
    skins = doc.get("skins") or []

    old_joint_node = node_index(nodes, TARGET_JOINT_NAME)
    mesh_node_index = node_index(nodes, TARGET_MESH_NODE_NAME)
    mesh_node = nodes[mesh_node_index]
    mesh_index = mesh_node.get("mesh")
    skin_index = mesh_node.get("skin")
    if not isinstance(mesh_index, int) or not isinstance(skin_index, int):
        fail("Object_95 is not the expected skinned mesh node")
    if mesh_index < 0 or mesh_index >= len(meshes) or skin_index < 0 or skin_index >= len(skins):
        fail("Object_95 mesh/skin index outside document")
    mesh = meshes[mesh_index]
    primitives = mesh.get("primitives") or []
    if len(primitives) != 1:
        fail(f"expected one Object_95 primitive, found {len(primitives)}")
    primitive = primitives[0]
    attrs = primitive.get("attributes") or {}
    position_accessor = attrs.get("POSITION")
    if not isinstance(position_accessor, int):
        fail("Object_95 primitive missing POSITION")
    vertex_count = int((doc.get("accessors") or [])[position_accessor].get("count", 0))
    if vertex_count != EXPECTED_TARGET_VERTEX_COUNT:
        fail(f"Pmag target vertex count drifted: {vertex_count}")

    indices = primitive_indices(doc, bytes(binary_payload), primitive, vertex_count)
    if len(indices) // 3 != EXPECTED_TARGET_TRIANGLES:
        fail(f"Pmag triangle count drifted: {len(indices) // 3}")
    components = ordered_components(vertex_count, indices)
    if len(components) != EXPECTED_COMPONENT_COUNT:
        fail(f"Pmag component count drifted: {len(components)}")
    if max(LOW_Y_COMPONENTS) >= len(components) or len(LOW_Y_COMPONENTS) != EXPECTED_LOW_COMPONENT_COUNT:
        fail("low-Y component partition contract drifted")
    low_vertices = sorted({v for c in LOW_Y_COMPONENTS for v in components[c]})
    high_vertices = sorted(set(range(vertex_count)) - set(low_vertices))
    if len(low_vertices) != EXPECTED_LOW_VERTEX_COUNT or len(high_vertices) != EXPECTED_HIGH_VERTEX_COUNT:
        fail(
            f"partition vertex counts drifted low={len(low_vertices)} high={len(high_vertices)}"
        )

    skin = skins[skin_index]
    joints = skin.get("joints") or []
    old_slots = [slot for slot, joint_node in enumerate(joints) if joint_node == old_joint_node]
    if len(old_slots) != 1:
        fail(f"Pmag joint slot not unique in target skin: {old_slots}")
    old_slot = old_slots[0]

    # Duplicate the Pmag bind transform into a sibling joint. This preserves the bind pose
    # exactly while giving the fore-end-only vertex partition an independent animation target.
    old_node = nodes[old_joint_node]
    new_node: dict[str, object] = {"name": DERIVED_JOINT_NAME}
    for key in ("translation", "rotation", "scale", "matrix"):
        if key in old_node:
            new_node[key] = copy.deepcopy(old_node[key])
    new_joint_node = len(nodes)
    nodes.append(new_node)
    parents = parent_map(nodes[:-1])
    if old_joint_node not in parents:
        fail("Pmag joint has no parent; cannot create bind-equivalent sibling")
    parent = parents[old_joint_node]
    parent_children = nodes[parent].setdefault("children", [])
    if not isinstance(parent_children, list):
        fail("Pmag parent children is not a list")
    parent_children.append(new_joint_node)

    matrices = read_inverse_bind_matrices(doc, bytes(binary_payload), skin)
    if len(matrices) != len(joints):
        fail(f"inverse bind matrix count {len(matrices)} != joint count {len(joints)}")
    matrices.append(matrices[old_slot])
    matrix_payload = b"".join(struct.pack("<16f", *matrix) for matrix in matrices)
    new_ibm_accessor = append_accessor(
        doc, binary_payload, matrix_payload, 5126, "MAT4", len(matrices)
    )
    skin["inverseBindMatrices"] = new_ibm_accessor
    new_slot = len(joints)
    joints.append(new_joint_node)
    skin["joints"] = joints

    # Find the JOINTS/WEIGHTS set carrying the 100%-weighted Pmag geometry and rewrite
    # only low-Y partition vertices to the new joint slot.
    joint_sets = sorted(
        int(key.split("_", 1)[1]) for key in attrs if key.startswith("JOINTS_")
    )
    remapped = 0
    for vertex in low_vertices:
        matches: list[tuple[int, int, int]] = []
        for set_index in joint_sets:
            joint_accessor = attrs.get(f"JOINTS_{set_index}")
            weight_accessor = attrs.get(f"WEIGHTS_{set_index}")
            if not isinstance(joint_accessor, int) or not isinstance(weight_accessor, int):
                fail(f"unpaired JOINTS/WEIGHTS set {set_index}")
            joint_row = structure.raw_accessor_values(doc, bytes(binary_payload), joint_accessor)[vertex]
            weight_row = structure.raw_accessor_values(doc, bytes(binary_payload), weight_accessor)[vertex]
            for component, (joint_value, weight) in enumerate(zip(joint_row, weight_row)):
                if int(round(joint_value)) == old_slot and float(weight) >= 0.5:
                    matches.append((joint_accessor, component, set_index))
        if len(matches) != 1:
            fail(f"low-Y vertex {vertex} expected one Pmag influence, found {matches}")
        joint_accessor, component, _set_index = matches[0]
        write_joint_component(doc, binary_payload, joint_accessor, vertex, component, new_slot)
        remapped += 1
    if remapped != EXPECTED_LOW_VERTEX_COUNT:
        fail(f"remapped vertex count drifted: {remapped}")

    # Ensure high-Y side-saddle partition remains on the original Pmag slot and was not
    # silently pulled into the new fore-end joint.
    high_original = 0
    high_new = 0
    for vertex in high_vertices:
        for set_index in joint_sets:
            joint_accessor = attrs.get(f"JOINTS_{set_index}")
            weight_accessor = attrs.get(f"WEIGHTS_{set_index}")
            if not isinstance(joint_accessor, int) or not isinstance(weight_accessor, int):
                continue
            joint_row = structure.raw_accessor_values(doc, bytes(binary_payload), joint_accessor)[vertex]
            weight_row = structure.raw_accessor_values(doc, bytes(binary_payload), weight_accessor)[vertex]
            for joint_value, weight in zip(joint_row, weight_row):
                if float(weight) < 0.5:
                    continue
                slot = int(round(joint_value))
                if slot == old_slot:
                    high_original += 1
                elif slot == new_slot:
                    high_new += 1
    if high_new != 0 or high_original != EXPECTED_HIGH_VERTEX_COUNT:
        fail(
            f"high-Y side-saddle ownership drifted original={high_original} new={high_new}"
        )

    base_translation = old_node.get("translation")
    if not isinstance(base_translation, list) or len(base_translation) != 3:
        fail("Pmag joint lacks animatable TRS translation")
    base = [float(v) for v in base_translation]
    translations = (
        tuple(base),
        (base[0], base[1] + PUMP_STROKE_Y, base[2]),
        (base[0], base[1] + PUMP_STROKE_Y, base[2]),
        tuple(base),
    )
    time_payload = struct.pack("<" + "f" * len(PUMP_TIMES), *PUMP_TIMES)
    translation_payload = b"".join(struct.pack("<3f", *row) for row in translations)
    time_accessor = append_accessor(
        doc, binary_payload, time_payload, 5126, "SCALAR", len(PUMP_TIMES),
        minimum=[min(PUMP_TIMES)], maximum=[max(PUMP_TIMES)],
    )
    translation_accessor = append_accessor(
        doc, binary_payload, translation_payload, 5126, "VEC3", len(translations)
    )
    animations = doc.setdefault("animations", [])
    animations.append({
        "name": DERIVED_ANIMATION_NAME,
        "samplers": [{
            "input": time_accessor,
            "output": translation_accessor,
            "interpolation": "LINEAR",
        }],
        "channels": [{
            "sampler": 0,
            "target": {"node": new_joint_node, "path": "translation"},
        }],
    })

    buffers = doc.get("buffers") or []
    if len(buffers) != 1:
        fail(f"expected one embedded buffer, found {len(buffers)}")
    buffers[0]["byteLength"] = len(binary_payload)

    json_payload = json.dumps(doc, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    while len(json_payload) % 4:
        json_payload += b" "
    while len(binary_payload) % 4:
        binary_payload.append(0)
    total_length = 12 + 8 + len(json_payload) + 8 + len(binary_payload)
    glb = bytearray()
    glb.extend(struct.pack("<4sII", b"glTF", 2, total_length))
    glb.extend(struct.pack("<II", len(json_payload), 0x4E4F534A))
    glb.extend(json_payload)
    glb.extend(struct.pack("<II", len(binary_payload), 0x004E4942))
    glb.extend(binary_payload)

    report: dict[str, object] = {
        "schema": 1,
        "status": "DERIVED_PUMP_SOURCE_READY_FOR_ISOLATED_UE58_IMPORT",
        "source_sha256": EXPECTED_SOURCE_SHA256,
        "derived_sha256": hashlib.sha256(glb).hexdigest(),
        "license_id": "CC-BY-4.0",
        "attribution": "Remington 870 by 8sianDude, modified for Oster Conflict PASS45",
        "target_original_joint": TARGET_JOINT_NAME,
        "derived_joint": DERIVED_JOINT_NAME,
        "derived_animation": DERIVED_ANIMATION_NAME,
        "component_count": len(components),
        "low_y_component_count": len(LOW_Y_COMPONENTS),
        "low_y_vertex_count": len(low_vertices),
        "high_y_component_count": len(components) - len(LOW_Y_COMPONENTS),
        "high_y_vertex_count": len(high_vertices),
        "old_joint_slot": old_slot,
        "new_joint_slot": new_slot,
        "pump_stroke_y_source_units": PUMP_STROKE_Y,
        "pump_duration_seconds": PUMP_DURATION,
        "pump_key_times": list(PUMP_TIMES),
        "stroke_basis": "donor easy_reload Pmag_061 measured dominant-Y travel; unrelated X/Z/rotation omitted",
        "direct_pmag_as_pump_mapping": "REJECTED",
        "high_y_side_saddle_partition_moved_by_new_joint": False,
        "ue58_import_pending": True,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    return bytes(glb), report


def load_source(path: Path | None) -> bytes:
    if path is None:
        return remote.fetch_bytes()
    if not path.is_file():
        fail(f"source file missing: {path}")
    return path.read_bytes()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", help="optional local exact registered donor GLB")
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()

    source_path = Path(args.source) if args.source else None
    data = load_source(source_path)
    glb, report = build_derived(data)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    glb_path = out_dir / DERIVED_SOURCE_NAME
    manifest_path = out_dir / DERIVED_MANIFEST_NAME
    glb_path.write_bytes(glb)
    manifest_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(
        "PASS45_REMINGTON870_DERIVED_PUMP_SOURCE: PASS "
        f"source_sha256={EXPECTED_SOURCE_SHA256} derived_sha256={report['derived_sha256']} "
        f"components={report['component_count']} fore_end_vertices={report['low_y_vertex_count']} "
        f"side_saddle_vertices={report['high_y_vertex_count']} stroke_y={PUMP_STROKE_Y:.6f} "
        f"duration={PUMP_DURATION:.3f} ue58_import_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
