#!/usr/bin/env python3
"""Fail-closed structural/motion audit for the committed Stein CC0 Lever Action FBX.

The audit proves only what the pinned source actually contains. It does not close
PASS45 item 16, does not invent a lever pivot/range, and does not claim UE runtime
acceptance. GitHub Actions materializes only the exact LFS payload and converts it
to inspection-only glTF2 with Assimp. Mechanical name matches are discovery hints,
not acceptance evidence unless source hierarchy/skin/animation facts support them.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
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
    if not isinstance(nodes, list) or not isinstance(meshes, list) or not nodes or not meshes:
        fail("inspection glTF lacks inspectable nodes/meshes")
    if not isinstance(skins, list) or not isinstance(animations, list):
        fail("inspection glTF has malformed skins/animations")

    node_names = [clean_name(node.get("name"), f"<node:{i}>") for i, node in enumerate(nodes)]
    mesh_names = [clean_name(mesh.get("name"), f"<mesh:{i}>") for i, mesh in enumerate(meshes)]

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
                node = nodes[index]
                rows.append({
                    "node_index": index,
                    "node_name": name,
                    "matched_terms": hits,
                    "parent_index": parent,
                    "parent_name": node_names[parent] if parent is not None else None,
                    "children": [node_names[c] for c in (node.get("children") or [])],
                    "translation": finite_vec(node.get("translation"), 3),
                    "rotation": finite_vec(node.get("rotation"), 4),
                    "scale": finite_vec(node.get("scale"), 3),
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
            row = {
                "channel_index": channel_index,
                "node_index": node_index,
                "node_name": node_names[node_index],
                "path": path,
            }
            targets.append(row)
            if any(term in node_names[node_index].lower() for term in MECHANICAL_TERMS):
                mechanical_animation_targets.append({"animation_index": animation_index, **row})
        animation_rows.append({
            "animation_index": animation_index,
            "animation_name": clean_name(animation.get("name"), f"<animation:{animation_index}>"),
            "channel_count": len(channels),
            "targets": targets,
        })

    if mechanical_animation_targets:
        classification = "DIRECT_AUTHORED_MECHANICAL_MOTION_EVIDENCE"
    elif mechanical_joint_names:
        classification = "MECHANICAL_JOINT_DERIVATIVE_CANDIDATE_NO_EMBEDDED_MOTION"
    elif mechanical_nodes:
        classification = "MECHANICAL_NODE_DERIVATIVE_CANDIDATE_NO_EMBEDDED_MOTION"
    elif skins or animations:
        classification = "SKELETAL_OR_ANIMATED_SOURCE_MECHANICAL_IDENTITY_UNPROVEN"
    elif len(nodes) > 1 or len(meshes) > 1:
        classification = "PARTITIONED_SOURCE_MECHANICAL_IDENTITY_UNPROVEN"
    else:
        classification = "NO_DIRECT_MECHANICAL_MOTION_OR_PART_IDENTITY_EVIDENCE"

    report: dict[str, Any] = {
        "schema": 1,
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
        "mechanical_named_candidates": mechanical_nodes,
        "identity_named_candidates": identity_nodes,
        "skins": skin_rows,
        "mechanical_joint_names": mechanical_joint_names,
        "animated_node_names": [node_names[i] for i in sorted(animated_indices)],
        "mechanical_animation_targets": mechanical_animation_targets,
        "animations": animation_rows,
        "classification": classification,
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
        f"mechanical_nodes={len(mechanical_nodes)} mechanical_joints={len(mechanical_joint_names)} "
        f"mechanical_animation_targets={len(mechanical_animation_targets)} classification={classification} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
