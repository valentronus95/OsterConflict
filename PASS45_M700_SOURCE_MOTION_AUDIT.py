#!/usr/bin/env python3
"""Fail-closed structural/motion audit for the already-committed Stein CC0 M700 FBX.

This audit does not create a production asset and cannot close PASS45 item 16.
GitHub Actions materializes only the exact M700 LFS payload, Assimp converts it to
an inspection-only glTF, and this script records what the source actually contains.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

EXPECTED_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
EXPECTED_SIZE = 638732
EXPECTED_SOURCE = (
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/"
    "WeaponsPack/M700/SKM_M700.fbx"
)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 M700 SOURCE MOTION AUDIT: FAIL\n[FAIL] {message}")


def clean_name(value: object, fallback: str) -> str:
    text = str(value or "").strip()
    return text or fallback


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
    animations = doc.get("animations") or []
    if not isinstance(nodes, list) or not isinstance(meshes, list):
        fail("inspection glTF lacks nodes/meshes arrays")
    if not nodes or not meshes:
        fail("inspection glTF contains no inspectable M700 nodes/meshes")

    node_names = [clean_name(node.get("name"), f"<node:{i}>") for i, node in enumerate(nodes)]
    mesh_names = [clean_name(mesh.get("name"), f"<mesh:{i}>") for i, mesh in enumerate(meshes)]

    def names_matching(needles: tuple[str, ...]) -> list[str]:
        out: list[str] = []
        seen: set[str] = set()
        for name in node_names + mesh_names:
            lower = name.lower()
            if any(needle in lower for needle in needles) and name not in seen:
                seen.add(name)
                out.append(name)
        return out

    bolt_named = names_matching(("bolt",))
    mechanical_named = names_matching(("bolt", "breech", "action", "handle", "receiver"))
    identity_named = names_matching(("m700", "rifle", "trigger", "mag", "scope", "barrel", "stock"))

    animated_node_indices: set[int] = set()
    animation_rows: list[dict[str, object]] = []
    for animation_index, animation in enumerate(animations):
        channels = animation.get("channels") or []
        targeted: list[dict[str, object]] = []
        for channel in channels:
            target = channel.get("target") or {}
            node_index = target.get("node")
            path = target.get("path")
            if isinstance(node_index, int) and 0 <= node_index < len(nodes):
                animated_node_indices.add(node_index)
                targeted.append({
                    "node_index": node_index,
                    "node_name": node_names[node_index],
                    "path": path,
                })
        animation_rows.append({
            "animation_index": animation_index,
            "animation_name": clean_name(animation.get("name"), f"<animation:{animation_index}>"),
            "channel_count": len(channels),
            "targets": targeted,
        })

    animated_node_names = [node_names[i] for i in sorted(animated_node_indices)]
    animated_bolt_nodes = [name for name in animated_node_names if "bolt" in name.lower()]

    skinned_node_rows: list[dict[str, object]] = []
    for node_index, node in enumerate(nodes):
        skin_index = node.get("skin")
        if isinstance(skin_index, int):
            skinned_node_rows.append({
                "node_index": node_index,
                "node_name": node_names[node_index],
                "skin_index": skin_index,
                "mesh_index": node.get("mesh"),
            })

    joint_names: list[str] = []
    for skin_index, skin in enumerate(skins):
        for joint in skin.get("joints") or []:
            if isinstance(joint, int) and 0 <= joint < len(nodes):
                joint_names.append(node_names[joint])
            else:
                fail(f"skin {skin_index} contains invalid joint index {joint!r}")
    joint_names = list(dict.fromkeys(joint_names))
    bolt_joint_names = [name for name in joint_names if "bolt" in name.lower()]

    if animated_bolt_nodes:
        classification = "DIRECT_AUTHORED_BOLT_MOTION_EVIDENCE"
    elif bolt_joint_names:
        classification = "BOLT_JOINT_DERIVATIVE_CANDIDATE_NO_EMBEDDED_BOLT_MOTION"
    elif bolt_named:
        classification = "BOLT_GEOMETRY_DERIVATIVE_CANDIDATE_NO_EMBEDDED_BOLT_MOTION"
    elif skins or animations:
        classification = "SKELETAL_OR_ANIMATED_SOURCE_REVIEW_REQUIRED_BOLT_IDENTITY_UNPROVEN"
    elif len(nodes) > 1 or len(meshes) > 1:
        classification = "PARTITIONED_SOURCE_REVIEW_REQUIRED_BOLT_IDENTITY_UNPROVEN"
    else:
        classification = "NO_DIRECT_BOLT_MOTION_OR_PART_IDENTITY_EVIDENCE"

    report = {
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
        "bolt_named_candidates": bolt_named,
        "mechanical_named_candidates": mechanical_named,
        "identity_named_candidates": identity_named,
        "joint_names": joint_names,
        "bolt_joint_names": bolt_joint_names,
        "skinned_nodes": skinned_node_rows,
        "animated_node_names": animated_node_names,
        "animated_bolt_nodes": animated_bolt_nodes,
        "animations": animation_rows,
        "classification": classification,
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    print(
        "PASS45 M700 SOURCE MOTION AUDIT: COMPLETE "
        f"nodes={len(nodes)} meshes={len(meshes)} skins={len(skins)} animations={len(animations)} "
        f"bolt_named={len(bolt_named)} bolt_joints={len(bolt_joint_names)} "
        f"animated_bolt_nodes={len(animated_bolt_nodes)} classification={classification} "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
