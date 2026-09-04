#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROJECT = ROOT / "OsterConflict"
INBOX = ROOT / "models_game_OC"
STATUS_DIR = PROJECT / "Saved" / "AssetStatus"
STATUS_JSON = STATUS_DIR / "LOCAL_ASSET_STATUS.json"
STATUS_TEXT = STATUS_DIR / "LOCAL_ASSET_STATUS.txt"
PREPARED_JSON = PROJECT / "Saved" / "LocalModelInbox" / "prepared_sources.json"
MANUAL_JSON = STATUS_DIR / "MANUAL_VISUAL_ACCEPTANCE.json"
MANUAL_TEXT = STATUS_DIR / "MANUAL_VISUAL_ACCEPTANCE.txt"
CLEANUP_JSON = STATUS_DIR / "ACCEPTED_ZIP_CLEANUP.json"
CLEANUP_TEXT = STATUS_DIR / "ACCEPTED_ZIP_CLEANUP.txt"

TRACKED_ACCEPTANCE_SOURCE = (
    "START_HERE.cmd",
    "RUN_R14_CURRENT_GAMEPLAY.cmd",
    "COLLECT_LOCAL_ASSET_STATUS.py",
    "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py",
    "OsterConflict/IMPORT_ALL_LOCAL_INBOX_UE58.cmd",
    "OsterConflict/IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "OsterConflict/RUN_PASS45_STRICT_MATERIAL_GATE.cmd",
    "OsterConflict/Scripts",
    "OsterConflict/Source",
)

VISUAL_CHECKLIST = (
    "HMMWV scale/orientation/materials and usable vehicle presentation",
    "M2 Browning mount/alignment/pitch/muzzle presentation",
    "BTR-4 proportions/orientation/materials",
    "all discovered weapon models including M16/M4 family",
    "buildings/props/furniture/fences/foliage/roads/terrain/water",
    "character skins and HUD/UI assets that were discovered and bound",
    "no obvious placeholder, broken material, absurd scale, or detached mesh",
)


def fail(message: str, code: int = 1) -> int:
    print(f"[FINALIZE STOP] {message}")
    return code


def load_json(path: Path) -> dict:
    if not path.is_file():
        raise RuntimeError(f"missing required file: {path}")
    value = json.loads(path.read_text(encoding="utf-8-sig", errors="strict"))
    if not isinstance(value, dict):
        raise RuntimeError(f"invalid JSON object: {path}")
    return value


def git_output(*args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(ROOT), *args],
        check=False,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed: {result.stderr.strip()}")
    return result.stdout.strip()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().lower()


def relative(path: Path) -> str:
    return str(path.resolve().relative_to(ROOT.resolve())).replace("\\", "/")


def verify_exact_remote_head(head: str) -> None:
    branch = git_output("branch", "--show-current")
    if not branch:
        raise RuntimeError("current Git branch is unknown; final acceptance refused")
    git_output("fetch", "origin", branch)
    remote = git_output("rev-parse", f"origin/{branch}")
    if remote.lower() != head.lower():
        raise RuntimeError(
            f"local HEAD {head} != origin/{branch} {remote}; pull current branch and rerun acceptance"
        )


def verify_clean_acceptance_source() -> None:
    output = git_output(
        "status",
        "--porcelain",
        "--untracked-files=no",
        "--",
        *TRACKED_ACCEPTANCE_SOURCE,
    )
    if output:
        raise RuntimeError(
            "tracked runtime/acceptance source has local edits; commit/revert them before final acceptance:\n"
            + output
        )


def verify_current_automated_status(status: dict, head: str) -> None:
    source_sha = str(status.get("source_sha") or "")
    if source_sha.lower() != head.lower():
        raise RuntimeError(f"LOCAL_ASSET_STATUS source {source_sha or 'UNKNOWN'} != current HEAD {head}")
    if status.get("runtime_scope") != "CURRENT_RUN_COMPLETED":
        raise RuntimeError("full current runtime has not completed successfully")

    stages = status.get("stages") or {}
    required = (
        "local_ue_import",
        "live_runtime_hookup",
        "strict_material_gate",
        "automated_runtime_evidence",
    )
    gaps = [name for name in required if stages.get(name) != "PASS"]
    if gaps:
        raise RuntimeError("automated acceptance gates are not PASS: " + ", ".join(gaps))

    production = status.get("production") or {}
    if production.get("vehicles_status") != "PASS" or production.get("weapons_status") != "PASS":
        raise RuntimeError("production vehicle/weapon status is not PASS")

    bindings = status.get("bindings") or {}
    if not bindings.get("all_models_bound"):
        raise RuntimeError("not all discovered models are runtime-bound")
    if bindings.get("unbound"):
        raise RuntimeError("unbound/GAP rows remain in LOCAL_ASSET_STATUS")

    category_counts = bindings.get("category_counts") or {}
    try:
        m16_m4_count = int(category_counts.get("M16_M4") or 0)
    except (TypeError, ValueError):
        m16_m4_count = 0
    if m16_m4_count < 1:
        raise RuntimeError("M16/M4 production content gap is still open; fresh manifest has no M16_M4 payload")


def verify_prepared_manifest(prepared: dict) -> set[str]:
    prepared_status = str(prepared.get("status") or "").upper()
    if prepared_status not in {"PASS", "NO_INBOX"}:
        raise RuntimeError(f"prepared_sources status is not PASS/NO_INBOX: {prepared.get('status')}")
    if prepared.get("conflicts"):
        raise RuntimeError("prepared_sources still contains package conflicts")

    accepted_hashes: set[str] = set()
    bad_archives: list[str] = []
    for row in prepared.get("archives", []) or []:
        if not isinstance(row, dict):
            continue
        status = str(row.get("status") or "").upper()
        digest = str(row.get("sha256") or "").lower()
        if status != "EXTRACTED" or len(digest) != 64:
            bad_archives.append(str(row.get("archive") or "UNKNOWN"))
            continue
        accepted_hashes.add(digest)
    if bad_archives:
        raise RuntimeError("one or more archive records were not safely extracted: " + "; ".join(bad_archives))
    return accepted_hashes


def preflight_source_zips(accepted_hashes: set[str]) -> list[tuple[Path, str, int]]:
    if not INBOX.is_dir():
        return []

    rows: list[tuple[Path, str, int]] = []
    unknown: list[str] = []
    for path in sorted((p for p in INBOX.rglob("*") if p.is_file() and p.suffix.lower() == ".zip"), key=lambda p: str(p).lower()):
        digest = sha256(path)
        if digest not in accepted_hashes:
            unknown.append(f"{relative(path)} sha256={digest}")
            continue
        rows.append((path, digest, path.stat().st_size))

    if unknown:
        raise RuntimeError(
            "source ZIP cleanup refused because current manifest does not prove these archives:\n"
            + "\n".join(unknown)
        )
    return rows


def run_preflight() -> tuple[str, list[tuple[Path, str, int]]]:
    head = git_output("rev-parse", "HEAD")
    if not head:
        raise RuntimeError("could not determine current Git HEAD")
    verify_exact_remote_head(head)
    verify_clean_acceptance_source()

    current = load_json(STATUS_JSON)
    verify_current_automated_status(current, head)

    prepared = load_json(PREPARED_JSON)
    accepted_hashes = verify_prepared_manifest(prepared)
    source_zips = preflight_source_zips(accepted_hashes)
    return head, source_zips


def write_manual_acceptance(head: str) -> None:
    now = datetime.now(timezone.utc).isoformat()
    payload = {
        "schema": "oster-conflict-pass45-manual-visual-acceptance-v1",
        "status": "PASS",
        "source_sha": head,
        "accepted_at_utc": now,
        "assertion": "User explicitly confirmed direct visual inspection after automated UE/runtime acceptance.",
        "checklist": list(VISUAL_CHECKLIST),
    }
    STATUS_DIR.mkdir(parents=True, exist_ok=True)
    MANUAL_JSON.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    MANUAL_TEXT.write_text(
        "PASS45_MANUAL_VISUAL_ACCEPTANCE=PASS\n"
        f"SOURCE_SHA={head}\n"
        f"ACCEPTED_AT_UTC={now}\n"
        + "\n".join(f"CHECKED={item}" for item in VISUAL_CHECKLIST)
        + "\n",
        encoding="utf-8",
    )


def write_cleanup_report(head: str, status: str, deleted: list[dict], error: str | None = None) -> None:
    now = datetime.now(timezone.utc).isoformat()
    payload = {
        "schema": "oster-conflict-pass45-accepted-zip-cleanup-v1",
        "status": status,
        "source_sha": head,
        "finished_at_utc": now,
        "deleted_count": len(deleted),
        "deleted_bytes": sum(int(row.get("bytes") or 0) for row in deleted),
        "deleted": deleted,
        "error": error,
    }
    STATUS_DIR.mkdir(parents=True, exist_ok=True)
    CLEANUP_JSON.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    lines = [
        f"PASS45_ACCEPTED_ZIP_CLEANUP={status}",
        f"SOURCE_SHA={head}",
        f"DELETED_COUNT={payload['deleted_count']}",
        f"DELETED_BYTES={payload['deleted_bytes']}",
    ]
    if error:
        lines.append(f"ERROR={error}")
    for row in deleted:
        lines.append(f"DELETED={row['path']} sha256={row['sha256']} bytes={row['bytes']}")
    CLEANUP_TEXT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def refresh_consolidated_status(head: str) -> None:
    sys.path.insert(0, str(ROOT))
    import COLLECT_LOCAL_ASSET_STATUS as asset_status

    asset_status.collect_snapshot(source_sha=head, import_result=0, runtime_result=0)


def main() -> int:
    args = set(sys.argv[1:])
    preflight_only = "--preflight" in args
    accept_visual = "--accept-visual" in args
    if preflight_only == accept_visual:
        return fail("use exactly one mode: --preflight or --accept-visual", 2)

    try:
        head, source_zips = run_preflight()
    except Exception as exc:
        return fail(str(exc), 4)

    if preflight_only:
        print("[FINALIZE PRECHECK PASS] Exact remote/source/runtime/content state is eligible for manual visual acceptance.")
        print(f"[FINALIZE PRECHECK PASS] Manifest-proven source ZIPs eligible for cleanup: {len(source_zips)}")
        return 0

    # Manual PASS is written only after every automated/source/content/ZIP preflight above succeeded.
    write_manual_acceptance(head)

    deleted: list[dict] = []
    try:
        for path, digest, size in source_zips:
            row = {"path": relative(path), "sha256": digest, "bytes": size}
            path.unlink()
            deleted.append(row)
        write_cleanup_report(head, "PASS", deleted)
        refresh_consolidated_status(head)
    except Exception as exc:
        write_cleanup_report(head, "FAIL", deleted, str(exc))
        try:
            refresh_consolidated_status(head)
        except Exception:
            pass
        return fail(f"ZIP cleanup did not complete: {exc}", 5)

    print("[FINALIZE PASS] Direct visual acceptance recorded for exact current remote HEAD.")
    print(f"[FINALIZE PASS] Safely deleted {len(deleted)} manifest-proven source ZIP(s).")
    print(f"[FINALIZE PASS] Consolidated status refreshed: {STATUS_TEXT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
