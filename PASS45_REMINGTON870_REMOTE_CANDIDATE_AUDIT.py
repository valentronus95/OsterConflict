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
import struct
import urllib.request
from pathlib import Path

REPO = "Parking-Master/FPS"
COMMIT = "ed07ea542111c2149c5dab735e752824d0b0541c"
PATH = "models/weapons/shotgun.glb"
RAW_URL = f"https://raw.githubusercontent.com/{REPO}/{COMMIT}/{PATH}"
EXPECTED_GIT_BLOB_SHA1 = "f822d184d96ede43d79a6f691d69cbe7cf60e686"
EXPECTED_SIZE = 20621580
UPSTREAM_MODEL_ID = "eea11de7e9d24b6683962b8388c319eb"
UPSTREAM_CREATOR = "8sianDude"
LICENSE_ID = "CC-BY-4.0"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 REMOTE CANDIDATE AUDIT: FAIL\n[FAIL] {message}")


def git_blob_sha1(data: bytes) -> str:
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def fetch_bytes() -> bytes:
    request = urllib.request.Request(
        RAW_URL,
        headers={"User-Agent": "OsterConflict-PASS45-Remington870-Audit/1"},
    )
    with urllib.request.urlopen(request, timeout=90) as response:
        if getattr(response, "status", 200) != 200:
            fail(f"remote HTTP status is {getattr(response, 'status', 'unknown')}")
        return response.read()


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
