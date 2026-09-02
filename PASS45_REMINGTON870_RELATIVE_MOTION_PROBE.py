#!/usr/bin/env python3
"""Measure Pmag_061 motion relative to PBody_058 from one exact local donor GLB.

This probe has no network/import/production authority. It consumes bytes already
fetched and pinned by PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT.py, reuses the
existing binary animation helpers, and keeps pump identity/runtime acceptance
fail-closed.
"""
from __future__ import annotations

import argparse
import json
import math
import os
from pathlib import Path

import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote
import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire

EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"
ACTION_CLIPS = ((2, "fire"), (3, "easy_reload"), (4, "full_reload"))
PBODY_NAME = "PBody_058"
PMAG_NAME = "Pmag_061"
EXPECTED_SHARED_PARENT = "Root_01"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 RELATIVE MOTION PROBE: FAIL\n[FAIL] {message}")


def node_index_by_name(nodes: list[dict], name: str) -> int:
    matches = [index for index, node in enumerate(nodes) if str(node.get("name") or "") == name]
    if len(matches) != 1:
        fail(f"expected exactly one node named {name}, found {matches}")
    return matches[0]


def parent_map(nodes: list[dict]) -> dict[int, int]:
    parents: dict[int, int] = {}
    for parent_index, node in enumerate(nodes):
        for child in node.get("children") or []:
            if not isinstance(child, int) or child < 0 or child >= len(nodes):
                fail(f"node {parent_index} has invalid child {child!r}")
            if child in parents:
                fail(f"node {child} has multiple parents")
            parents[child] = parent_index
    return parents


def translation_track(
    doc: dict,
    binary_payload: bytes,
    animation_index: int,
    node_index: int,
) -> tuple[list[float], list[tuple[float, ...]]]:
    animations = doc.get("animations") or []
    if animation_index < 0 or animation_index >= len(animations):
        fail(f"missing animation index {animation_index}")
    animation = animations[animation_index]
    samplers = animation.get("samplers") or []

    matches: list[dict] = []
    for channel in animation.get("channels") or []:
        target = channel.get("target") or {}
        if target.get("node") == node_index and target.get("path") == "translation":
            matches.append(channel)
    if len(matches) != 1:
        fail(
            f"animation {animation_index} node {node_index} expected one translation channel, "
            f"found {len(matches)}"
        )

    sampler_index = matches[0].get("sampler")
    if not isinstance(sampler_index, int) or sampler_index < 0 or sampler_index >= len(samplers):
        fail(f"animation {animation_index} node {node_index} has invalid sampler")
    sampler = samplers[sampler_index]
    input_index = sampler.get("input")
    if not isinstance(input_index, int):
        fail(f"animation {animation_index} node {node_index} sampler has no input accessor")

    raw_times = acquire.accessor_values(doc, binary_payload, input_index)
    values = acquire.sampled_animation_output(doc, binary_payload, sampler)
    if len(raw_times) != len(values):
        fail(
            f"animation {animation_index} node {node_index} key/value count mismatch "
            f"{len(raw_times)} != {len(values)}"
        )
    times: list[float] = []
    for row in raw_times:
        if len(row) != 1:
            fail(f"animation {animation_index} node {node_index} input accessor is not scalar")
        times.append(float(row[0]))
    if any(times[index] > times[index + 1] for index in range(len(times) - 1)):
        fail(f"animation {animation_index} node {node_index} key times are not monotonic")
    if not values or len(values[0]) != 3:
        fail(f"animation {animation_index} node {node_index} translation output is not VEC3")
    return times, values


def relative_translation_row(
    doc: dict,
    binary_payload: bytes,
    animation_index: int,
    pbody_index: int,
    pmag_index: int,
) -> dict[str, object]:
    pbody_times, pbody_values = translation_track(doc, binary_payload, animation_index, pbody_index)
    pmag_times, pmag_values = translation_track(doc, binary_payload, animation_index, pmag_index)
    if len(pbody_times) != len(pmag_times):
        fail(
            f"animation {animation_index} sibling key counts differ: "
            f"PBody={len(pbody_times)} Pmag={len(pmag_times)}"
        )
    for key_index, (pbody_time, pmag_time) in enumerate(zip(pbody_times, pmag_times)):
        if abs(pbody_time - pmag_time) > 1e-6:
            fail(
                f"animation {animation_index} sibling key time drift at {key_index}: "
                f"{pbody_time} != {pmag_time}"
            )

    relative = [
        tuple(pmag[axis] - pbody[axis] for axis in range(3))
        for pbody, pmag in zip(pbody_values, pmag_values)
    ]
    origin = relative[0]
    deltas = [
        tuple(sample[axis] - origin[axis] for axis in range(3))
        for sample in relative
    ]
    delta_min = [min(sample[axis] for sample in deltas) for axis in range(3)]
    delta_max = [max(sample[axis] for sample in deltas) for axis in range(3)]
    axis_range = [delta_max[axis] - delta_min[axis] for axis in range(3)]
    peak = max(math.sqrt(sum(component * component for component in sample)) for sample in deltas)
    dominant_axis_index = max(range(3), key=lambda axis: axis_range[axis])

    return {
        "key_times_aligned": True,
        "sample_count": len(relative),
        "start_time": round(pbody_times[0], 6),
        "end_time": round(pbody_times[-1], 6),
        "initial_relative_translation": [round(component, 6) for component in origin],
        "delta_min": [round(component, 6) for component in delta_min],
        "delta_max": [round(component, 6) for component in delta_max],
        "axis_range": [round(component, 6) for component in axis_range],
        "dominant_axis": ("X", "Y", "Z")[dominant_axis_index],
        "peak_relative_displacement": round(peak, 6),
    }


def write_outputs(values: dict[str, object]) -> None:
    output_path = os.environ.get("GITHUB_OUTPUT")
    if not output_path:
        return
    with open(output_path, "a", encoding="utf-8") as handle:
        for key, value in values.items():
            handle.write(f"{key}={value}\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    args = parser.parse_args()

    data = Path(args.input).read_bytes()
    identity = remote.verify_pinned_bytes(data)
    if identity.get("sha256") != EXPECTED_SHA256:
        fail(f"unexpected donor SHA-256 {identity.get('sha256')}")

    doc = remote.parse_glb_json(data)
    remote.require_animation_contract(doc)
    remote.require_skin(doc)
    binary_payload = acquire.glb_binary_chunk(data)
    nodes = doc.get("nodes") or []
    if not isinstance(nodes, list):
        fail("GLB nodes collection missing")

    pbody_index = node_index_by_name(nodes, PBODY_NAME)
    pmag_index = node_index_by_name(nodes, PMAG_NAME)
    parents = parent_map(nodes)
    pbody_parent = parents.get(pbody_index)
    pmag_parent = parents.get(pmag_index)
    if pbody_parent is None or pmag_parent is None or pbody_parent != pmag_parent:
        fail(
            f"{PBODY_NAME} and {PMAG_NAME} are not siblings under one parent: "
            f"{pbody_parent} vs {pmag_parent}"
        )
    shared_parent_name = str(nodes[pbody_parent].get("name") or "")
    if shared_parent_name != EXPECTED_SHARED_PARENT:
        fail(f"shared parent drifted: expected {EXPECTED_SHARED_PARENT}, got {shared_parent_name!r}")

    rows: dict[str, dict[str, object]] = {}
    for animation_index, semantic in ACTION_CLIPS:
        rows[f"{semantic}_index_{animation_index}"] = relative_translation_row(
            doc,
            binary_payload,
            animation_index,
            pbody_index,
            pmag_index,
        )

    fire_peak = float(rows["fire_index_2"]["peak_relative_displacement"])
    easy_peak = float(rows["easy_reload_index_3"]["peak_relative_displacement"])
    full_peak = float(rows["full_reload_index_4"]["peak_relative_displacement"])
    if easy_peak <= 0.0 or full_peak <= 0.0:
        fail("reload relative displacement unexpectedly vanished")

    summary: dict[str, object] = {
        "source_sha256": identity["sha256"],
        "shared_parent": shared_parent_name,
        "pbody_node_index": pbody_index,
        "pmag_node_index": pmag_index,
        "fire_relative_peak": f"{fire_peak:.6f}",
        "easy_reload_relative_peak": f"{easy_peak:.6f}",
        "full_reload_relative_peak": f"{full_peak:.6f}",
        "fire_to_easy_reload_peak_ratio": f"{fire_peak / easy_peak:.6f}",
        "fire_to_full_reload_peak_ratio": f"{fire_peak / full_peak:.6f}",
        "sibling_relative_translation_measured": 1,
        "pump_node_identity": "UNPROVEN",
        "standalone_pump_clip": "UNPROVEN",
        "fire_clip_internal_pump_phase": "UNPROVEN",
        "ue58_import_pending": 1,
        "runtime_acceptance": 0,
        "item16_checked": 0,
    }
    write_outputs(summary)

    report = {
        **summary,
        "action_relative_translation": rows,
    }
    print("PASS45 REMINGTON870 RELATIVE MOTION PROBE: PASS")
    print(json.dumps(report, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
