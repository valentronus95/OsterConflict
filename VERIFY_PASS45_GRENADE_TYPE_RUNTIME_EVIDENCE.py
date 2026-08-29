#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GRENADE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCGrenadeProjectile.cpp"
EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 GRENADE TYPE RUNTIME EVIDENCE: FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


grenade = read(GRENADE)
evidence = read(EVIDENCE)
errors: list[str] = []

for marker in (
    "PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_READY",
    "PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_FAIL",
    "shared_generic_body=1",
    "exact_type_body=0",
    "type_specific_content_gap=1",
):
    if marker not in grenade:
        errors.append(f"grenade source missing fail-honest type identity marker: {marker}")

if 'require(gameplay, "PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_READY"' not in evidence:
    errors.append("strict runtime evidence does not require successful authored grenade type identity material load")
if 'forbid(gameplay, "PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_FAIL"' not in evidence:
    errors.append("strict runtime evidence does not reject grenade type identity material failure")
if '"GRENADE_TYPE_IDENTITY_MATERIAL=PASS\\n"' not in evidence:
    errors.append("strict runtime evidence output does not record grenade type identity material PASS")
if "exact per-type body content remains explicit" not in evidence:
    errors.append("strict evidence no longer preserves the exact per-type grenade body content gap")

if errors:
    print("PASS45 GRENADE TYPE RUNTIME EVIDENCE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 GRENADE TYPE RUNTIME EVIDENCE: PASS")
print("- authored frag/smoke/flash identity material readiness is mandatory runtime evidence")
print("- identity material load failure is fatal to automated acceptance")
print("- exact per-type grenade bodies remain an explicit content gap")
print("STATUS: SOURCE CONTRACT ONLY; direct UE 5.8 visual acceptance remains pending")
