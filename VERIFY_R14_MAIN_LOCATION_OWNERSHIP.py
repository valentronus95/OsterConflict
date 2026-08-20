from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
PRIVATE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"

required_files = [
    PRIVATE / "OCR137MuseumPhotoModelSubsystem.cpp",
    PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp",
    PRIVATE / "OCR140SilpoPhotoModelSubsystem.cpp",
    PRIVATE / "OCR141SilpoDetailSubsystem.cpp",
    PRIVATE / "OCR142SilpoInteriorDetailSubsystem.cpp",
    PRIVATE / "OCR143SilpoFacadeIdentitySubsystem.cpp",
    PRIVATE / "OCGeoReference.cpp",
    ROOT / "START_HERE.cmd",
    ROOT / "RUN_R14_MAIN_SANDBOX_TEST.cmd",
]

forbidden_legacy_files = [
    PRIVATE / "OCR13SilpoPhotoModelSubsystem.cpp",
    PRIVATE / "OCR13CultureHousePhotoModelSubsystem.cpp",
]

errors: list[str] = []

for path in required_files:
    if not path.is_file():
        errors.append(f"missing current R14 file: {path.relative_to(ROOT)}")

for path in forbidden_legacy_files:
    if path.exists():
        errors.append(f"legacy mixed-location owner returned: {path.relative_to(ROOT)}")

geo_path = PRIVATE / "OCGeoReference.cpp"
if geo_path.is_file():
    geo = geo_path.read_text(encoding="utf-8")
    required_geo_tokens = {
        "Museum canonical anchor": "50.948239, 30.883865",
        "Silpo canonical anchor": "50.948833799986254, 30.87572244094098",
        "Stadium canonical anchor": "50.949360, 30.884660",
    }
    for label, token in required_geo_tokens.items():
        if token not in geo:
            errors.append(f"{label} missing or changed: {token}")

silpo_path = PRIVATE / "OCR140SilpoPhotoModelSubsystem.cpp"
if silpo_path.is_file():
    silpo = silpo_path.read_text(encoding="utf-8")
    for token in (
        "FOCGeoReference::Silpo()",
        'TEXT("R140_SilpoPhotoModel")',
        'TEXT("SilpoOster_BohdanaKhmelnytskoho54")',
    ):
        if token not in silpo:
            errors.append(f"R14 Silpo ownership token missing: {token}")

launcher_path = ROOT / "START_HERE.cmd"
if launcher_path.is_file():
    launcher = launcher_path.read_text(encoding="utf-8", errors="replace")
    if "R14 CURRENT MAIN" not in launcher:
        errors.append("START_HERE.cmd is not marked as R14 CURRENT MAIN")
    if "Launch CURRENT R14 main Sandbox location test" not in launcher:
        errors.append("START_HERE.cmd does not route location playtest to current R14 main")
    if "Launch R11 local listen-server visual test" in launcher:
        errors.append("START_HERE.cmd regressed to the legacy R11 playtest route")

if errors:
    print("R14 MAIN LOCATION OWNERSHIP: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    sys.exit(1)

print("R14 MAIN LOCATION OWNERSHIP: PASS")
print("Museum, Silpo and Stadium use the current R14 integration tree.")
print("Legacy R13 Silpo/Culture House photo-model owners are absent.")
