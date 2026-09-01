#!/usr/bin/env python3
"""Audit/acquire Pass45 CC0 manual-action audio donors without overclaiming runtime readiness."""

from __future__ import annotations

import argparse
import hashlib
import html
import json
import os
import re
import subprocess
import sys
import urllib.request
import wave
from pathlib import Path

USER_AGENT = "OsterConflict-Pass45-AudioIntake/1.0 (+https://github.com/valentronus95/OsterConflict)"

DONORS = {
    "lever": {
        "source_page": "https://freesound.org/people/C-V/sounds/523401/",
        "source_id": "523401",
        "source_title": "Lever action cocking.wav",
        "source_author": "C-V",
        "source_duration_s": 0.958,
        "source_family": "real .22 lever-action rifle action",
        "mirror_pages": [
            "https://pixabay.com/sound-effects/household-086318-lever-action-cockingwav-39666/",
        ],
        "output_name": "lever_action_cc0_preview_donor.wav",
        "identity_scope": "lever-action-family donor; not exact Stein/Marlin/Model-1894 identity",
        "expected_transport_sha256": "",
    },
    "bolt": {
        "source_page": "https://freesound.org/people/rammbostein/sounds/263459/",
        "source_id": "263459",
        "source_title": "Mosin Nagant Bolt.wav",
        "source_author": "rammbostein",
        "source_duration_s": 6.500,
        "source_family": "real bolt-action rifle action",
        "mirror_pages": [
            "https://pixabay.com/sound-effects/film-special-effects-mosin-nagant-bolt-85049/",
        ],
        "output_name": "bolt_action_cc0_preview_donor.wav",
        "identity_scope": "bolt-action-family donor; Mosin-Nagant source, not M700 identity",
        "expected_transport_sha256": "",
    },
}


def fetch_bytes(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT, "Accept": "*/*"})
    with urllib.request.urlopen(req, timeout=45) as response:
        data = response.read()
    if not data:
        raise RuntimeError(f"empty response: {url}")
    return data


def fetch_text(url: str) -> str:
    return fetch_bytes(url).decode("utf-8", errors="replace")


def normalize_embedded_url(value: str) -> str:
    value = html.unescape(value)
    value = value.replace("\\u0026", "&").replace("\\/", "/")
    return value


def extract_audio_urls(page_text: str) -> list[str]:
    normalized = normalize_embedded_url(page_text)
    candidates: list[str] = []
    patterns = (
        r"https://cdn\.freesound\.org/previews/[^\"'<>\s]+?\.(?:mp3|ogg)(?:\?[^\"'<>\s]*)?",
        r"https://cdn\.pixabay\.com/download/audio/[^\"'<>\s]+?\.mp3(?:\?[^\"'<>\s]*)?",
    )
    for pattern in patterns:
        for match in re.findall(pattern, normalized, flags=re.IGNORECASE):
            url = match.rstrip("\\")
            if url not in candidates:
                candidates.append(url)
    candidates.sort(key=lambda u: ("-hq." not in u.lower(), ".mp3" not in u.lower(), len(u)))
    return candidates


def validate_source_contract(source_html: str, donor: dict[str, object]) -> None:
    missing = []
    for needle in (str(donor["source_id"]), str(donor["source_title"]), str(donor["source_author"])):
        if needle.lower() not in source_html.lower():
            missing.append(needle)
    cc0_markers = (
        "Creative Commons 0",
        "creativecommons.org/publicdomain/zero/1.0",
        "creativecommons.org/publicdomain/zero/1.0/",
    )
    if not any(marker.lower() in source_html.lower() for marker in cc0_markers):
        missing.append("CC0 marker")
    if missing:
        raise RuntimeError(f"source provenance page missing expected markers: {missing}")


def resolve_transport(donor: dict[str, object], source_html: str) -> tuple[str, str]:
    source_urls = extract_audio_urls(source_html)
    if source_urls:
        return source_urls[0], "freesound_public_preview"

    errors = []
    for mirror in donor["mirror_pages"]:
        try:
            mirror_html = fetch_text(str(mirror))
            if str(donor["source_title"]).split(".")[0].lower() not in mirror_html.lower():
                errors.append(f"title marker absent on {mirror}")
                continue
            urls = extract_audio_urls(mirror_html)
            if urls:
                return urls[0], f"pixabay_freesound_community_mirror:{mirror}"
            errors.append(f"no public audio URL on {mirror}")
        except Exception as exc:  # noqa: BLE001 - audit should report all transport failures
            errors.append(f"{mirror}: {exc}")
    raise RuntimeError("unable to resolve public CC0 preview transport; " + "; ".join(errors))


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def run_ffmpeg(source: Path, target: Path) -> None:
    cmd = [
        "ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-i", str(source),
        "-map_metadata", "-1",
        "-vn",
        "-ac", "1",
        "-ar", "48000",
        "-c:a", "pcm_s16le",
        str(target),
    ]
    subprocess.run(cmd, check=True)


def inspect_wav(path: Path) -> dict[str, object]:
    with wave.open(str(path), "rb") as wf:
        frames = wf.getnframes()
        rate = wf.getframerate()
        duration = frames / float(rate)
        return {
            "channels": wf.getnchannels(),
            "sample_width_bytes": wf.getsampwidth(),
            "sample_rate_hz": rate,
            "frames": frames,
            "duration_s": round(duration, 6),
        }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("audit", "write"), default="audit")
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--manifest", required=True)
    args = parser.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = Path(args.manifest)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)

    results: dict[str, object] = {
        "schema": 1,
        "runtime_ready": False,
        "ue_import_pending": True,
        "item16_checked": False,
        "derivative_recipe": "ffmpeg -map_metadata -1 -vn -ac 1 -ar 48000 -c:a pcm_s16le",
        "donors": {},
    }

    for key, donor in DONORS.items():
        print(f"PASS45_AUDIO_INTAKE_BEGIN donor={key}", flush=True)
        source_html = fetch_text(str(donor["source_page"]))
        validate_source_contract(source_html, donor)
        transport_url, transport_kind = resolve_transport(donor, source_html)
        transport_bytes = fetch_bytes(transport_url)
        transport_sha = sha256(transport_bytes)
        expected = str(donor["expected_transport_sha256"])

        if args.mode == "write":
            if not expected:
                raise RuntimeError(f"write mode forbidden without pinned transport SHA256 for {key}")
            if transport_sha != expected:
                raise RuntimeError(
                    f"transport SHA256 changed for {key}: expected={expected} actual={transport_sha}"
                )

        encoded_path = out_dir / f".{key}_transport_audio"
        encoded_path.write_bytes(transport_bytes)
        wav_path = out_dir / str(donor["output_name"])
        run_ffmpeg(encoded_path, wav_path)
        encoded_path.unlink(missing_ok=True)
        wav_bytes = wav_path.read_bytes()
        wav_info = inspect_wav(wav_path)

        source_duration = float(donor["source_duration_s"])
        if abs(float(wav_info["duration_s"]) - source_duration) > max(0.35, source_duration * 0.10):
            raise RuntimeError(
                f"duration mismatch for {key}: source={source_duration} derivative={wav_info['duration_s']}"
            )

        record = {
            "source_page": donor["source_page"],
            "source_title": donor["source_title"],
            "source_author": donor["source_author"],
            "source_license": "CC0",
            "source_family": donor["source_family"],
            "identity_scope": donor["identity_scope"],
            "transport_kind": transport_kind,
            "transport_url": transport_url,
            "transport_sha256": transport_sha,
            "transport_bytes": len(transport_bytes),
            "derivative_file": str(donor["output_name"]),
            "derivative_sha256": sha256(wav_bytes),
            "derivative_bytes": len(wav_bytes),
            "derivative_audio": wav_info,
            "runtime_ready": False,
            "ue_import_pending": True,
        }
        results["donors"][key] = record
        print(
            f"PASS45_AUDIO_INTAKE_AUDIT donor={key} transport_sha256={transport_sha} "
            f"wav_sha256={record['derivative_sha256']} duration_s={wav_info['duration_s']} "
            f"transport={transport_kind}",
            flush=True,
        )

    manifest_path.write_text(json.dumps(results, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS45_AUDIO_INTAKE_COMPLETE mode={args.mode} runtime_ready=0 item16_checked=0")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001 - fail visibly in CI
        print(f"PASS45_AUDIO_INTAKE_FAIL {exc}", file=sys.stderr)
        raise
