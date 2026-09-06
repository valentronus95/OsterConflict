#!/usr/bin/env python3
"""Regression contract for item-16 calibration-review provenance invalidation."""
from __future__ import annotations

import subprocess
import tempfile
from pathlib import Path

from VERIFY_PASS45_ITEM16_CALIBRATION_RECEIPT_BINDING import (
    CALIBRATION_CRITICAL_PATHS,
    validate_evidence_head_repository,
)

REVIEW = "PASS45_ITEM16_M700_LEVER_CALIBRATION_REVIEW.py"


def git(repo: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=repo,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed: {result.stderr}")
    return result.stdout.strip()


def main() -> int:
    failures: list[str] = []

    if REVIEW not in CALIBRATION_CRITICAL_PATHS:
        failures.append("calibration review implementation is missing from CALIBRATION_CRITICAL_PATHS")

    with tempfile.TemporaryDirectory(prefix="pass45_item16_review_provenance_") as temp_dir:
        repo = Path(temp_dir)
        git(repo, "init", "-q")
        git(repo, "config", "user.email", "pass45@example.invalid")
        git(repo, "config", "user.name", "PASS45 Contract")

        (repo / REVIEW).write_text("accepted_envelope = 1\n", encoding="utf-8")
        git(repo, "add", REVIEW)
        git(repo, "commit", "-q", "-m", "calibration evidence implementation")
        evidence_head = git(repo, "rev-parse", "HEAD")

        (repo / "README.txt").write_text("non critical\n", encoding="utf-8")
        git(repo, "add", "README.txt")
        git(repo, "commit", "-q", "-m", "non-critical descendant")
        errors = validate_evidence_head_repository(evidence_head, repo_root=repo)
        if errors:
            failures.append(f"non-critical descendant unexpectedly invalidated calibration: {errors}")

        (repo / REVIEW).write_text("accepted_envelope = 2\n", encoding="utf-8")
        git(repo, "add", REVIEW)
        git(repo, "commit", "-q", "-m", "calibration review semantics drift")
        errors = validate_evidence_head_repository(evidence_head, repo_root=repo)
        if not any(REVIEW in error and "fresh current-head UE 5.8 visual calibration is required" in error for error in errors):
            failures.append(f"review semantics drift did not invalidate stale calibration approval: {errors}")

    if failures:
        print("PASS45 ITEM16 CALIBRATION REVIEW PROVENANCE CONTRACT: FAIL")
        for failure in failures:
            print(f"[FAIL] {failure}")
        print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
        return 1

    print("PASS45 ITEM16 CALIBRATION REVIEW PROVENANCE CONTRACT: PASS")
    print("review_implementation_critical=1 noncritical_descendant_allowed=1 stale_review_drift_rejected=1")
    print("runtime_acceptance=0 item16_checked=0 merge_permitted=0 user_local_execution_requested=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())