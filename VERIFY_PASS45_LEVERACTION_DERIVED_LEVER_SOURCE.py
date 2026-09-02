#!/usr/bin/env python3
"""Fail-closed verifier for the PASS45 Lever Action derived calibration source."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

EXPECTED_SOURCE_SHA256 = "b2bf25bd47e9c4f6404897f67ad2a76a02971365fb7a689761936891d4591c69"
EXPECTED_STATUS = "LEVERACTION_DERIVED_CALIBRATION_PILOT_ONLY"
EXPECTED_ANIMATION = "PASS45_LeverAction_Cycle"
EXPECTED_DURATION = 0.85
EXPECTED_ANGLE_DEG = -45.0
EXPECTED_WEIGHTED_VERTICES = 964
EXPECTED_TRANSLATION = [-8.145542018667129e-10, -0.14167749881744385, -0.012812890112400055]


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 LEVERACTION DERIVED LEVER VERIFY: FAIL\n[FAIL] {message}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_accessor(doc: dict, payload: bytes, accessor_index: int) -> list[tuple[float, ...]]:
    accessors = doc.get("accessors") or []
    views = doc.get("bufferViews") or []
    if not (0 <= accessor_index < len(accessors)):
        fail(f"invalid accessor {accessor_index}")
    accessor = accessors[accessor_index]
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or not (0 <= view_index < len(views)):
        fail(f"accessor {accessor_index} missing bufferView")
    view = views[view_index]
    if view.get("buffer", 0) != 0:
        fail("derived verifier supports only buffer 0")
    if accessor.get("componentType") != 5126:
        fail(f"accessor {accessor_index} is not FLOAT")
    components = {"SCALAR": 1, "VEC4": 4}.get(accessor.get("type"))
    if components is None:
        fail(f"accessor {accessor_index} has unexpected type {accessor.get('type')}")
    count = int(accessor.get("count", 0))
    packed = components * 4
    stride = int(view.get("byteStride", packed))
    base = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    rows: list[tuple[float, ...]] = []
    for row in range(count):
        offset = base + row * stride
        if offset + packed > len(payload):
            fail(f"accessor {accessor_index} overruns BIN")
        rows.append(tuple(float(v) for v in struct.unpack_from("<" + "f" * components, payload, offset)))
    return rows


def quat_x_angle_deg(quat: tuple[float, ...]) -> float:
    if len(quat) != 4:
        fail("rotation accessor is not VEC4")
    x, y, z, w = quat
    if abs(y) > 1e-5 or abs(z) > 1e-5:
        fail(f"derived lever rotation left local-X axis: {quat}")
    norm = math.sqrt(x * x + y * y + z * z + w * w)
    if abs(norm - 1.0) > 1e-4:
        fail(f"non-unit quaternion: {quat} norm={norm}")
    return math.degrees(2.0 * math.atan2(x, w))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gltf", required=True)
    parser.add_argument("--manifest", required=True)
    parser.add_argument("--audit", required=True)
    args = parser.parse_args()

    gltf_path = Path(args.gltf)
    manifest_path = Path(args.manifest)
    audit_path = Path(args.audit)
    for path in (gltf_path, manifest_path, audit_path):
        if not path.is_file():
            fail(f"missing input: {path}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    audit = json.loads(audit_path.read_text(encoding="utf-8"))
    doc = json.loads(gltf_path.read_text(encoding="utf-8"))

    if manifest.get("status") != EXPECTED_STATUS:
        fail(f"manifest status drifted: {manifest.get('status')}")
    if manifest.get("source_sha256") != EXPECTED_SOURCE_SHA256:
        fail("manifest source identity drifted")
    if abs(float(manifest.get("cycle_duration_seconds", -1.0)) - EXPECTED_DURATION) > 1e-6:
        fail("manifest duration drifted")
    if abs(float(manifest.get("pilot_max_angle_deg", 999.0)) - EXPECTED_ANGLE_DEG) > 1e-6:
        fail("manifest pilot angle drifted")
    if manifest.get("source_authored_endpoint") is not False:
        fail("derived pilot falsely claims source-authored endpoint")
    if manifest.get("pilot_angle_accepted") is not False:
        fail("calibration angle must remain unaccepted before direct UE visual proof")
    if manifest.get("production_cutover") is not False or manifest.get("runtime_acceptance") is not False:
        fail("derived pilot falsely claims production/runtime acceptance")
    if manifest.get("item16_checked") is not False:
        fail("item 16 must remain open")
    if manifest.get("derived_gltf_sha256") != sha256(gltf_path):
        fail("derived glTF SHA-256 mismatch")

    if audit.get("source_sha256") != EXPECTED_SOURCE_SHA256:
        fail("source audit identity drifted")
    if audit.get("animation_count") != 0:
        fail("source audit no longer proves animation_count=0")
    if audit.get("source_authored_lever_angle_or_endpoint") is not False:
        fail("source audit unexpectedly claims authored lever endpoint")
    if audit.get("lever_joint_has_weighted_geometry") is not True:
        fail("source audit no longer proves weighted LEVER geometry")
    geometry_rows = audit.get("mechanical_joint_geometry") or []
    lever_rows = [row for row in geometry_rows if row.get("node_name") == "LEVER"]
    if len(lever_rows) != 1:
        fail(f"source audit LEVER geometry row count={len(lever_rows)}")
    if int(lever_rows[0].get("weighted_vertex_count", -1)) != EXPECTED_WEIGHTED_VERTICES:
        fail(f"LEVER weighted vertex count drifted: {lever_rows[0].get('weighted_vertex_count')}")

    buffers = doc.get("buffers") or []
    if len(buffers) != 1:
        fail(f"derived glTF buffer count={len(buffers)}")
    uri = buffers[0].get("uri")
    if not isinstance(uri, str) or not uri:
        fail("derived buffer URI missing")
    bin_path = (gltf_path.parent / uri).resolve()
    if not bin_path.is_file():
        fail(f"derived BIN missing: {bin_path}")
    if int(buffers[0].get("byteLength", -1)) != bin_path.stat().st_size:
        fail("derived BIN byteLength mismatch")
    if manifest.get("derived_bin_sha256") != sha256(bin_path):
        fail("derived BIN SHA-256 mismatch")

    nodes = doc.get("nodes") or []
    lever_indices = [index for index, node in enumerate(nodes) if node.get("name") == "LEVER"]
    if len(lever_indices) != 1:
        fail(f"derived LEVER node count={len(lever_indices)}")
    lever_index = lever_indices[0]
    lever = nodes[lever_index]
    if "matrix" in lever:
        fail("animated LEVER node still uses matrix instead of TRS")
    translation = [float(value) for value in (lever.get("translation") or [])]
    if len(translation) != 3 or any(abs(a - b) > 1e-6 for a, b in zip(translation, EXPECTED_TRANSLATION)):
        fail(f"LEVER bind translation drifted: {translation}")
    if [float(value) for value in (lever.get("rotation") or [])] != [0.0, 0.0, 0.0, 1.0]:
        fail("LEVER bind rotation is not identity after matrix-to-TRS conversion")

    animations = doc.get("animations") or []
    if len(animations) != 1:
        fail(f"derived animation count={len(animations)}")
    animation = animations[0]
    if animation.get("name") != EXPECTED_ANIMATION:
        fail(f"derived animation identity drifted: {animation.get('name')}")
    channels = animation.get("channels") or []
    samplers = animation.get("samplers") or []
    if len(channels) != 1 or len(samplers) != 1:
        fail("derived animation must contain exactly one channel/sampler")
    target = channels[0].get("target") or {}
    if target.get("node") != lever_index or target.get("path") != "rotation":
        fail(f"derived channel targets unexpected object: {target}")
    if channels[0].get("sampler") != 0:
        fail("derived channel sampler index drifted")
    sampler = samplers[0]
    if sampler.get("interpolation") != "LINEAR":
        fail("derived animation interpolation drifted")
    time_accessor = sampler.get("input")
    rot_accessor = sampler.get("output")
    if not isinstance(time_accessor, int) or not isinstance(rot_accessor, int):
        fail("derived sampler accessors missing")
    times = [row[0] for row in read_accessor(doc, bin_path.read_bytes(), time_accessor)]
    quats = read_accessor(doc, bin_path.read_bytes(), rot_accessor)
    if len(times) != len(quats) or len(times) < 3:
        fail("derived key counts are invalid")
    if abs(times[0]) > 1e-6 or abs(times[-1] - EXPECTED_DURATION) > 1e-5:
        fail(f"derived duration keys drifted: first={times[0]} last={times[-1]}")
    if any(right <= left for left, right in zip(times, times[1:])):
        fail(f"derived key times are not strictly increasing: {times}")

    angles = [quat_x_angle_deg(quat) for quat in quats]
    if abs(angles[0]) > 1e-4 or abs(angles[-1]) > 1e-4:
        fail(f"derived cycle does not return to bind pose: angles={angles}")
    if abs(min(angles) - EXPECTED_ANGLE_DEG) > 0.05:
        fail(f"derived pilot max angle drifted: angles={angles}")
    if max(abs(angle) for angle in angles) < 10.0:
        fail("derived lever motion is trivial")

    extras = ((doc.get("asset") or {}).get("extras") or {}).get("PASS45") or {}
    if extras.get("classification") != EXPECTED_STATUS:
        fail("glTF PASS45 classification drifted")
    if extras.get("source_authored_endpoint") is not False or extras.get("pilot_angle_accepted") is not False:
        fail("glTF metadata falsely promotes calibration motion")
    if extras.get("second_gameplay_timer") is not False:
        fail("derived presentation must not own a second gameplay timer")

    print("PASS45_LEVERACTION_DERIVED_LEVER_SOURCE_VERIFY: PASS")
    print(f"- LEVER weighted vertices: {EXPECTED_WEIGHTED_VERTICES}")
    print(f"- standalone sequence: {EXPECTED_ANIMATION}, {EXPECTED_DURATION:.2f}s")
    print(f"- local-X pilot excursion: {EXPECTED_ANGLE_DEG:.1f}deg, returns to bind pose")
    print("- source-authored endpoint: false; direct UE visual calibration still required")
    print("production_cutover=0 runtime_acceptance=0 item16_checked=0")


if __name__ == "__main__":
    main()
