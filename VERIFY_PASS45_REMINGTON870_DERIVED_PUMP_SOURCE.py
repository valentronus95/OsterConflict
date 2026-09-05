#!/usr/bin/env python3
"""Verify the deterministic derived Remington pump source remains fail-closed."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import PASS45_REMINGTON870_DERIVED_PUMP_SOURCE as derived
import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire
import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 DERIVED PUMP SOURCE VERIFY: FAIL\n[FAIL] {message}")


def node_index(nodes: list[dict], name: str) -> int:
    matches = [i for i, node in enumerate(nodes) if str(node.get("name") or "") == name]
    if len(matches) != 1:
        fail(f"expected one node named {name}, found {matches}")
    return matches[0]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--glb", required=True)
    parser.add_argument("--manifest", required=True)
    args = parser.parse_args()

    glb_path = Path(args.glb)
    manifest_path = Path(args.manifest)
    if not glb_path.is_file() or not manifest_path.is_file():
        fail("derived GLB/manifest missing")

    data = glb_path.read_bytes()
    report = json.loads(manifest_path.read_text(encoding="utf-8"))
    if report.get("source_sha256") != derived.EXPECTED_SOURCE_SHA256:
        fail("manifest source SHA drifted")
    if report.get("derived_sha256") != __import__("hashlib").sha256(data).hexdigest():
        fail("manifest derived SHA does not match GLB")
    required_report = {
        "status": "DERIVED_PUMP_SOURCE_READY_FOR_ISOLATED_UE58_IMPORT",
        "license_id": "CC-BY-4.0",
        "target_original_joint": derived.TARGET_JOINT_NAME,
        "derived_joint": derived.DERIVED_JOINT_NAME,
        "derived_animation": derived.DERIVED_ANIMATION_NAME,
        "component_count": derived.EXPECTED_COMPONENT_COUNT,
        "low_y_component_count": derived.EXPECTED_LOW_COMPONENT_COUNT,
        "low_y_vertex_count": derived.EXPECTED_LOW_VERTEX_COUNT,
        "high_y_component_count": derived.EXPECTED_HIGH_COMPONENT_COUNT,
        "high_y_vertex_count": derived.EXPECTED_HIGH_VERTEX_COUNT,
        "direct_pmag_as_pump_mapping": "REJECTED",
        "high_y_side_saddle_partition_moved_by_new_joint": False,
        "ue58_import_pending": True,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    for key, expected in required_report.items():
        if report.get(key) != expected:
            fail(f"manifest drift {key}: expected={expected!r} actual={report.get(key)!r}")

    doc = remote.parse_glb_json(data)
    binary_payload = acquire.glb_binary_chunk(data)
    nodes = doc.get("nodes") or []
    skins = doc.get("skins") or []
    meshes = doc.get("meshes") or []
    animations = doc.get("animations") or []
    if len(nodes) != 110:
        fail(f"derived node count expected 110, got {len(nodes)}")
    if len(animations) != 6:
        fail(f"derived animation count expected 6, got {len(animations)}")

    old_joint_node = node_index(nodes, derived.TARGET_JOINT_NAME)
    new_joint_node = node_index(nodes, derived.DERIVED_JOINT_NAME)
    mesh_node_index = node_index(nodes, derived.TARGET_MESH_NODE_NAME)
    mesh_node = nodes[mesh_node_index]
    skin_index = mesh_node.get("skin")
    mesh_index = mesh_node.get("mesh")
    if not isinstance(skin_index, int) or not isinstance(mesh_index, int):
        fail("target derived mesh node lost skin/mesh")
    skin = skins[skin_index]
    joints = skin.get("joints") or []
    old_slots = [i for i, node in enumerate(joints) if node == old_joint_node]
    new_slots = [i for i, node in enumerate(joints) if node == new_joint_node]
    if len(old_slots) != 1 or len(new_slots) != 1:
        fail(f"derived skin joint slots invalid old={old_slots} new={new_slots}")
    old_slot, new_slot = old_slots[0], new_slots[0]
    if new_slot != int(report.get("new_joint_slot", -1)):
        fail("manifest new joint slot drifted")

    primitive = (meshes[mesh_index].get("primitives") or [])[0]
    attrs = primitive.get("attributes") or {}
    position_accessor = attrs.get("POSITION")
    if not isinstance(position_accessor, int):
        fail("derived Object_95 primitive lacks POSITION")
    vertex_count = int((doc.get("accessors") or [])[position_accessor].get("count", 0))
    if vertex_count != derived.EXPECTED_TARGET_VERTEX_COUNT:
        fail(f"derived target vertex count drifted: {vertex_count}")

    joint_sets = sorted(int(k.split("_", 1)[1]) for k in attrs if k.startswith("JOINTS_"))
    old_owned = 0
    new_owned = 0
    other_owned = 0
    joint_rows_by_set: dict[int, list[tuple[float, ...]]] = {}
    weight_rows_by_set: dict[int, list[tuple[float, ...]]] = {}
    for set_index in joint_sets:
        ja = attrs.get(f"JOINTS_{set_index}")
        wa = attrs.get(f"WEIGHTS_{set_index}")
        if not isinstance(ja, int) or not isinstance(wa, int):
            fail(f"unpaired derived JOINTS/WEIGHTS set {set_index}")
        joint_rows_by_set[set_index] = structure.raw_accessor_values(doc, binary_payload, ja)
        weight_rows_by_set[set_index] = structure.raw_accessor_values(doc, binary_payload, wa)

    for vertex in range(vertex_count):
        owners: list[int] = []
        for set_index in joint_sets:
            for joint_value, weight in zip(joint_rows_by_set[set_index][vertex], weight_rows_by_set[set_index][vertex]):
                if float(weight) >= 0.5:
                    owners.append(int(round(joint_value)))
        if len(owners) != 1:
            fail(f"derived target vertex {vertex} expected one >=0.5 owner, found {owners}")
        if owners[0] == new_slot:
            new_owned += 1
        elif owners[0] == old_slot:
            old_owned += 1
        else:
            other_owned += 1
    if new_owned != derived.EXPECTED_LOW_VERTEX_COUNT:
        fail(f"derived fore-end owner count expected {derived.EXPECTED_LOW_VERTEX_COUNT}, got {new_owned}")
    if old_owned != derived.EXPECTED_HIGH_VERTEX_COUNT or other_owned != 0:
        fail(f"side-saddle ownership drift old={old_owned} other={other_owned}")

    matches = [animation for animation in animations if animation.get("name") == derived.DERIVED_ANIMATION_NAME]
    if len(matches) != 1:
        fail(f"derived pump animation count invalid: {len(matches)}")
    animation = matches[0]
    channels = animation.get("channels") or []
    samplers = animation.get("samplers") or []
    if len(channels) != 1 or len(samplers) != 1:
        fail("derived pump animation must contain exactly one channel/sampler")
    target = channels[0].get("target") or {}
    if target.get("node") != new_joint_node or target.get("path") != "translation":
        fail("derived pump animation does not exclusively target fore-end translation")
    sampler = samplers[0]
    times = acquire.accessor_values(doc, binary_payload, int(sampler["input"]))
    translations = acquire.accessor_values(doc, binary_payload, int(sampler["output"]))
    if [round(row[0], 6) for row in times] != [round(v, 6) for v in derived.PUMP_TIMES]:
        fail(f"derived pump time keys drifted: {times}")
    if len(translations) != len(derived.PUMP_TIMES):
        fail("derived pump translation key count drifted")
    base = translations[0]
    peak_delta_y = min(row[1] - base[1] for row in translations)
    if abs(peak_delta_y - derived.PUMP_STROKE_Y) > 1e-5:
        fail(f"derived pump Y stroke drifted: {peak_delta_y}")
    if any(abs(row[0] - base[0]) > 1e-6 or abs(row[2] - base[2]) > 1e-6 for row in translations):
        fail("derived pump animation introduced X/Z motion")

    print("PASS45 REMINGTON870 DERIVED PUMP SOURCE VERIFY: PASS")
    print(
        f"source_sha256={derived.EXPECTED_SOURCE_SHA256} derived_sha256={report['derived_sha256']} "
        f"fore_end_vertices={new_owned} side_saddle_vertices={old_owned} "
        f"pump_joint={derived.DERIVED_JOINT_NAME} pump_animation={derived.DERIVED_ANIMATION_NAME} "
        "isolated_ue58_import_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
