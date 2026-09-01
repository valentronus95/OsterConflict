#!/usr/bin/env python3
"""Acquire the exact audited Remington 870 GLB into Oster source assets.

This copies only the already-pinned donor bytes. It does not create a UE uasset,
wire gameplay, populate animation profiles, or claim runtime acceptance.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as audit

SOURCE_FILE = "remington_870_8siandude_ccby4.glb"
MANIFEST_FILE = "MANIFEST.json"
LICENSE_URL = "https://creativecommons.org/licenses/by/4.0/"
UPSTREAM_URL = "https://sketchfab.com/3d-models/remington-870-eea11de7e9d24b6683962b8388c319eb"
ATTRIBUTION = "Remington 870 by 8sianDude, licensed under CC BY 4.0"


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
        "intended_fp_clips": ["ironsight", "fire", "reload", "dryfire"],
        "source_asset_file": SOURCE_FILE,
        "derivative_notes": "Exact pinned donor GLB copied without geometry/animation modification; UE 5.8 import, clip mapping/retargeting, materials, first-person fit and runtime acceptance remain pending.",
        "runtime_ready": False,
        "ue58_import_pending": True,
        "item16_checked": False,
    }

    manifest_path = out_dir / MANIFEST_FILE
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(
        "PASS45_REMINGTON870_SOURCE_ACQUIRED "
        f"sha256={identity['sha256']} bytes={identity['size']} "
        f"animations={structure['animations']} skins={structure['skins']} "
        "runtime_ready=0 ue58_import_pending=1 item16_checked=0"
    )


if __name__ == "__main__":
    main()
