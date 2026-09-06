#!/usr/bin/env python3
"""Fail-closed identity audit for the unregistered local Remington 870 MotionLab candidate.

This tool does not promote content. It only fingerprints one local GLB and records
structural metadata needed to resolve provenance without conflating it with the
registered 8sianDude donor.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

LOCAL_CANDIDATE_BASENAME = "Remington_870_FREE.glb"
REQUIRED_ACTION_NAMES = ("PumpAction", "Cube.002Action")
REGISTERED_CANONICAL_DONOR = (
    "SOURCE_ASSETS/PASS45/Remington870/remington_870_8siandude_ccby4.glb"
)
STATUS = "UNREGISTERED_LOCAL_CANDIDATE"
PROMOTION_BLOCKERS = (
    "source_url_unpinned",
    "creator_unpinned",
    "license_unpinned",
    "repository_payload_unpinned",
)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 LOCAL CANDIDATE IDENTITY AUDIT: FAIL\n[FAIL] {message}")


def parse_glb_json(data: bytes) -> dict:
    if len(data) < 20:
        fail("GLB is too short")
    magic, version, declared_length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF":
        fail(f"unexpected GLB magic {magic!r}")
    if version != 2:
        fail(f"expected glTF 2.0 binary, got {version}")
    if declared_length != len(data):
        fail(f"declared length {declared_length} != actual {len(data)}")

    offset = 12
    json_payload: bytes | None = None
    while offset + 8 <= len(data):
        chunk_length, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        end = offset + chunk_length
        if end > len(data):
            fail("GLB chunk extends past file end")
        payload = data[offset:end]
        offset = end
        if chunk_type == 0x4E4F534A:
            json_payload = payload
            break
    if json_payload is None:
        fail("GLB has no JSON chunk")
    try:
        return json.loads(json_payload.decode("utf-8").rstrip("\x00 \t\r\n"))
    except Exception as exc:
        fail(f"invalid GLB JSON chunk: {exc}")
    raise AssertionError("unreachable")


def clean_name(value: object, fallback: str) -> str:
    text = str(value or "").strip()
    return text or fallback


def action_summary(doc: dict) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    nodes = doc.get("nodes") or []
    for index, animation in enumerate(doc.get("animations") or []):
        channels = animation.get("channels") or []
        target_names: set[str] = set()
        target_paths: dict[str, int] = {}
        for channel in channels:
            target = channel.get("target") or {}
            node_index = target.get("node")
            if isinstance(node_index, int) and 0 <= node_index < len(nodes):
                target_names.add(clean_name(nodes[node_index].get("name"), f"<node:{node_index}>"))
            path = clean_name(target.get("path"), "<unknown>")
            target_paths[path] = target_paths.get(path, 0) + 1
        rows.append(
            {
                "index": index,
                "name": clean_name(animation.get("name"), f"<animation:{index}>"),
                "channel_count": len(channels),
                "sampler_count": len(animation.get("samplers") or []),
                "target_node_names": sorted(target_names),
                "target_path_counts": dict(sorted(target_paths.items())),
            }
        )
    return rows


def inspect(path: Path) -> dict[str, object]:
    if path.name != LOCAL_CANDIDATE_BASENAME:
        fail(
            f"expected basename {LOCAL_CANDIDATE_BASENAME!r}, got {path.name!r}; "
            "do not reuse this audit as a generic donor promotion path"
        )
    data = path.read_bytes()
    doc = parse_glb_json(data)
    actions = action_summary(doc)
    action_names = [str(row["name"]) for row in actions]
    missing = [name for name in REQUIRED_ACTION_NAMES if name not in action_names]
    if missing:
        fail(f"MotionLab contract drift: missing actions {missing}; observed {action_names}")

    asset = doc.get("asset") or {}
    materials = doc.get("materials") or []
    meshes = doc.get("meshes") or []
    nodes = doc.get("nodes") or []
    skins = doc.get("skins") or []

    return {
        "audit": "PASS45_REMINGTON870_LOCAL_CANDIDATE_IDENTITY_AUDIT",
        "schema": 1,
        "status": STATUS,
        "candidate_basename": path.name,
        "bytes": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "glb_asset": {
            "version": asset.get("version"),
            "minVersion": asset.get("minVersion"),
            "generator": asset.get("generator"),
            "copyright": asset.get("copyright"),
            "extras": asset.get("extras"),
        },
        "animation_names": action_names,
        "animations": actions,
        "node_count": len(nodes),
        "mesh_count": len(meshes),
        "skin_count": len(skins),
        "material_names": [
            clean_name(material.get("name"), f"<material:{index}>")
            for index, material in enumerate(materials)
        ],
        "required_motionlab_actions_preserved": True,
        "registered_canonical_donor": REGISTERED_CANONICAL_DONOR,
        "same_as_registered_canonical_donor": "UNPROVEN",
        "source_url": None,
        "creator": None,
        "license_id": None,
        "public_repo_allowed": None,
        "promotion_blockers": list(PROMOTION_BLOCKERS),
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("inspect",))
    parser.add_argument("input", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    summary = inspect(args.input)
    rendered = json.dumps(summary, indent=2, sort_keys=True)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered + "\n", encoding="utf-8")
    print("PASS45 REMINGTON870 LOCAL CANDIDATE IDENTITY AUDIT: INSPECTED / QUARANTINED")
    print(rendered)


if __name__ == "__main__":
    main()
