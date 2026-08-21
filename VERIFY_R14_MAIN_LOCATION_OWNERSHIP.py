from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PRIVATE = SOURCE / "Private"
PUBLIC = SOURCE / "Public"

required_files = [
    PRIVATE / "OCR137MuseumPhotoModelSubsystem.cpp",
    PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp",
    PRIVATE / "OCR140SilpoPhotoModelSubsystem.cpp",
    PRIVATE / "OCR141SilpoDetailSubsystem.cpp",
    PRIVATE / "OCR142SilpoInteriorDetailSubsystem.cpp",
    PRIVATE / "OCR143SilpoFacadeIdentitySubsystem.cpp",
    PRIVATE / "OCR146CultureHousePhotoModelSubsystem.cpp",
    PRIVATE / "OCR146LandmarkSeparationSubsystem.cpp",
    PRIVATE / "OCGeoReference.cpp",
    PUBLIC / "OCR146CultureHousePhotoModelSubsystem.h",
    PUBLIC / "OCR146LandmarkSeparationSubsystem.h",
    PUBLIC / "OCGeoReference.h",
    ROOT / "START_HERE.cmd",
    ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd",
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
        "Culture House Hranovskoho 3 map anchor": "50.948694, 30.881435",
    }
    for label, token in required_geo_tokens.items():
        if token not in geo:
            errors.append(f"{label} missing or changed: {token}")
    if "FOCGeoReferencePoint FOCGeoReference::CultureHouse()" not in geo:
        errors.append("dedicated CultureHouse() geo owner is missing")

geo_header_path = PUBLIC / "OCGeoReference.h"
if geo_header_path.is_file():
    geo_header = geo_header_path.read_text(encoding="utf-8")
    if "static FOCGeoReferencePoint CultureHouse();" not in geo_header:
        errors.append("CultureHouse() is missing from OCGeoReference public contract")

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

culture_path = PRIVATE / "OCR146CultureHousePhotoModelSubsystem.cpp"
if culture_path.is_file():
    culture = culture_path.read_text(encoding="utf-8")
    for token in (
        "FOCGeoReference::CultureHouse()",
        'TEXT("R146_CultureHouseAuthoritative")',
        'TEXT("CultureHouseOster_Hranovskoho3")',
        "CultureHouseStartupDelaySeconds = 0.28f",
    ):
        if token not in culture:
            errors.append(f"R14.6 Culture House ownership token missing: {token}")
    if "FOCGeoReference::Museum()" in culture or "FOCGeoReference::Silpo()" in culture:
        errors.append("Culture House owner references Museum/Silpo geo owners")

separation_path = PRIVATE / "OCR146LandmarkSeparationSubsystem.cpp"
if separation_path.is_file():
    separation = separation_path.read_text(encoding="utf-8")
    for token in (
        "FOCGeoReference::Museum()",
        "FOCGeoReference::Silpo()",
        "FOCGeoReference::CultureHouse()",
        'Name == TEXT("Buildings")',
        'Name == TEXT("LandmarkBlocks")',
        "FOCGeoReference::CultureParkNorth()",
        "SyntheticParkLinkMid",
        'ActorHasTag(TEXT("R13_CultureHousePhotoModel"))',
        'ActorHasTag(TEXT("R13_SilpoPhotoModel"))',
    ):
        if token not in separation:
            errors.append(f"R14.6 separation guard token missing: {token}")

launcher_path = ROOT / "START_HERE.cmd"
if launcher_path.is_file():
    launcher = launcher_path.read_text(encoding="utf-8", errors="replace")
    if "CURRENT MAIN" not in launcher or "R14" not in launcher:
        errors.append("START_HERE.cmd is not marked as current R14 main")
    if "R14.7" not in launcher:
        errors.append("START_HERE.cmd is not marked as the current R14.7 launcher")
    if 'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' not in launcher:
        errors.append("START_HERE.cmd does not route normal playtest to RUN_R14_CURRENT_GAMEPLAY.cmd")
    if 'call "%~dp0RUN_R14_MAIN_SANDBOX_TEST.cmd"' not in launcher:
        errors.append("START_HERE.cmd does not retain the R14 sandbox diagnostic route")
    if "Launch R11 local listen-server visual test" in launcher:
        errors.append("START_HERE.cmd regressed to the legacy R11 playtest route")

if errors:
    print("R14 MAIN LOCATION OWNERSHIP: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    sys.exit(1)

print("R14 MAIN LOCATION OWNERSHIP: PASS")
print("Museum, Silpo, Culture House and Stadium are bound to separate current-main site owners.")
print("Culture House uses Hranovskoho 3 and cannot inherit Museum/Silpo coordinates.")
print("Normal gameplay and Sandbox diagnostics are separate launch routes.")
print("Legacy R13 Silpo/Culture House photo-model owners are absent and synthetic north-civic map geometry is guarded.")
