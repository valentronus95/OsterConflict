#!/usr/bin/env python3
"""Fail-closed verifier for the M700 derived bolt translation calibration source."""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

import PASS45_M700_DERIVED_BOLT_TRANSLATION_SOURCE as source_contract


def fail(message: str) -> None:
    raise SystemExit(f"VERIFY PASS45 M700 DERIVED BOLT TRANSLATION: FAIL\n[FAIL] {message}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_accessor(doc: dict, binary: bytes, accessor_index: int) -> list[tuple[float, ...]]:
    accessors = doc.get("accessors") or []
    views = doc.get("bufferViews") or []
    if not 0 <= accessor_index < len(accessors):
        fail(f"invalid accessor index {accessor_index}")
    accessor = accessors[accessor_index]
    view_index = accessor.get("bufferView")
    if not isinstance(view_index, int) or not 0 <= view_index < len(views):
        fail(f"accessor {accessor_index} lacks valid bufferView")
    view = views[view_index]
    if view.get("buffer", 0) != 0:
        fail("derived accessor references nonzero buffer")
    if accessor.get("componentType") != 5126:
        fail("derived animation accessor is not FLOAT")
    widths = {"SCALAR": 1, "VEC3": 3}
    width = widths.get(accessor.get("type"))
    if width is None:
        fail(f"unsupported accessor type {accessor.get('type')!r}")
    count = int(accessor.get("count", -1))
    stride = int(view.get("byteStride", width * 4))
    if stride < width * 4:
        fail("invalid accessor byteStride")
    base = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    fmt = "<" + "f" * width
    rows: list[tuple[float, ...]] = []
    for index in range(count):
        offset = base + index * stride
        if offset < 0 or offset + width * 4 > len(binary):
            fail("accessor overruns derived BIN")
        rows.append(tuple(float(v) for v in struct.unpack_from(fmt, binary, offset)))
    return rows


def close(a: float, b: float, tolerance: float = 1e-6) -> bool:
    return abs(float(a) - float(b)) <= tolerance


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gltf", required=True)
    parser.add_argument("--manifest", required=True)
    parser.add_argument("--motion-audit", required=True)
    parser.add_argument("--geometry-audit", required=True)
    args = parser.parse_args()

    gltf_path = Path(args.gltf)
    manifest_path = Path(args.manifest)
    motion_audit_path = Path(args.motion_audit)
    geometry_audit_path = Path(args.geometry_audit)
    for path, label in (
        (gltf_path, "derived glTF"),
        (manifest_path, "manifest"),
        (motion_audit_path, "motion audit"),
        (geometry_audit_path, "geometry audit"),
    ):
        if not path.is_file():
            fail(f"{label} missing: {path}")

    doc = json.loads(gltf_path.read_text(encoding="utf-8"))
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    motion = json.loads(motion_audit_path.read_text(encoding="utf-8"))
    geometry = json.loads(geometry_audit_path.read_text(encoding="utf-8"))
    source_contract.validate_audits(motion, geometry)

    if manifest.get("status") != "M700_BOLT_TRANSLATION_CALIBRATION_PILOT_ONLY":
        fail(f"manifest status drifted: {manifest.get('status')!r}")
    if manifest.get("source_sha256") != source_contract.EXPECTED_SOURCE_SHA256:
        fail("manifest source SHA drifted")
    if manifest.get("source_authored_endpoint") is not False:
        fail("manifest falsely claims a source-authored endpoint")
    if manifest.get("bolt_stop_used_as_endpoint") is not False:
        fail("manifest must not use BOLT_STOP as a travel endpoint")
    if manifest.get("pilot_travel_accepted") is not False:
        fail("calibration pilot travel must remain unaccepted")
    if manifest.get("rotation_channel_authored") is not False:
        fail("translation pilot must not claim a rotation channel")
    if manifest.get("rotation_calibration_pending") is not True:
        fail("M700 rotation calibration must remain pending")
    if manifest.get("production_cutover") is not False:
        fail("manifest falsely claims production cutover")
    if manifest.get("runtime_acceptance") is not False:
        fail("manifest falsely claims runtime acceptance")
    if manifest.get("item16_checked") is not False:
        fail("manifest falsely closes item 16")

    bin_name = manifest.get("derived_bin")
    if not isinstance(bin_name, str):
        fail("manifest missing derived BIN name")
    bin_path = gltf_path.parent / bin_name
    if not bin_path.is_file():
        fail(f"derived BIN missing: {bin_path}")
    if sha256(gltf_path) != manifest.get("derived_gltf_sha256"):
        fail("derived glTF SHA mismatch")
    if sha256(bin_path) != manifest.get("derived_bin_sha256"):
        fail("derived BIN SHA mismatch")

    buffers = doc.get("buffers") or []
    if len(buffers) != 1 or buffers[0].get("uri") != bin_name:
        fail("derived glTF buffer contract drifted")
    binary = bin_path.read_bytes()
    if int(buffers[0].get("byteLength", -1)) != len(binary):
        fail("derived BIN byteLength mismatch")

    nodes = doc.get("nodes") or []
    bolt_matches = [
        i for i, node in enumerate(nodes)
        if str(node.get("name") or "") == source_contract.BOLT_NODE_NAME
    ]
    stop_matches = [
        i for i, node in enumerate(nodes)
        if str(node.get("name") or "") == source_contract.BOLT_STOP_NODE_NAME
    ]
    if len(bolt_matches) != 1 or len(stop_matches) != 1:
        fail("derived BOLT/BOLT_STOP identities are not unique")
    bolt_index = bolt_matches[0]
    stop_index = stop_matches[0]
    if bolt_index != int(manifest.get("bolt_node_index", -1)):
        fail("manifest BOLT node index drifted")
    if stop_index != int(manifest.get("bolt_stop_node_index", -1)):
        fail("manifest BOLT_STOP node index drifted")

    animations = doc.get("animations") or []
    if len(animations) != 1:
        fail(f"expected exactly one derived animation, got {len(animations)}")
    animation = animations[0]
    if animation.get("name") != source_contract.DERIVED_ANIMATION_NAME:
        fail(f"derived animation identity drifted: {animation.get('name')!r}")
    channels = animation.get("channels") or []
    samplers = animation.get("samplers") or []
    if len(channels) != 1 or len(samplers) != 1:
        fail("derived translation pilot must have one sampler/channel")
    target = channels[0].get("target") or {}
    if target.get("node") != bolt_index or target.get("path") != "translation":
        fail(f"derived channel target drifted: {target!r}")
    if target.get("node") == stop_index:
        fail("BOLT_STOP must not be animated by the calibration pilot")

    sampler = samplers[0]
    if sampler.get("interpolation") != "LINEAR":
        fail("derived sampler interpolation drifted")
    time_index = sampler.get("input")
    value_index = sampler.get("output")
    if not isinstance(time_index, int) or not isinstance(value_index, int):
        fail("derived sampler accessors are invalid")

    times = [row[0] for row in read_accessor(doc, binary, time_index)]
    translations = read_accessor(doc, binary, value_index)
    if len(times) != len(source_contract.KEY_TIMES) or len(translations) != len(times):
        fail("derived key count drifted")
    for actual, expected in zip(times, source_contract.KEY_TIMES):
        if not close(actual, expected, 2e-6):
            fail(f"derived key time drifted: actual={actual} expected={expected}")

    bind = tuple(float(v) for v in manifest.get("bolt_bind_translation", []))
    if len(bind) != 3:
        fail("manifest BOLT bind translation missing")
    expected_offsets = source_contract.KEY_OFFSETS_Y
    max_delta = 0.0
    for index, (value, expected_offset) in enumerate(zip(translations, expected_offsets)):
        if not close(value[0], bind[0], 2e-6) or not close(value[2], bind[2], 2e-6):
            fail(f"pilot moved BOLT outside local Y at key {index}: {value}")
        actual_offset = value[1] - bind[1]
        if not close(actual_offset, expected_offset, 2e-6):
            fail(
                f"pilot Y offset drifted at key {index}: "
                f"actual={actual_offset} expected={expected_offset}"
            )
        max_delta = max(max_delta, abs(actual_offset))

    if max_delta <= 0.01:
        fail(f"derived BOLT translation is trivial: {max_delta}")
    if not close(max_delta, source_contract.PILOT_MAX_TRAVEL, 2e-6):
        fail(f"derived pilot travel drifted: {max_delta}")
    if any(
        not close(translations[-1][axis], translations[0][axis], 2e-6)
        for axis in range(3)
    ):
        fail("derived bolt does not return to the bind translation")

    extras = ((doc.get("asset") or {}).get("extras") or {}).get("PASS45") or {}
    required_extras = {
        "classification": "M700_BOLT_TRANSLATION_CALIBRATION_PILOT_ONLY",
        "source_authored_endpoint": False,
        "bolt_stop_used_as_endpoint": False,
        "pilot_travel_accepted": False,
        "rotation_channel_authored": False,
        "rotation_calibration_pending": True,
        "runtime_acceptance": False,
        "item16_checked": False,
    }
    for key, expected in required_extras.items():
        if extras.get(key) != expected:
            fail(f"derived PASS45 extras drifted: {key}={extras.get(key)!r}")

    print(
        "VERIFY_PASS45_M700_DERIVED_BOLT_TRANSLATION_PASS "
        f"animation={source_contract.DERIVED_ANIMATION_NAME} "
        f"max_translation_delta={max_delta:.9f} "
        "bolt_stop_animated=0 source_authored_endpoint=0 pilot_travel_accepted=0 "
        "rotation_calibration_pending=1 production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
