#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"
BINDINGS = ROOT / "PASS45_REFERENCE_PACK_BINDINGS.md"
REF = ROOT / "_DOCS" / "REFERENCE_PACKS" / "LOC_MUSEUM_001_OSTER_MUSEUM" / "REFERENCE_SPEC.md"
AUDIT = ROOT / "_DOCS" / "REFERENCE_PACKS" / "LOC_MUSEUM_001_OSTER_MUSEUM" / "CURRENT_SOURCE_GAP_AUDIT_2026-08-28.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing required Museum reference-contract file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tz = read(TZ)
bindings = read(BINDINGS)
ref = read(REF)
audit = read(AUDIT)

# Canonical Pass45 remains the execution/status owner; the location pack is a bound subordinate Gate E/K contract.
req("single canonical active TZ for Pass 45" in tz,
    "PASS45_RUNTIME_RECOVERY_TZ.md lost its canonical execution/status ownership marker")
req("PASS45_RUNTIME_RECOVERY_TZ.md" in bindings,
    "reference binding index is not explicitly attached to the canonical Pass45 TZ")
req("_DOCS/REFERENCE_PACKS/LOC_MUSEUM_001_OSTER_MUSEUM/REFERENCE_SPEC.md" in bindings,
    "Museum normative appendix is not bound in PASS45_REFERENCE_PACK_BINDINGS.md")
req("Gate E" in bindings and "Gate K" in bindings,
    "Museum reference pack is not bound to both landmark/environment and visual-fidelity acceptance")
req("REFERENCE BOUND / SOURCE AUDIT REQUIRED / UE 5.8 RUNTIME ACCEPTANCE REQUIRED" in bindings,
    "binding index can no longer distinguish reference binding from runtime acceptance")

# Evidence truth model: no hidden geometry is allowed to become exact merely because a proxy/source test exists.
for needle in ("VERIFIED", "PROBABLE", "UNKNOWN"):
    req(needle in ref, f"Museum reference evidence class missing: {needle}")

for needle in (
    "predominantly **one-storey**",
    "dark charcoal/near-black raised plinth",
    "grey-blue / desaturated blue-grey timber cladding",
    "three front-facing vertically proportioned windows",
    "decorative brick cornice/frieze",
    "repeated pale square inset elements",
    "straight and central",
    "mature tall spruce/conifer trees dominate the main approach",
    "football field is on player-left",
):
    req(needle in ref, f"Museum normative identity contract lost required cue: {needle}")

for idx in range(1, 8):
    req(f"MUS-CAM-0{idx}" in ref, f"Museum runtime screenshot acceptance camera missing: MUS-CAM-0{idx}")

req("Source tests alone cannot close this gate" in ref,
    "Museum pack can incorrectly impersonate runtime visual acceptance")
req("/mnt/data/...` paths" in ref and "non-portable" in ref,
    "Museum reference contract lost the non-portable /mnt/data rule")

# The current source conflict must remain visible until deliberately resolved.
for needle in (
    "MUSEUM-GAP-10",
    "modernized stadium assumptions conflict with the Museum-side pack",
    "Do not merge unrelated path families",
    "MUS-CAM-07",
    "SOURCE CONFLICT OPEN / RUNTIME UNACCEPTED",
):
    req(needle in audit, f"Museum source-gap audit lost required open conflict/ownership truth: {needle}")

# Prevent the most dangerous evidence bleed: three hero landmarks are independent owners.
for needle in ("Museum", "Culture House", "Silpo"):
    req(needle in ref, f"Museum reference separation contract lost landmark: {needle}")
req("different places and different runtime/site owners" in ref,
    "Museum/Culture House/Silpo separation is no longer a hard reference rule")

if errors:
    print("PASS45 MUSEUM REFERENCE CONTRACT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MUSEUM REFERENCE CONTRACT: PASS")
print("- canonical Pass45 TZ remains the execution/status owner")
print("- LOC_MUSEUM_001 is bound as a normative Gate E/K visual-reference appendix")
print("- VERIFIED/PROBABLE/UNKNOWN evidence classes and Museum identity anchors are guarded")
print("- MUS-CAM-01..07 remain mandatory direct UE 5.8 visual evidence")
print("- Museum-adjacent field vs modernized stadium conflict remains fail-visible until explicitly resolved")
print("- Central Park ParkPaths and the Museum hero approach cannot be silently merged")
print("STATUS: REFERENCE CONTRACT SOURCE-GUARDED; UE 5.8 RUNTIME VISUAL ACCEPTANCE REMAINS REQUIRED")
