#!/usr/bin/env python3
"""PASS45 2026-09-03 quarantined asset-intake auditor.

Reads downloaded ZIP payloads only. It never extracts into production, runs Git,
imports Unreal assets, deletes files, or changes runtime authority.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import zipfile
from collections import Counter, defaultdict

ROOT = Path(__file__).resolve().parent
DEFAULT_REPORT_DIR = ROOT / "PC_TEST" / "TEST_RESULTS"
MODEL_EXTS = {".fbx", ".glb", ".gltf", ".obj", ".blend", ".dae"}
TEXTURE_EXTS = {".png", ".jpg", ".jpeg", ".tga", ".dds", ".exr", ".bmp", ".webp", ".ktx", ".ktx2"}
AUDIO_EXTS = {".wav", ".ogg", ".mp3", ".flac"}
ANIMATION_HINTS = ("anim", "animation", "rig", "skeleton", "armature", "skeletal")
LICENSE_HINTS = ("license", "licence", "copying", "copyright", "credits", "attribution")
SOURCE_HINTS = ("readme", "source", "provenance", "sketchfab", "fab", "url")
DANGEROUS_SUFFIXES = {".exe", ".dll", ".bat", ".cmd", ".ps1", ".vbs", ".js", ".msi", ".scr", ".com"}


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(8 * 1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def normalized_zip_name(name: str) -> str:
    return name.replace("\\", "/")


def unsafe_member(name: str) -> bool:
    normalized = normalized_zip_name(name)
    if normalized.startswith("/") or normalized.startswith("\\"):
        return True
    if re.match(r"^[A-Za-z]:", normalized):
        return True
    parts = PurePosixPath(normalized).parts
    return any(part == ".." for part in parts)


def detect_source_root(explicit: str | None) -> Path:
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit).expanduser())
    env_root = os.environ.get("PASS45_ASSET_INTAKE_DIR")
    if env_root:
        candidates.append(Path(env_root).expanduser())
    candidates.extend(
        [
            ROOT / "models_game_OC",
            ROOT.parent / "OsterConflict" / "models_game_OC",
        ]
    )
    for candidate in candidates:
        candidate = candidate.resolve()
        if candidate.is_dir() and any(candidate.rglob("*.zip")):
            return candidate
    tried = "\n".join(f"  - {p}" for p in candidates)
    raise SystemExit(
        "PASS45 ASSET INTAKE: FAIL\n"
        "No models_game_OC ZIP source directory was found.\n"
        "Tried:\n" + tried + "\n"
        "Pass the source folder explicitly as the first argument if it lives elsewhere."
    )


def inspect_archive(path: Path, root: Path) -> dict:
    rel = path.relative_to(root).as_posix()
    result: dict = {
        "archive": rel,
        "bytes": path.stat().st_size,
        "sha256": sha256_file(path),
        "status": "UNINSPECTED",
        "entry_count": 0,
        "uncompressed_bytes": 0,
        "models": [],
        "textures": [],
        "audio": [],
        "animation_or_rig_hints": [],
        "license_files": [],
        "source_or_readme_files": [],
        "dangerous_files": [],
        "unsafe_members": [],
        "encrypted_members": [],
        "extensions": {},
        "errors": [],
    }
    try:
        with zipfile.ZipFile(path) as zf:
            infos = zf.infolist()
            result["entry_count"] = len(infos)
            result["uncompressed_bytes"] = sum(info.file_size for info in infos)
            ext_counter: Counter[str] = Counter()
            for info in infos:
                if info.is_dir():
                    continue
                name = normalized_zip_name(info.filename)
                suffix = Path(name).suffix.lower()
                if suffix:
                    ext_counter[suffix] += 1
                if unsafe_member(name):
                    result["unsafe_members"].append(name)
                if info.flag_bits & 0x1:
                    result["encrypted_members"].append(name)
                lower = name.lower()
                base = Path(name).name.lower()
                if suffix in MODEL_EXTS:
                    result["models"].append(name)
                if suffix in TEXTURE_EXTS:
                    result["textures"].append(name)
                if suffix in AUDIO_EXTS:
                    result["audio"].append(name)
                if any(hint in lower for hint in ANIMATION_HINTS):
                    result["animation_or_rig_hints"].append(name)
                if any(hint in base for hint in LICENSE_HINTS):
                    result["license_files"].append(name)
                if any(hint in base for hint in SOURCE_HINTS):
                    result["source_or_readme_files"].append(name)
                if suffix in DANGEROUS_SUFFIXES:
                    result["dangerous_files"].append(name)
            result["extensions"] = dict(sorted(ext_counter.items()))
    except (zipfile.BadZipFile, OSError, RuntimeError) as exc:
        result["errors"].append(f"{type(exc).__name__}: {exc}")

    if result["errors"] or result["unsafe_members"] or result["encrypted_members"]:
        result["status"] = "REJECT_ARCHIVE"
    elif not result["models"]:
        result["status"] = "NO_MODEL_PAYLOAD"
    elif not result["license_files"] and not result["source_or_readme_files"]:
        result["status"] = "NEEDS_PROVENANCE"
    else:
        result["status"] = "AUDITABLE_CANDIDATE"
    return result


def markdown_report(data: dict) -> str:
    lines = [
        "# PASS45 Asset Intake Audit — 2026-09-03",
        "",
        f"- Source: `{data['source_root']}`",
        f"- ZIP archives: **{data['archive_count']}**",
        f"- Total compressed bytes: **{data['total_bytes']:,}**",
        f"- Exact duplicate groups: **{len(data['duplicate_groups'])}**",
        f"- Archive rejects: **{data['status_counts'].get('REJECT_ARCHIVE', 0)}**",
        f"- Needs provenance: **{data['status_counts'].get('NEEDS_PROVENANCE', 0)}**",
        f"- Auditable candidates: **{data['status_counts'].get('AUDITABLE_CANDIDATE', 0)}**",
        "",
        "> This report is intake evidence only. It does not import, promote, or runtime-accept any asset.",
        "",
        "## Archives",
        "",
        "| Archive | Status | Models | Textures | License/source clues | SHA-256 |",
        "|---|---|---:|---:|---:|---|",
    ]
    for item in data["archives"]:
        clues = len(item["license_files"]) + len(item["source_or_readme_files"])
        lines.append(
            f"| `{item['archive']}` | `{item['status']}` | {len(item['models'])} | "
            f"{len(item['textures'])} | {clues} | `{item['sha256'][:16]}…` |"
        )
    lines.extend(["", "## Exact duplicates", ""])
    if not data["duplicate_groups"]:
        lines.append("None detected by exact archive SHA-256.")
    else:
        for group in data["duplicate_groups"]:
            lines.append(f"- `{group['sha256']}`: " + ", ".join(f"`{p}`" for p in group["archives"]))
    lines.extend(
        [
            "",
            "## Gate state",
            "",
            "- `runtime_acceptance=0`",
            "- `item16_checked=0`",
            "- `merge_permitted=0`",
            "",
            "Next: manually/procedurally resolve exact provenance/license for each candidate, then promote only approved assets one by one.",
        ]
    )
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", help="Path to models_game_OC")
    parser.add_argument("--report-dir", help="Override report output directory")
    args = parser.parse_args()

    source_root = detect_source_root(args.source)
    report_dir = Path(args.report_dir).expanduser().resolve() if args.report_dir else DEFAULT_REPORT_DIR
    report_dir.mkdir(parents=True, exist_ok=True)

    archives = [inspect_archive(path, source_root) for path in sorted(source_root.rglob("*.zip"))]
    by_sha: dict[str, list[str]] = defaultdict(list)
    for item in archives:
        by_sha[item["sha256"]].append(item["archive"])
    duplicates = [
        {"sha256": sha, "archives": paths}
        for sha, paths in sorted(by_sha.items())
        if len(paths) > 1
    ]
    status_counts = Counter(item["status"] for item in archives)
    report = {
        "schema": 1,
        "audit": "PASS45_ASSET_INTAKE_2026-09-03",
        "source_root": str(source_root),
        "archive_count": len(archives),
        "total_bytes": sum(item["bytes"] for item in archives),
        "status_counts": dict(sorted(status_counts.items())),
        "duplicate_groups": duplicates,
        "archives": archives,
        "runtime_acceptance": 0,
        "item16_checked": 0,
        "merge_permitted": 0,
    }

    json_path = report_dir / "PASS45_ASSET_INTAKE_20260903_REPORT.json"
    md_path = report_dir / "PASS45_ASSET_INTAKE_20260903_REPORT.md"
    json_path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    md_path.write_text(markdown_report(report), encoding="utf-8")

    print("PASS45_ASSET_INTAKE_AUDIT_COMPLETE")
    print(f"source={source_root}")
    print(f"archives={len(archives)} total_bytes={report['total_bytes']}")
    print(f"duplicates={len(duplicates)} rejects={status_counts.get('REJECT_ARCHIVE', 0)}")
    print(f"report_json={json_path}")
    print(f"report_md={md_path}")
    print("runtime_acceptance=0")
    print("item16_checked=0")
    print("merge_permitted=0")

    if any(item["status"] == "REJECT_ARCHIVE" for item in archives):
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
