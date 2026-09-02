#!/usr/bin/env python3
"""Guard the Remington_870_FREE local candidate against accidental production promotion."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CANDIDATE = "Remington_870_FREE.glb"
AUDIT = Path("PASS45_REMINGTON870_LOCAL_CANDIDATE_IDENTITY_AUDIT.py")
DOC = Path("_DOCS/PASS45_REMINGTON870_LOCAL_CANDIDATE_QUARANTINE_2026-09-02.md")
WORKFLOW = Path(".github/workflows/pass45-remington870-local-candidate-quarantine.yml")
SELF = Path(__file__).name
MANIFEST = Path("SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")
REGISTER = Path("_DOCS/THIRD_PARTY_CODE_AND_ASSET_REGISTER.md")

ALLOWED_CANDIDATE_REFERENCES = {
    AUDIT.as_posix(),
    DOC.as_posix(),
    WORKFLOW.as_posix(),
    SELF.as_posix(),
}
TEXT_SUFFIXES = {
    ".py", ".md", ".json", ".yml", ".yaml", ".cmd", ".bat", ".ini",
    ".cpp", ".h", ".cs", ".txt", ".toml",
}


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 REMINGTON870 LOCAL CANDIDATE QUARANTINE: FAIL\n[FAIL] {message}")


def read_text(relative: Path) -> str:
    path = ROOT / relative
    if not path.is_file():
        fail(f"missing required file: {relative.as_posix()}")
    return path.read_text(encoding="utf-8")


def verify_audit_contract() -> None:
    source = read_text(AUDIT)
    for needle in (
        'LOCAL_CANDIDATE_BASENAME = "Remington_870_FREE.glb"',
        'REQUIRED_ACTION_NAMES = ("PumpAction", "Cube.002Action")',
        'STATUS = "UNREGISTERED_LOCAL_CANDIDATE"',
        '"source_url": None',
        '"creator": None',
        '"license_id": None',
        '"public_repo_allowed": None',
        '"production_cutover": False',
        '"runtime_acceptance": False',
        '"item16_checked": False',
    ):
        if needle not in source:
            fail(f"identity audit lost fail-closed contract: {needle}")


def verify_quarantine_doc() -> None:
    text = read_text(DOC)
    for needle in (
        "IMPORTED-MOTION PROOF ONLY",
        "UNREGISTERED_LOCAL_CANDIDATE",
        "remington_870_8siandude_ccby4.glb",
        "do not merge PR #94",
        "source_url",
        "creator",
        "license_id",
    ):
        if needle not in text:
            fail(f"quarantine document lost required boundary: {needle}")


def verify_registered_manifest_stays_separate() -> None:
    manifest = json.loads(read_text(MANIFEST))
    if manifest.get("source_asset_file") != "remington_870_8siandude_ccby4.glb":
        fail(f"registered donor filename drifted: {manifest.get('source_asset_file')!r}")
    if manifest.get("source_sha256") != "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2":
        fail("registered donor SHA-256 drifted")
    if manifest.get("runtime_ready") is not False:
        fail("registered donor must remain runtime_ready=false before acceptance")
    if manifest.get("item16_checked") is not False:
        fail("registered donor must not close item 16")
    if CANDIDATE in json.dumps(manifest, sort_keys=True):
        fail("unregistered local candidate leaked into registered donor manifest")


def verify_third_party_register_stays_separate() -> None:
    text = read_text(REGISTER)
    for needle in (
        "PASS45-3P-WEAPON-001",
        "remington_870_8siandude_ccby4.glb",
        "8sianDude",
        "CC-BY-4.0",
    ):
        if needle not in text:
            fail(f"third-party register lost registered donor identity: {needle}")
    if CANDIDATE in text:
        fail("unregistered local candidate must not appear as a registered third-party asset")


def verify_candidate_not_promoted_elsewhere() -> None:
    violations: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or ".git" in path.parts:
            continue
        rel = path.relative_to(ROOT).as_posix()
        if rel in ALLOWED_CANDIDATE_REFERENCES:
            continue
        if path.suffix.lower() not in TEXT_SUFFIXES:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if CANDIDATE in text:
            violations.append(rel)
    if violations:
        fail("unregistered local candidate leaked outside quarantine: " + ", ".join(sorted(violations)))


def main() -> None:
    verify_audit_contract()
    verify_quarantine_doc()
    verify_registered_manifest_stays_separate()
    verify_third_party_register_stays_separate()
    verify_candidate_not_promoted_elsewhere()
    print("PASS45 REMINGTON870 LOCAL CANDIDATE QUARANTINE: PASS")
    print("registered_donor=8sianDude local_candidate=UNREGISTERED_LOCAL_CANDIDATE production_cutover=0 item16_checked=0")


if __name__ == "__main__":
    main()
