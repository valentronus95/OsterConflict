#!/usr/bin/env python3
"""Acquire the exact audited Remington 870 GLB into Oster source assets.

This copies only the already-pinned donor bytes. It does not create a UE uasset,
wire gameplay, populate animation profiles, or claim runtime acceptance.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as audit

SOURCE_FILE = "remington_870_8siandude_ccby4.glb"
MANIFEST_FILE = "MANIFEST.json"
LICENSE_URL = "https://creativecommons.org/licenses/by/4.0/"
UPSTREAM_URL = "https://sketchfab.com/3d-models/remington-870-eea11de7e9d24b6683962b8388c319eb"
ATTRIBUTION = "Remington 870 by 8sianDude, licensed under CC BY 4.0"
ACTION_CLIPS = (
    (2, "fire"),
    (3, "easy_reload"),
    (4, "full_reload"),
)


def donor_action_target_summary(doc: dict) -> dict[str, dict[str, object]]:
    """Record deterministic moving-target evidence without claiming UE acceptance."""
    animations = doc.get("animations") or []
    nodes = doc.get("nodes") or []
    summary: dict[str, dict[str, object]] = {}

    for index, semantic in ACTION_CLIPS:
        if index >= len(animations):
            raise SystemExit(
                f"PASS45 REMINGTON870 SOURCE ACQUIRE: animation index {index} ({semantic}) missing"
            )
        channels = animations[index].get("channels") or []
        target_node_indices: set[int] = set()
        path_counts: dict[str, int] = {}

        for channel in channels:
            target = channel.get("target") or {}
            node_index = target.get("node")
            target_path = str(target.get("path") or "UNKNOWN")
            path_counts[target_path] = path_counts.get(target_path, 0) + 1
            if isinstance(node_index, int):
                if node_index < 0 or node_index >= len(nodes):
                    raise SystemExit(
                        "PASS45 REMINGTON870 SOURCE ACQUIRE: "
                        f"animation index {index} ({semantic}) targets invalid node {node_index}"
                    )
                target_node_indices.add(node_index)

        ordered_indices = sorted(target_node_indices)
        target_node_names: list[str] = []
        for node_index in ordered_indices:
            raw_name = nodes[node_index].get("name")
            name = str(raw_name).strip() if raw_name is not None else ""
            target_node_names.append(name or f"<unnamed:{node_index}>")

        fingerprint_payload = {
            "animation_index": index,
            "semantic": semantic,
            "target_node_indices": ordered_indices,
            "target_node_names": target_node_names,
            "target_path_counts": dict(sorted(path_counts.items())),
        }
        fingerprint = hashlib.sha256(
            json.dumps(
                fingerprint_payload,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
            ).encode("utf-8")
        ).hexdigest()

        summary[f"{semantic}_index_{index}"] = {
            "channel_count": len(channels),
            "unique_target_nodes": len(ordered_indices),
            "target_node_indices": ordered_indices,
            "target_node_names": target_node_names,
            "target_path_counts": dict(sorted(path_counts.items())),
            "target_fingerprint_sha256": fingerprint,
        }

    return summary


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    data = audit.fetch_bytes()
    identity = audit.verify_pinned_bytes(data)
    doc = audit.parse_glb_json(data)
    audit.require_animation_contract(doc)
    audit.require_skin(doc)
    structure = audit.structural_summary(doc)
    action_targets = donor_action_target_summary(doc)

    source_path = out_dir / SOURCE_FILE
    source_path.write_bytes(data)

    manifest = {
        "schema": 1,
        "weapon": "Remington870",
        "status": "APPROVED_FOR_UE_IMPORT",
        "source_name": "Remington 870",
        "source_url": UPSTREAM_URL,
        "source_model_id": audit.UPSTREAM_MODEL_ID,
        "source_transport_repo": audit.REPO,
        "source_transport_commit": audit.COMMIT,
        "source_transport_path": audit.PATH,
        "source_git_blob_sha1": audit.EXPECTED_GIT_BLOB_SHA1,
        "source_sha256": identity["sha256"],
        "source_bytes": identity["size"],
        "license_id": audit.LICENSE_ID,
        "license_url": LICENSE_URL,
        "attribution": ATTRIBUTION,
        "public_repo_allowed": True,
        "rigged_or_articulated": True,
        "animation_capable": True,
        "donor_animation_count": structure["animations"],
        "donor_skin_count": structure["skins"],
        "donor_node_count": structure["nodes"],
        "donor_mesh_count": structure["meshes"],
        "proven_donor_action_channels": {
            "fire_index_2": structure["fire_channels"],
            "easy_reload_index_3": structure["easy_reload_channels"],
            "full_reload_index_4": structure["full_reload_channels"],
        },
        "donor_action_targets": action_targets,
        "intended_fp_clips": ["ironsight", "fire", "reload", "dryfire"],
        "source_asset_file": SOURCE_FILE,
        "derivative_notes": "Exact pinned donor GLB copied without geometry/animation modification; UE 5.8 import, clip mapping/retargeting, materials, first-person fit and runtime acceptance remain pending.",
        "runtime_ready": False,
        "ue58_import_pending": True,
        "item16_checked": False,
    }

    manifest_path = out_dir / MANIFEST_FILE
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    target_fingerprints = ",".join(
        f"{key}:{value['target_fingerprint_sha256']}"
        for key, value in sorted(action_targets.items())
    )
    print(
        "PASS45_REMINGTON870_SOURCE_ACQUIRED "
        f"sha256={identity['sha256']} bytes={identity['size']} "
        f"animations={structure['animations']} skins={structure['skins']} "
        f"action_target_fingerprints={target_fingerprints} "
        "runtime_ready=0 ue58_import_pending=1 item16_checked=0"
    )


if __name__ == "__main__":
    main()
