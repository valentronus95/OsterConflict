#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"
BINDINGS = ROOT / "PASS45_REFERENCE_PACK_BINDINGS.md"
PACK_ROOT = ROOT / "_DOCS" / "REFERENCE_PACKS" / "LOC_MUSEUM_001_OSTER_MUSEUM"
REF = PACK_ROOT / "REFERENCE_SPEC.md"
AUDIT = PACK_ROOT / "CURRENT_SOURCE_GAP_AUDIT_2026-08-28.md"
GROUNDING = PACK_ROOT / "PUBLIC_GROUNDING_RECONCILIATION_2026-08-28.md"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp"

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
grounding = read(GROUNDING)
source = read(SOURCE)

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

# Public text is grounding, not permission to overwrite the user-photo exterior.
for needle in (
    "вул. Татарівська, 30",
    "wooden house",
    "photo-driven",
    "does **not** authorize replacing the photo-visible brick exterior",
):
    req(needle in grounding, f"Museum public/photo discrepancy note lost required authority rule: {needle}")

# Prevent the most dangerous evidence bleed: three hero landmarks are independent owners.
for needle in ("Museum", "Culture House", "Silpo"):
    req(needle in ref, f"Museum reference separation contract lost landmark: {needle}")
req("different places and different runtime/site owners" in ref,
    "Museum/Culture House/Silpo separation is no longer a hard reference rule")

# Item 32 source continuation: the current museum owner must now carry a conservative reference-backed site slice.
# It remains explicitly provisional and must never impersonate direct UE 5.8 screenshot acceptance.
for needle in (
    "/Game/KiteDemo/LevelContent/Architecture/SM_1Meter_01.SM_1Meter_01",
    "/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_2_Inst.M_Concrete_2_Inst",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
    "R137Museum_AuthoredConcreteApproach",
    "R137Museum_ReferenceConiferCorridor",
    "PASS45_MUSEUM_SITE_REFERENCE_SLICE_READY",
    "path=straight_concrete_pedestrian",
    "slabs=6",
    "conifer_corridor=8",
    "exact_tree_species_claim=0",
    "exact_site_coordinates_claim=0",
    "stadium_owner_duplicated=0",
    "runtime_visual_acceptance=pending",
):
    req(needle in source, f"Museum site reference source slice missing/falsified: {needle}")

req("/Engine/BasicShapes/" not in source,
    "Museum R137 source owner regressed to an Engine BasicShape visual")
req("SM_Stonepath_Var01" not in source,
    "Museum hero approach incorrectly reused the Central Park decorative stone-path family")
req("runtime_visual_acceptance=accepted" not in source and "runtime_photo_acceptance=1" not in source,
    "Museum source code falsely claims runtime visual/photo acceptance")

if errors:
    print("PASS45 MUSEUM REFERENCE CONTRACT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MUSEUM REFERENCE CONTRACT: PASS")
print("- canonical Pass45 TZ remains the execution/status owner")
print("- LOC_MUSEUM_001 is bound as a normative Gate E/K visual-reference appendix")
print("- VERIFIED/PROBABLE/UNKNOWN evidence classes and Museum identity anchors are guarded")
print("- public identity/address grounding cannot overwrite the photo-driven visible exterior")
print("- R137 now owns a narrow authored concrete approach plus a conservative mature-conifer corridor source slice")
print("- the site slice does not claim exact paving dimensions, exact tree species/coordinates, or stadium ownership")
print("- MUS-CAM-01..07 remain mandatory direct UE 5.8 visual evidence")
print("- Museum-adjacent field vs modernized stadium conflict remains fail-visible until explicitly resolved")
print("- Central Park ParkPaths and the Museum hero approach cannot be silently merged")
print("STATUS: REFERENCE/SITE SOURCE CONTRACT GUARDED; UE 5.8 RUNTIME VISUAL ACCEPTANCE REMAINS REQUIRED")
