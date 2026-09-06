#!/usr/bin/env python3
"""Audit one exact remote Remington 870 GLB candidate without importing it into Oster.

This remains separate from RUN_ALL_VERIFY because the remote fetch is a networked
provenance/content probe. The local intake verifier pins this script and its candidate.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import urllib.request
from pathlib import Path

REPO = "Parking-Master/FPS"
COMMIT = "ed07ea542111c2149c5dab735e752824d0b0541c"
PATH = "models/weapons/shotgun.glb"
RAW_URL = f"https://raw.githubusercontent.com/{REPO}/{COMMIT}/{PATH}"
EXPECTED_GIT_BLOB_SHA1 = "f822d184d96ede43d79a6f691d69cbe7cf60e686"
EXPECTED_SIZE = 20621580

CONSUMER_PATH = "src.html"
CONSUMER_RAW_URL = f"https://raw.githubusercontent.com/{REPO}/{COMMIT}/{CONSUMER_PATH}"
EXPECTED_CONSUMER_GIT_BLOB_SHA1 = "18400e77c5b54b44e38dfd5cfd37a70efd19d43b"
EXPECTED_CONSUMER_SIZE = 125420

UPSTREAM_MODEL_ID = "eea11de7e9d24b6683962b8388c319eb"
UPSTREAM_CREATOR = "8sianDude"
LICENSE_ID = "CC-BY-4.0"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 REMOTE CANDIDATE AUDIT: FAIL\n[FAIL] {message}")


def git_blob_sha1(data: bytes) -> str:
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def fetch_url(url: str) -> bytes:
    request = urllib.request.Request(
        url,
        headers={"User-Agent": "OsterConflict-PASS45-Remington870-Audit/1"},
    )
    with urllib.request.urlopen(request, timeout=90) as response:
        if getattr(response, "status", 200) != 200:
            fail(f"remote HTTP status is {getattr(response, 'status', 'unknown')} for {url}")
        return response.read()


def fetch_bytes() -> bytes:
    return fetch_url(RAW_URL)


def verify_pinned_bytes(data: bytes) -> dict[str, str | int]:
    if len(data) != EXPECTED_SIZE:
        fail(f"size drift: expected {EXPECTED_SIZE}, got {len(data)}")
    actual_git_sha = git_blob_sha1(data)
    if actual_git_sha != EXPECTED_GIT_BLOB_SHA1:
        fail(f"Git blob identity drift: expected {EXPECTED_GIT_BLOB_SHA1}, got {actual_git_sha}")
    return {
        "size": len(data),
        "git_blob_sha1": actual_git_sha,
        "sha256": hashlib.sha256(data).hexdigest(),
    }


def verify_pinned_consumer_bytes(data: bytes) -> dict[str, str | int]:
    if len(data) != EXPECTED_CONSUMER_SIZE:
        fail(f"consumer size drift: expected {EXPECTED_CONSUMER_SIZE}, got {len(data)}")
    actual_git_sha = git_blob_sha1(data)
    if actual_git_sha != EXPECTED_CONSUMER_GIT_BLOB_SHA1:
        fail(
            "consumer Git blob identity drift: "
            f"expected {EXPECTED_CONSUMER_GIT_BLOB_SHA1}, got {actual_git_sha}"
        )
    return {
        "consumer_size": len(data),
        "consumer_git_blob_sha1": actual_git_sha,
        "consumer_sha256": hashlib.sha256(data).hexdigest(),
    }


def parse_glb_json(data: bytes) -> dict:
    if len(data) < 20:
        fail("GLB is too short")
    magic, version, declared_length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF":
        fail(f"unexpected GLB magic {magic!r}")
    if version != 2:
        fail(f"expected glTF 2.0 binary, got version {version}")
    if declared_length != len(data):
        fail(f"GLB declared length {declared_length} != actual {len(data)}")

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


def structural_summary(doc: dict) -> dict[str, int]:
    animations = doc.get("animations") or []
    skins = doc.get("skins") or []
    nodes = doc.get("nodes") or []
    meshes = doc.get("meshes") or []

    def channels(index: int) -> int:
        if index >= len(animations):
            return 0
        return len(animations[index].get("channels") or [])

    return {
        "animations": len(animations),
        "skins": len(skins),
        "nodes": len(nodes),
        "meshes": len(meshes),
        "fire_channels": channels(2),
        "easy_reload_channels": channels(3),
        "full_reload_channels": channels(4),
    }


def require_animation_contract(doc: dict) -> None:
    animations = doc.get("animations") or []
    if len(animations) < 5:
        fail(f"source consumer requires animation indices 0..4 but GLB exposes only {len(animations)} animations")
    for index, semantic in ((2, "fire"), (3, "easy_reload"), (4, "full_reload")):
        clip = animations[index]
        if not (clip.get("channels") or []) or not (clip.get("samplers") or []):
            fail(f"animation index {index} ({semantic}) is empty")


def require_skin(doc: dict) -> None:
    skins = doc.get("skins") or []
    if len(skins) < 1:
        fail("candidate has no glTF skin; direct current skeletal/manual-action bridge compatibility is unproven")


def between(text: str, start_marker: str, end_marker: str) -> str:
    start = text.find(start_marker)
    if start < 0:
        fail(f"consumer source lost required marker: {start_marker}")
    end = text.find(end_marker, start + len(start_marker))
    if end < 0:
        fail(f"consumer source lost required end marker: {end_marker}")
    return text[start:end]


def require_consumer_contract(data: bytes) -> dict[str, object]:
    identity = verify_pinned_consumer_bytes(data)
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as exc:
        fail(f"consumer source is not UTF-8: {exc}")

    for needle in (
        'weaponDefs = ["rifle", "pistol", "sniper", "assault", "uzi", "shotgun", "rocket", "railgun"];',
        'gunsAnimations[i] = gun.animations;',
        'mixers[i] = new THREE.AnimationMixer(gun.scene);',
        '"shotgun": "Remington 870"',
    ):
        if needle not in text:
            fail(f"consumer contract drifted; missing: {needle}")

    reload_section = between(text, "function reload() {", "function blowUpWarthog")
    fire_section = between(text, "function fire(", "let hasSwitchedFirst")

    fire_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][2]"
    easy_reload_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][3]"
    full_reload_expr = "gunsAnimations[weaponDefs.indexOf(currentWeapon)][4]"

    if fire_expr not in fire_section:
        fail("consumer fire() no longer invokes donor animation index 2")
    if easy_reload_expr not in reload_section:
        fail("consumer reload() no longer invokes donor animation index 3 for ordinary/easy reload")
    if full_reload_expr not in reload_section:
        fail("consumer reload() no longer invokes donor animation index 4 for full/empty reload")
    if easy_reload_expr in fire_section or full_reload_expr in fire_section:
        fail("consumer fire() unexpectedly invokes a reload clip")
    if fire_expr in reload_section:
        fail("consumer reload() unexpectedly invokes the fire clip")

    shotgun_trigger = re.search(
        r'else if \(currentWeapon == "shotgun"\) \{\s*fire\(1,\s*36\);\s*\}',
        text,
    )
    if shotgun_trigger is None:
        fail("shotgun trigger no longer routes through the shared fire() consumer path")

    if re.search(r"\bpump\b", text, flags=re.IGNORECASE):
        fail("pinned consumer unexpectedly gained an explicit pump-named invocation/path; re-audit semantics")

    dynamic_indices = {
        int(match)
        for match in re.findall(
            r"gunsAnimations\[weaponDefs\.indexOf\(currentWeapon\)\]\[(\d+)\]",
            text,
        )
    }
    if not {0, 1, 2, 3, 4}.issubset(dynamic_indices):
        fail(f"consumer dynamic animation index set drifted: {sorted(dynamic_indices)}")
    if any(index > 4 for index in dynamic_indices):
        fail(f"consumer invokes an animation index outside the five-clip donor contract: {sorted(dynamic_indices)}")

    return {
        **identity,
        "consumer_fire_clip_index": 2,
        "consumer_easy_reload_clip_index": 3,
        "consumer_full_reload_clip_index": 4,
        "shotgun_fire_routes_shared_fire": 1,
        "separate_consumer_pump_invocation": 0,
        "fire_clip_internal_pump_phase": "UNPROVEN",
        "pump_node_identity": "UNPROVEN",
        "ue58_import_pending": 1,
        "runtime_acceptance": 0,
        "item16_checked": 0,
    }


def write_github_outputs(values: dict[str, object]) -> None:
    output_path = os.environ.get("GITHUB_OUTPUT")
    if not output_path:
        return
    with open(output_path, "a", encoding="utf-8") as handle:
        for key, value in values.items():
            handle.write(f"{key}={value}\n")


def load_local(path: str) -> tuple[bytes, dict]:
    data = Path(path).read_bytes()
    verify_pinned_bytes(data)
    return data, parse_glb_json(data)


def main() -> None:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)

    audit = sub.add_parser("audit")
    audit.add_argument("--output", default="remington870_remote_candidate.glb")

    sub.add_parser("audit-consumer-contract")

    animation = sub.add_parser("require-animation-contract")
    animation.add_argument("input")

    skin = sub.add_parser("require-skin")
    skin.add_argument("input")

    args = parser.parse_args()

    if args.command == "audit":
        data = fetch_bytes()
        identity = verify_pinned_bytes(data)
        doc = parse_glb_json(data)
        structure = structural_summary(doc)
        Path(args.output).write_bytes(data)
        summary = {
            **identity,
            **structure,
            "repo": REPO,
            "commit": COMMIT,
            "path": PATH,
            "upstream_model_id": UPSTREAM_MODEL_ID,
            "upstream_creator": UPSTREAM_CREATOR,
            "license_id": LICENSE_ID,
        }
        write_github_outputs(summary)
        print("PASS45 REMINGTON870 REMOTE CANDIDATE AUDIT: INSPECTED")
        print(json.dumps(summary, indent=2, sort_keys=True))
        return

    if args.command == "audit-consumer-contract":
        consumer = fetch_url(CONSUMER_RAW_URL)
        summary = require_consumer_contract(consumer)
        write_github_outputs(summary)
        print("PASS45 REMINGTON870 TRANSPORT CONSUMER CONTRACT: PASS")
        print(json.dumps(summary, indent=2, sort_keys=True))
        return

    _, doc = load_local(args.input)
    if args.command == "require-animation-contract":
        require_animation_contract(doc)
        print("PASS45 REMINGTON870 REMOTE CANDIDATE: ANIMATION CONTRACT PASS")
        return
    if args.command == "require-skin":
        require_skin(doc)
        print("PASS45 REMINGTON870 REMOTE CANDIDATE: SKIN CONTRACT PASS")
        return


if __name__ == "__main__":
    main()
