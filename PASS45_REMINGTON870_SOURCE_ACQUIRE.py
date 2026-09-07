#!/usr/bin/env python3
"""Acquire the exact audited Remington 870 GLB into Oster source assets.

This copies only the already-pinned donor bytes. It does not create a UE uasset,
wire gameplay, populate animation profiles, or claim runtime acceptance.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
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
WEAPON_NODE_PREFIXES = ("Rif_", "Trigger_", "PBody_", "Pmag_")


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


def glb_binary_chunk(data: bytes) -> bytes:
    if len(data) < 20:
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: GLB too short for binary probe")
    magic, version, declared_length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF" or version != 2 or declared_length != len(data):
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: GLB header drift during binary probe")

    offset = 12
    binary_payload: bytes | None = None
    while offset + 8 <= len(data):
        chunk_length, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        end = offset + chunk_length
        if end > len(data):
            raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: GLB chunk overruns file")
        payload = data[offset:end]
        offset = end
        if chunk_type == 0x004E4942:
            binary_payload = payload
            break
    if binary_payload is None:
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: GLB has no BIN chunk")
    return binary_payload


def accessor_values(doc: dict, binary_payload: bytes, accessor_index: int) -> list[tuple[float, ...]]:
    accessors = doc.get("accessors") or []
    buffer_views = doc.get("bufferViews") or []
    if accessor_index < 0 or accessor_index >= len(accessors):
        raise SystemExit(f"PASS45 REMINGTON870 SOURCE ACQUIRE: invalid accessor {accessor_index}")
    accessor = accessors[accessor_index]
    if accessor.get("sparse"):
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: sparse accessor {accessor_index} unsupported in fail-closed probe"
        )
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or view_index < 0 or view_index >= len(buffer_views):
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: accessor {accessor_index} missing valid bufferView"
        )
    view = buffer_views[view_index]
    if view.get("buffer", 0) != 0:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: accessor {accessor_index} references nonzero buffer"
        )

    component_type = accessor.get("componentType")
    if component_type != 5126:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: animation accessor {accessor_index} is not FLOAT"
        )
    type_components = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}
    accessor_type = accessor.get("type")
    component_count = type_components.get(accessor_type)
    if component_count is None:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: unsupported accessor type {accessor_type!r}"
        )
    count = accessor.get("count")
    if not isinstance(count, int) or count < 1:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: accessor {accessor_index} has invalid count"
        )

    packed_size = component_count * 4
    stride = view.get("byteStride", packed_size)
    if not isinstance(stride, int) or stride < packed_size:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: accessor {accessor_index} has invalid stride {stride}"
        )
    base_offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    fmt = "<" + ("f" * component_count)
    values: list[tuple[float, ...]] = []
    for item_index in range(count):
        item_offset = base_offset + item_index * stride
        if item_offset < 0 or item_offset + packed_size > len(binary_payload):
            raise SystemExit(
                f"PASS45 REMINGTON870 SOURCE ACQUIRE: accessor {accessor_index} exceeds BIN chunk"
            )
        values.append(tuple(float(v) for v in struct.unpack_from(fmt, binary_payload, item_offset)))
    return values


def sampled_animation_output(
    doc: dict,
    binary_payload: bytes,
    sampler: dict,
) -> list[tuple[float, ...]]:
    input_index = sampler.get("input")
    output_index = sampler.get("output")
    if not isinstance(input_index, int) or not isinstance(output_index, int):
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: animation sampler lacks accessors")
    accessors = doc.get("accessors") or []
    key_count = accessors[input_index].get("count") if input_index < len(accessors) else None
    if not isinstance(key_count, int) or key_count < 1:
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: invalid animation key count")
    values = accessor_values(doc, binary_payload, output_index)
    interpolation = str(sampler.get("interpolation") or "LINEAR").upper()
    if interpolation == "CUBICSPLINE":
        if len(values) != key_count * 3:
            raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: malformed CUBICSPLINE output")
        return values[1::3]
    if interpolation not in {"LINEAR", "STEP"}:
        raise SystemExit(
            f"PASS45 REMINGTON870 SOURCE ACQUIRE: unsupported interpolation {interpolation}"
        )
    if len(values) != key_count:
        raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: animation input/output count mismatch")
    return values


def max_translation_displacement(values: list[tuple[float, ...]]) -> float:
    if not values or len(values[0]) != 3:
        return 0.0
    origin = values[0]
    return max(
        math.sqrt(sum((sample[axis] - origin[axis]) ** 2 for axis in range(3)))
        for sample in values
    )


def max_quaternion_angle_degrees(values: list[tuple[float, ...]]) -> float:
    if not values or len(values[0]) != 4:
        return 0.0

    def normalize(q: tuple[float, ...]) -> tuple[float, ...]:
        norm = math.sqrt(sum(component * component for component in q))
        if norm <= 1e-12:
            raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: zero-length rotation quaternion")
        return tuple(component / norm for component in q)

    origin = normalize(values[0])
    max_angle = 0.0
    for value in values:
        current = normalize(value)
        dot = abs(sum(origin[i] * current[i] for i in range(4)))
        dot = max(-1.0, min(1.0, dot))
        angle = math.degrees(2.0 * math.acos(dot))
        max_angle = max(max_angle, angle)
    return max_angle


def donor_weapon_motion_summary(data: bytes, doc: dict) -> dict[str, dict[str, object]]:
    """Measure factual donor weapon-node motion; node semantics remain unclaimed."""
    binary_payload = glb_binary_chunk(data)
    animations = doc.get("animations") or []
    nodes = doc.get("nodes") or []
    summary: dict[str, dict[str, object]] = {}

    for index, semantic in ACTION_CLIPS:
        clip = animations[index]
        samplers = clip.get("samplers") or []
        clip_rows: dict[str, dict[str, object]] = {}
        for channel in clip.get("channels") or []:
            target = channel.get("target") or {}
            node_index = target.get("node")
            path = str(target.get("path") or "")
            sampler_index = channel.get("sampler")
            if not isinstance(node_index, int) or not isinstance(sampler_index, int):
                continue
            if node_index < 0 or node_index >= len(nodes) or sampler_index < 0 or sampler_index >= len(samplers):
                raise SystemExit("PASS45 REMINGTON870 SOURCE ACQUIRE: invalid animation channel linkage")
            node_name = str(nodes[node_index].get("name") or "").strip()
            if not node_name.startswith(WEAPON_NODE_PREFIXES):
                continue

            values = sampled_animation_output(doc, binary_payload, samplers[sampler_index])
            row = clip_rows.setdefault(
                node_name,
                {
                    "node_index": node_index,
                    "translation_max_displacement": 0.0,
                    "rotation_max_angle_degrees": 0.0,
                    "animated_paths": [],
                },
            )
            paths = row["animated_paths"]
            if isinstance(paths, list) and path not in paths:
                paths.append(path)
            if path == "translation":
                row["translation_max_displacement"] = round(max_translation_displacement(values), 6)
            elif path == "rotation":
                row["rotation_max_angle_degrees"] = round(max_quaternion_angle_degrees(values), 6)

        for row in clip_rows.values():
            paths = row.get("animated_paths")
            if isinstance(paths, list):
                paths.sort()
        summary[f"{semantic}_index_{index}"] = dict(sorted(clip_rows.items()))

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
    weapon_motion = donor_weapon_motion_summary(data, doc)

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
        "donor_weapon_motion": weapon_motion,
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
        "weapon_motion_measured=1 runtime_ready=0 ue58_import_pending=1 item16_checked=0"
    )


if __name__ == "__main__":
    main()
