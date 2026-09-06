#!/usr/bin/env python3
"""Build a deterministic pilot-only M700 bolt translation source.

The committed Stein CC0 M700 FBX has a distinct weighted BOLT joint and a separate
weighted BOLT_STOP component, but no authored animation channels. The audited
BOLT_STOP transform is explicitly NOT a proven bolt-travel endpoint.

This builder therefore keeps the exact source geometry/skeleton and adds only a
bounded calibration-only translation channel on BOLT. The pilot axis is local Y
because the audited BOLT weighted-geometry AABB is strongly longest on Y. The
travel magnitude is deliberately a small fraction of that audited geometry extent;
it is NOT source-authored, NOT accepted production travel, and does not close
PASS45 item 16. Local UE 5.8 visual calibration must determine the final
translation/rotation cycle before production cutover.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SOURCE = Path(
    "OsterConflict/Content/Raw/R13/Weapons/SteinClassicWeapons/WeaponsPack/M700/SKM_M700.fbx"
)
EXPECTED_SOURCE_SHA256 = "b7e003e01be8441e452730bc06c38c5e9752e523ae1b401ed2a6cc6cdca16840"
EXPECTED_SOURCE_SIZE = 638732
BOLT_NODE_NAME = "BOLT"
BOLT_STOP_NODE_NAME = "BOLT_STOP"
DERIVED_ANIMATION_NAME = "PASS45_M700_BoltTranslationPilot"
DERIVED_GLTF_NAME = "m700_pass45_derived_bolt_translation.gltf"
DERIVED_BIN_NAME = "m700_pass45_derived_bolt_translation.bin"
MANIFEST_NAME = "PASS45_M700_DERIVED_BOLT_TRANSLATION_MANIFEST.json"
CYCLE_DURATION = 1.10
AUDITED_BOLT_AABB_EXTENT = (0.05160917155444622, 0.19576822873204947, 0.05394992511719465)
PILOT_TRAVEL_FRACTION_OF_Y_EXTENT = 0.20
PILOT_MAX_TRAVEL = AUDITED_BOLT_AABB_EXTENT[1] * PILOT_TRAVEL_FRACTION_OF_Y_EXTENT
KEY_TIMES = (0.0, 0.18, 0.48, 0.78, CYCLE_DURATION)
KEY_OFFSETS_Y = (0.0, PILOT_MAX_TRAVEL * 0.18, PILOT_MAX_TRAVEL, PILOT_MAX_TRAVEL * 0.32, 0.0)
EXPECTED_BIND_TRANSLATION = (
    -8.273194544017315e-05,
    -0.14141424000263214,
    -0.03047895058989525,
)


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 M700 DERIVED BOLT TRANSLATION SOURCE: FAIL\n[FAIL] {message}")


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


def load_json(path: Path, label: str) -> dict:
    if not path.is_file():
        fail(f"{label} missing: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        fail(f"invalid {label}: {exc}")
    if not isinstance(value, dict):
        fail(f"{label} root is not an object")
    return value


def validate_audits(motion: dict, geometry: dict) -> None:
    for label, audit in (("motion audit", motion), ("geometry audit", geometry)):
        if audit.get("source_sha256") != EXPECTED_SOURCE_SHA256:
            fail(f"{label} source SHA drifted")
        if audit.get("production_cutover") is not False:
            fail(f"{label} falsely claims production cutover")
        if audit.get("runtime_acceptance") is not False:
            fail(f"{label} falsely claims runtime acceptance")
        if audit.get("item16_checked") is not False:
            fail(f"{label} falsely closes item 16")

    if int(motion.get("animation_count", -1)) != 0:
        fail("motion audit no longer proves zero authored animations")
    if motion.get("classification") != "BOLT_JOINT_DERIVATIVE_CANDIDATE_NO_EMBEDDED_BOLT_MOTION":
        fail(f"motion audit classification drifted: {motion.get('classification')!r}")
    if (
        (motion.get("source_authored_stop_delta") or {}).get(
            "usable_as_sibling_local_translation_delta"
        )
        is not True
    ):
        fail("motion audit source sibling fact drifted")
    if motion.get("bolt_endpoint_classification") != "BOLT_AND_BOLT_STOP_BOTH_WEIGHT_GEOMETRY_REVIEW_REQUIRED":
        fail("motion audit endpoint classification drifted")

    if geometry.get("classification") != "DISTINCT_WEIGHTED_COMPONENTS_NO_SHARED_VERTICES":
        fail(f"geometry audit classification drifted: {geometry.get('classification')!r}")
    if geometry.get("target_weighted_vertices_disjoint") is not True:
        fail("BOLT/BOLT_STOP are no longer proven disjoint")
    if geometry.get("source_authored_stop_delta_safe_as_bolt_travel") is not False:
        fail("BOLT_STOP must remain rejected as a bolt-travel endpoint")
    joint_geometry = geometry.get("joint_geometry") or {}
    bolt = joint_geometry.get(BOLT_NODE_NAME) or {}
    stop = joint_geometry.get(BOLT_STOP_NODE_NAME) or {}
    if int(bolt.get("weighted_vertex_count", -1)) != 1317:
        fail(f"BOLT weighted vertex count drifted: {bolt.get('weighted_vertex_count')!r}")
    if int(stop.get("weighted_vertex_count", -1)) != 60:
        fail(f"BOLT_STOP weighted vertex count drifted: {stop.get('weighted_vertex_count')!r}")
    extent = bolt.get("aabb_extent")
    if not isinstance(extent, list) or len(extent) != 3:
        fail("BOLT AABB extent missing")
    actual = tuple(float(v) for v in extent)
    if any(abs(a - b) > 1e-9 for a, b in zip(actual, AUDITED_BOLT_AABB_EXTENT)):
        fail(f"BOLT AABB extent drifted: {actual}")
    if not (actual[1] > actual[0] * 2.5 and actual[1] > actual[2] * 2.5):
        fail("local Y is no longer a strongly dominant BOLT geometry axis")


def normalize_bolt_node(node: dict) -> list[float]:
    matrix = node.get("matrix")
    if not isinstance(matrix, list) or len(matrix) != 16:
        fail("BOLT node no longer uses the audited 4x4 matrix form")
    values = [float(value) for value in matrix]
    basis = (
        values[0], values[1], values[2],
        values[4], values[5], values[6],
        values[8], values[9], values[10],
    )
    identity = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
    if any(abs(a - b) > 1e-6 for a, b in zip(basis, identity)):
        fail(f"BOLT bind basis drifted: {basis}")
    if any(abs(values[index]) > 1e-6 for index in (3, 7, 11)) or abs(values[15] - 1.0) > 1e-6:
        fail("BOLT matrix contains unsupported projective terms")
    translation = [values[12], values[13], values[14]]
    if any(abs(a - b) > 1e-6 for a, b in zip(translation, EXPECTED_BIND_TRANSLATION)):
        fail(f"BOLT bind translation drifted: {translation}")
    node.pop("matrix", None)
    node["translation"] = translation
    node["rotation"] = [0.0, 0.0, 0.0, 1.0]
    node["scale"] = [1.0, 1.0, 1.0]
    return translation


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", default=str(EXPECTED_SOURCE))
    parser.add_argument("--gltf", required=True)
    parser.add_argument("--motion-audit", required=True)
    parser.add_argument("--geometry-audit", required=True)
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
        fail("M700 Git LFS payload is not hydrated")
    if len(source_bytes) != EXPECTED_SOURCE_SIZE:
        fail(f"source size drifted: {len(source_bytes)}")
    source_digest = sha256_bytes(source_bytes)
    if source_digest != EXPECTED_SOURCE_SHA256:
        fail(f"source SHA-256 drifted: {source_digest}")

    motion_audit = load_json(Path(args.motion_audit), "motion audit")
    geometry_audit = load_json(Path(args.geometry_audit), "geometry audit")
    validate_audits(motion_audit, geometry_audit)

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
    bolt_matches = [i for i, node in enumerate(nodes) if str(node.get("name") or "") == BOLT_NODE_NAME]
    stop_matches = [i for i, node in enumerate(nodes) if str(node.get("name") or "") == BOLT_STOP_NODE_NAME]
    if len(bolt_matches) != 1 or len(stop_matches) != 1:
        fail(f"expected unique BOLT/BOLT_STOP nodes, bolt={bolt_matches} stop={stop_matches}")
    bolt_node_index = bolt_matches[0]
    stop_node_index = stop_matches[0]

    skins = doc.get("skins") or []
    memberships = [i for i, skin in enumerate(skins) if bolt_node_index in (skin.get("joints") or [])]
    if len(memberships) != 1:
        fail(f"BOLT must belong to exactly one skin, memberships={memberships}")
    if stop_node_index not in (skins[memberships[0]].get("joints") or []):
        fail("BOLT_STOP no longer belongs to the same audited skin")

    bind_translation = normalize_bolt_node(nodes[bolt_node_index])
    payload = bytearray(original_bin.read_bytes())

    time_raw = b"".join(struct.pack("<f", float(value)) for value in KEY_TIMES)
    translations = [
        (bind_translation[0], bind_translation[1] + offset_y, bind_translation[2])
        for offset_y in KEY_OFFSETS_Y
    ]
    translation_raw = b"".join(struct.pack("<3f", *value) for value in translations)

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
    translation_accessor = append_accessor(
        doc,
        payload,
        translation_raw,
        component_type=5126,
        accessor_type="VEC3",
        count=len(translations),
        minimum=[
            min(value[0] for value in translations),
            min(value[1] for value in translations),
            min(value[2] for value in translations),
        ],
        maximum=[
            max(value[0] for value in translations),
            max(value[1] for value in translations),
            max(value[2] for value in translations),
        ],
    )

    doc["animations"] = [
        {
            "name": DERIVED_ANIMATION_NAME,
            "samplers": [
                {
                    "input": time_accessor,
                    "output": translation_accessor,
                    "interpolation": "LINEAR",
                }
            ],
            "channels": [
                {
                    "sampler": 0,
                    "target": {"node": bolt_node_index, "path": "translation"},
                }
            ],
        }
    ]
    doc["buffers"][0]["uri"] = DERIVED_BIN_NAME
    doc["buffers"][0]["byteLength"] = len(payload)

    extras = doc.setdefault("asset", {}).setdefault("extras", {})
    extras["PASS45"] = {
        "classification": "M700_BOLT_TRANSLATION_CALIBRATION_PILOT_ONLY",
        "source_authored_motion": False,
        "source_authored_endpoint": False,
        "bolt_stop_used_as_endpoint": False,
        "derived_axis": "local_y",
        "derived_axis_basis": "audited_bolt_weighted_geometry_aabb_long_axis_candidate",
        "pilot_travel": PILOT_MAX_TRAVEL,
        "pilot_travel_accepted": False,
        "rotation_channel_authored": False,
        "rotation_calibration_pending": True,
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
        "status": "M700_BOLT_TRANSLATION_CALIBRATION_PILOT_ONLY",
        "source": EXPECTED_SOURCE.as_posix(),
        "source_license": "CC0-1.0 (Stein Games Classic Weapons Pack; repository provenance)",
        "source_sha256": source_digest,
        "source_size": len(source_bytes),
        "inspection_converter": "Assimp -> glTF2",
        "bolt_node": BOLT_NODE_NAME,
        "bolt_stop_node": BOLT_STOP_NODE_NAME,
        "bolt_node_index": bolt_node_index,
        "bolt_stop_node_index": stop_node_index,
        "bolt_skin_index": memberships[0],
        "bolt_bind_translation": bind_translation,
        "bolt_weighted_vertex_count": 1317,
        "bolt_stop_weighted_vertex_count": 60,
        "bolt_stop_used_as_endpoint": False,
        "source_authored_animation_count": 0,
        "source_authored_endpoint": False,
        "derived_animation": DERIVED_ANIMATION_NAME,
        "derived_target_path": "translation",
        "derived_axis": "local_y",
        "derived_axis_basis": "audited_bolt_weighted_geometry_aabb_long_axis_candidate",
        "audited_bolt_aabb_extent": list(AUDITED_BOLT_AABB_EXTENT),
        "pilot_travel_fraction_of_y_extent": PILOT_TRAVEL_FRACTION_OF_Y_EXTENT,
        "pilot_max_travel": PILOT_MAX_TRAVEL,
        "pilot_travel_accepted": False,
        "rotation_channel_authored": False,
        "rotation_calibration_pending": True,
        "cycle_duration_seconds": CYCLE_DURATION,
        "key_times": list(KEY_TIMES),
        "key_offsets_y": list(KEY_OFFSETS_Y),
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
        "PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE_READY "
        f"source_sha256={source_digest} joint={BOLT_NODE_NAME} "
        f"animation={DERIVED_ANIMATION_NAME} duration={CYCLE_DURATION:.2f} "
        f"pilot_max_travel={PILOT_MAX_TRAVEL:.9f} source_authored_endpoint=0 "
        "bolt_stop_used_as_endpoint=0 pilot_travel_accepted=0 rotation_calibration_pending=1 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
