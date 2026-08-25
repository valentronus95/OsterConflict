#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CONTENT = ROOT / "OsterConflict" / "Content"
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
REGISTRY = ROOT / "R14_MODEL_REGISTRY.md"
PREFLIGHT = ROOT / "VERIFY_PASS45_REQUIRED_LOCAL_CONTENT.py"
STRICT = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
RUNTIME_VALIDATOR = SRC / "OCProductionWeaponRuntimeValidationSubsystem.cpp"
VARIANTS = SRC / "OCWeaponVariants.cpp"
errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


registry = read(REGISTRY)
preflight = read(PREFLIGHT)
strict = read(STRICT)
runtime = read(RUNTIME_VALIDATOR)
variants = read(VARIANTS)

expected = (
    (
        "Remington 870",
        CONTENT / "Production" / "Weapons" / "Remington870" / "SM_Remington870.uasset",
        "/Game/Production/Weapons/Remington870/SM_Remington870",
        "SM_Remington870.SM_Remington870",
    ),
    (
        "M249",
        CONTENT / "Production" / "Weapons" / "M249" / "SM_M249.uasset",
        "/Game/Production/Weapons/M249/SM_M249",
        "SM_M249.SM_M249",
    ),
)

for label, file_path, object_path, object_name in expected:
    # Current repository truth. When a legitimate authored asset is later added, this gate is expected
    # to fail until registry/status/preflight are updated in the same coherent change.
    req(not file_path.exists(),
        f"{label} production asset now exists; update Pass45 content-gap truth and runtime acceptance contract coherently")
    req(f"| {label} | `{object_path}` | Static | **CONTENT GAP / NOT READY**" in registry,
        f"registry does not explicitly mark {label} CONTENT GAP / NOT READY")
    req(object_path in preflight, f"strict local preflight does not check {label} canonical path")
    req(file_path.name in preflight, f"strict local preflight does not check {label} uasset filename")
    req(object_name in variants, f"runtime weapon variant no longer points to {label} canonical production object")
    req(object_name in runtime, f"runtime material validator no longer expects {label} canonical production object")

req('VERIFY_PASS45_REQUIRED_LOCAL_CONTENT.py' in strict,
    "strict acceptance no longer invokes required local content preflight")
req('This is a CONTENT GAP, not a runtime-code failure.' in strict,
    "strict acceptance no longer classifies missing required weapon payload as content gap")
req('SUMMARY=11/11 production weapon classes PASS' in read(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"),
    "final Pass45 evidence no longer requires all 11 production weapon classes")
req('No generated grey/default substitute is accepted as closure.' in registry,
    "asset registry no longer forbids fake substitute closure")

if errors:
    print("PASS45 WEAPON CONTENT GAP TRUTH: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON CONTENT GAP TRUTH: PASS")
print("- Remington 870 canonical production uasset is absent and explicitly NOT READY")
print("- M249 canonical production uasset is absent and explicitly NOT READY")
print("- runtime code/validator still point to the canonical target paths")
print("- strict final acceptance fails fast before gameplay when either local asset is missing")
print("- final automated evidence still requires 11/11 weapon classes")
print("STATUS: explicit CONTENT GAP; no generated/default substitute accepted")
