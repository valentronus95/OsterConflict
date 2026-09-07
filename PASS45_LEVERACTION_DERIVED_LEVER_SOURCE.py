#!/usr/bin/env python3
"""Build a deterministic pilot-only Lever Action moving-part source.

The committed Stein CC0 FBX has a distinct weighted LEVER joint but no authored
animation channels or authored open-angle/end-point. This builder therefore
reuses the exact source geometry/skeleton and adds a bounded calibration-only
rotation channel to LEVER after Assimp inspection conversion.

The 0.85 s duration matches Oster's existing authoritative LeverAction gameplay
cycle. The 45 degree local-X excursion is explicitly an Oster calibration pilot,
not a source-authored endpoint and not production/runtime acceptance.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import shutil
import struct
from pathlib import Path

EXPECTED_SOURCE = Path(
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/LeverAction/SKM_LeverAction.fbx"
)
EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
EXPECTED_SOURCE_SIZE = 570332
LEVER_NODE_NAME = "LEVER"
DERIVED_ANIMATION_NAME = "PASS45_LeverAction_Cycle"
DERIVED_GLTF_NAME = "leveraction_pass45_derived_cycle.gltf"
DERIVED_BIN_NAME = "leveraction_pass45_derived_cycle.bin"
MANIFEST_NAME = "PASS45_LEVERACTION_DERIVED_LEVER_MANIFEST.json"
CYCLE_DURATION = 0.85
PILOT_MAX_ANGLE_DEG = -45.0
KEY_TIMES = (0.0, 0.20, 0.42, 0.66, CYCLE_DURATION)
KEY_ANGLES_DEG = (0.0, PILOT_MAX_ANGLE_DEG * 0.55, PILOT_MAX_ANGLE_DEG, PILOT_MAX_ANGLE_DEG * 0.35, 0.0)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 LEVERACTION DERIVED LEVER SOURCE: FAIL\n[FAIL] {message}")


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def append_aligned(payload: bytearray, data: bytes) -> tuple[int, int]:
    while len(payload) % 4:
        payload.append(0)
    offset = len(payload)
    payload.extend(data)
    return offset, len(data)


def append_accessor(
    doc: dict,
    payload: bytearray,
    raw: bytes,
    *,
    component_type: int,
    accessor_type: str,
    count: int,
    minimum: list[float] | None = None,
    maximum: list[float] | None = None,
) -> int:
    offset, length = append_aligned(payload, raw)
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


def quat_x(degrees: float) -> tuple[float, float, float, float]:
    half = math.radians(degrees) * 0.5
    return (math.sin(half), 0.0, 0.0, math.cos(half))


def normalize_lever_node(node: dict) -> list[float]:
    """Convert the audited translation-only matrix to animation-compatible TRS."""
    matrix = node.get("matrix")
    if not isinstance(matrix, list) or len(matrix) != 16:
        fail("LEVER node no longer uses the audited 4x4 matrix form")
    values = [float(value) for value in matrix]
    expected_basis = (
        values[0], values[1], values[2], values[4], values[5], values[6], values[8], values[9], values[10]
    )
    identity_basis = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
    if any(abs(a - b) > 1e-6 for a, b in zip(expected_basis, identity_basis)):
        fail(f"LEVER bind basis drifted: {expected_basis}")
    if any(abs(values[index]) > 1e-6 for index in (3, 7, 11)) or abs(values[15] - 1.0) > 1e-6:
        fail("LEVER matrix contains unsupported projective terms")
    translation = [values[12], values[13], values[14]]
    expected_translation = [
        -8.145542018667129e-10,
        -0.14167749881744385,
        -0.012812890112400055,
    ]
    if any(abs(a - b) > 1e-6 for a, b in zip(translation, expected_translation)):
        fail(f"LEVER bind translation drifted: {translation}")
    node.pop("matrix", None)
    node["translation"] = translation
    node["rotation"] = [0.0, 0.0, 0.0, 1.0]
    node["scale"] = [1.0, 1.0, 1.0]
    return translation


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", default=str(EXPECTED_SOURCE))
    parser.add_argument("--gltf", required=True)
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()

    source = Path(args.source)
    gltf_path = Path(args.gltf)
    out_dir = Path(args.out_dir)
    if source.as_posix() != EXPECTED_SOURCE.as_posix():
        fail(f"unexpected source path: {source.as_posix()}")
    if not source.is_file():
        fail(f"source missing: {source}")
    source_bytes = source.read_bytes()
    if source_bytes.startswith(b"version https://git-lfs.github.com/spec/v1"):
        fail("Lever Action Git LFS payload is not hydrated")
    if len(source_bytes) != EXPECTED_SOURCE_SIZE:
        fail(f"source size drifted: {len(source_bytes)}")
    source_digest = sha256_bytes(source_bytes)
    if source_digest != EXPECTED_SOURCE_SHA256:
        fail(f"source SHA-256 drifted: {source_digest}")

    if not gltf_path.is_file():
        fail(f"inspection glTF missing: {gltf_path}")
    try:
        original_doc = json.loads(gltf_path.read_text(encoding="utf-8"))
    except Exception as exc:
        fail(f"invalid inspection glTF: {exc}")
    if original_doc.get("animations"):
        fail("source conversion unexpectedly contains authored animation; re-audit before deriving")
    buffers = original_doc.get("buffers") or []
    if len(buffers) != 1:
        fail(f"expected one Assimp external buffer, found {len(buffers)}")
    uri = buffers[0].get("uri")
    if not isinstance(uri, str) or not uri or uri.startswith("data:"):
        fail("inspection glTF buffer is not a supported external file")
    original_bin = (gltf_path.parent / uri).resolve()
    if not original_bin.is_file():
        fail(f"inspection BIN missing: {original_bin}")

    doc = copy.deepcopy(original_doc)
    nodes = doc.get("nodes") or []
    matches = [index for index, node in enumerate(nodes) if str(node.get("name") or "") == LEVER_NODE_NAME]
    if len(matches) != 1:
        fail(f"expected one LEVER node, found {matches}")
    lever_node_index = matches[0]
    lever_node = nodes[lever_node_index]

    skins = doc.get("skins") or []
    joint_memberships = [index for index, skin in enumerate(skins) if lever_node_index in (skin.get("joints") or [])]
    if len(joint_memberships) != 1:
        fail(f"LEVER must belong to exactly one skin, memberships={joint_memberships}")
    bind_translation = normalize_lever_node(lever_node)

    payload = bytearray(original_bin.read_bytes())
    time_raw = b"".join(struct.pack("<f", float(value)) for value in KEY_TIMES)
    rotation_values = [quat_x(angle) for angle in KEY_ANGLES_DEG]
    rotation_raw = b"".join(struct.pack("<4f", *quat) for quat in rotation_values)
    time_accessor = append_accessor(
        doc,
        payload,
        time_raw,
        component_type=5126,
        accessor_type="SCALAR",
        count=len(KEY_TIMES),
        minimum=[0.0],
        maximum=[CYCLE_DURATION],
    )
    rotation_accessor = append_accessor(
        doc,
        payload,
        rotation_raw,
        component_type=5126,
        accessor_type="VEC4",
        count=len(rotation_values),
    )
    doc["animations"] = [
        {
            "name": DERIVED_ANIMATION_NAME,
            "samplers": [
                {
                    "input": time_accessor,
                    "output": rotation_accessor,
                    "interpolation": "LINEAR",
                }
            ],
            "channels": [
                {
                    "sampler": 0,
                    "target": {"node": lever_node_index, "path": "rotation"},
                }
            ],
        }
    ]
    doc["buffers"][0]["uri"] = DERIVED_BIN_NAME
    doc["buffers"][0]["byteLength"] = len(payload)

    asset = doc.setdefault("asset", {})
    extras = asset.setdefault("extras", {})
    extras["PASS45"] = {
        "classification": "LEVERACTION_DERIVED_CALIBRATION_PILOT_ONLY",
        "source_authored_motion": False,
        "source_authored_endpoint": False,
        "derived_axis": "local_x",
        "derived_axis_basis": "weighted_geometry_plane_normal_candidate",
        "pilot_max_angle_deg": PILOT_MAX_ANGLE_DEG,
        "pilot_angle_accepted": False,
        "authoritative_cycle_seconds": CYCLE_DURATION,
        "second_gameplay_timer": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }

    out_dir.mkdir(parents=True, exist_ok=True)
    out_gltf = out_dir / DERIVED_GLTF_NAME
    out_bin = out_dir / DERIVED_BIN_NAME
    out_manifest = out_dir / MANIFEST_NAME
    out_bin.write_bytes(payload)
    out_gltf.write_text(json.dumps(doc, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    manifest = {
        "schema": 1,
        "status": "LEVERACTION_DERIVED_CALIBRATION_PILOT_ONLY",
        "source": EXPECTED_SOURCE.as_posix(),
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": source_digest,
        "source_size": len(source_bytes),
        "inspection_converter": "Assimp -> glTF2",
        "lever_node": LEVER_NODE_NAME,
        "lever_node_index": lever_node_index,
        "lever_skin_index": joint_memberships[0],
        "lever_bind_translation": bind_translation,
        "source_authored_animation_count": 0,
        "source_authored_endpoint": False,
        "derived_animation": DERIVED_ANIMATION_NAME,
        "derived_target_path": "rotation",
        "derived_axis": "local_x",
        "derived_axis_basis": "weighted_geometry_plane_normal_candidate",
        "pilot_max_angle_deg": PILOT_MAX_ANGLE_DEG,
        "pilot_angle_accepted": False,
        "cycle_duration_seconds": CYCLE_DURATION,
        "key_times": list(KEY_TIMES),
        "key_angles_deg": list(KEY_ANGLES_DEG),
        "derived_gltf": DERIVED_GLTF_NAME,
        "derived_gltf_sha256": sha256_bytes(out_gltf.read_bytes()),
        "derived_bin": DERIVED_BIN_NAME,
        "derived_bin_sha256": sha256_bytes(out_bin.read_bytes()),
        "production_cutover": False,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    out_manifest.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(
        "PASS45_LEVERACTION_DERIVED_LEVER_SOURCE_READY "
        f"source_sha256={source_digest} joint={LEVER_NODE_NAME} "
        f"animation={DERIVED_ANIMATION_NAME} duration={CYCLE_DURATION:.2f} "
        f"pilot_max_angle_deg={PILOT_MAX_ANGLE_DEG:.1f} source_authored_endpoint=0 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
