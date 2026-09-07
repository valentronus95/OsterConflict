#!/usr/bin/env python3
"""Guard the Remington 870 Pmag connected-component evidence audit."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
AUDIT = ROOT / "PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT.py"
WORKFLOW = ROOT / ".github/workflows/pass45-remington870-pmag-component-audit.yml"
EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 PMAG COMPONENT AUDIT CONTRACT: FAIL\n[FAIL] {message}")


def require(path: Path) -> str:
    if not path.is_file():
        fail(f"missing required file: {path.relative_to(ROOT).as_posix()}")
    return path.read_text(encoding="utf-8")


def main() -> None:
    source = require(AUDIT)
    workflow = require(WORKFLOW)
    for needle in (
        "import PASS45_REMINGTON870_PUMP_IDENTITY_VISUAL_AUDIT as visual",
        f'EXPECTED_SHA256 = "{EXPECTED_SHA256}"',
        'TARGET = "Pmag_061"',
        "class UnionFind",
        "component_rows(",
        "component_count",
        '"status": "COMPONENT_TOPOLOGY_EVIDENCE_ONLY"',
        '"pump_node_identity": "UNPROVEN"',
        '"standalone_pump_clip": "UNPROVEN"',
        '"production_cutover": False',
        '"runtime_acceptance": False',
        '"item16_checked": False',
        "--output-json",
        "--output-svg",
    ):
        if needle not in source:
            fail(f"component audit lost contract token: {needle}")
    for needle in (
        '"pump_node_identity": "PROVEN"',
        '"production_cutover": True',
        '"runtime_acceptance": True',
        '"item16_checked": True',
        "EditorAssetLibrary",
        "/Game/Production/Weapons/Remington870",
    ):
        if needle in source:
            fail(f"component audit contains forbidden promotion token: {needle}")
    for needle in (
        "PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT.py",
        "VERIFY_PASS45_REMINGTON870_PMAG_COMPONENT_AUDIT.py",
        "actions/upload-artifact@v4",
        "pass45-remington870-pmag-component-evidence",
    ):
        if needle not in workflow:
            fail(f"component workflow lost contract token: {needle}")
    print("PASS45 REMINGTON870 PMAG COMPONENT AUDIT CONTRACT: PASS")
    print("topology_only=1 semantic_acceptance=0 production_cutover=0 runtime_acceptance=0 item16_checked=0")


if __name__ == "__main__":
    main()
