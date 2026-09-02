#!/usr/bin/env python3
"""Structural guard for the Remington 870 bind-pose visual identity audit."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent
AUDIT = ROOT / "PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT.py"
WORKFLOW = ROOT / ".github/workflows/pass45-remington870-pump-identity-visual-audit.yml"
EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 PUMP IDENTITY VISUAL AUDIT CONTRACT: FAIL\n[FAIL] {message}")


def require(path: Path) -> str:
    if not path.is_file():
        fail(f"missing required file: {path.relative_to(ROOT).as_posix()}")
    return path.read_text(encoding="utf-8")


def main() -> None:
    source = require(AUDIT)
    workflow = require(WORKFLOW)

    required_source = (
        "import PASS45_REMINGTON870_REMOTE_CANDIDATE_AUDIT as remote",
        "import PASS45_REMINGTON870_SOURCE_ACQUIRE as acquire",
        "import PASS45_REMINGTON870_STRUCTURE_AUDIT as structure",
        f'EXPECTED_SHA256 = "{EXPECTED_SHA256}"',
        'TARGETS = ("PBody_058", "Pmag_061")',
        "skin_inverse_binds(",
        "global_matrices(",
        "target_world_points(",
        "context_world_points(",
        '"status": "VISUAL_SEMANTIC_REVIEW_REQUIRED"',
        '"pump_node_identity": "UNPROVEN"',
        '"standalone_pump_clip": "UNPROVEN"',
        '"production_cutover": False',
        '"runtime_acceptance": False',
        '"item16_checked": False',
        "--output-json",
        "--output-svg",
    )
    for needle in required_source:
        if needle not in source:
            fail(f"visual audit lost required fail-closed contract: {needle}")

    forbidden_source = (
        '"pump_node_identity": "Pmag_061"',
        '"pump_node_identity": "PROVEN"',
        '"item16_checked": True',
        '"production_cutover": True',
        '"runtime_acceptance": True',
        "EditorAssetLibrary",
        "/Game/Production/Weapons/Remington870",
    )
    for needle in forbidden_source:
        if needle in source:
            fail(f"visual audit contains forbidden promotion/mutation token: {needle}")

    required_workflow = (
        "PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT.py",
        "VERIFY_PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT.py",
        "actions/upload-artifact@v4",
        "pass45-remington870-pump-identity-visual-evidence",
        "--output-json",
        "--output-svg",
    )
    for needle in required_workflow:
        if needle not in workflow:
            fail(f"workflow lost required visual-evidence contract: {needle}")

    print("PASS45 REMINGTON870 PUMP IDENTITY VISUAL AUDIT CONTRACT: PASS")
    print(
        "exact_donor=1 bind_pose_world_space=1 visual_semantic_review_required=1 "
        "production_cutover=0 runtime_acceptance=0 item16_checked=0"
    )


if __name__ == "__main__":
    main()
