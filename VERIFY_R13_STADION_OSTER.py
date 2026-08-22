from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

STADIUM_CPP = SRC / "Private/OCR13StadiumSurfaceSubsystem.cpp"
STADIUM_HEADER = SRC / "Public/OCR13StadiumSurfaceSubsystem.h"
LEGACY_CPP = SRC / "Private/OCR13MuseumStadiumPhotoFidelitySubsystem.cpp"
LEGACY_HEADER = SRC / "Public/OCR13MuseumStadiumPhotoFidelitySubsystem.h"
GEO_CPP = SRC / "Private/OCGeoReference.cpp"
WORLD_CPP = SRC / "Private/OCWorldSectorOster.cpp"
RUNTIME_CPP = SRC / "Private/OCR13StadiumRuntimeValidationSubsystem.cpp"
RUNTIME_HEADER = SRC / "Public/OCR13StadiumRuntimeValidationSubsystem.h"
REFERENCE_INDEX = ROOT / "REFERENCE_PHOTOS/stadion_oster/INDEX.md"
REFERENCE_ARCHIVE = ROOT / "REFERENCE_PHOTOS/stadion_oster/stadion_oster_reference_pack_2026-08-20.zip"
STATUS = ROOT / "STADION_OSTER_IMPLEMENTATION_STATUS.md"
LAUNCHER = ROOT / "RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd"


def fail(message: str) -> None:
    raise SystemExit(f"R13 STADION OSTER VERIFY FAIL: {message}")


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        fail(f"{where}: missing {needle!r}")


stadium_cpp = read(STADIUM_CPP)
stadium_header = read(STADIUM_HEADER)
legacy_cpp = read(LEGACY_CPP)
legacy_header = read(LEGACY_HEADER)
geo_cpp = read(GEO_CPP)
world_cpp = read(WORLD_CPP)
runtime_cpp = read(RUNTIME_CPP)
runtime_header = read(RUNTIME_HEADER)
reference_index = read(REFERENCE_INDEX)
status = read(STATUS)
launcher = read(LAUNCHER)

# Reference payload integrity is explicit. A corrupt/missing archive is never silently accepted.
if REFERENCE_ARCHIVE.exists():
    if REFERENCE_ARCHIVE.stat().st_size <= 0:
        fail("canonical stadium reference archive is empty")
    if not zipfile.is_zipfile(REFERENCE_ARCHIVE):
        require(reference_index, "Integrity status: `RESTORE_REQUIRED`", "reference archive integrity status")
else:
    require(reference_index, "Integrity status: `RESTORE_REQUIRED`", "missing reference archive status")

for needle in (
    "Authoritative presentation owner for the hard-georeferenced Stadion Oster site",
    "void ApplyStadiumSurface(UWorld& World);",
):
    require(stadium_header, needle, "stadium header")

for needle in (
    'return { TEXT("StadionOster"), 50.949360, 30.884660, EOCReferenceConfidence::A,',
    "Canonical hard-georeferenced stadium center",
):
    require(geo_cpp, needle, "geo reference")

for needle in (
    "constexpr float FieldYawDegrees = -21.5f;",
    "constexpr float FieldLengthCm = 10500.0f;",
    "constexpr float FieldWidthCm = 6800.0f;",
    "FOCGeoReference::Stadium();",
    "SCENE_QUERY_STAT(OCStadiumGround)",
    "World.LineTraceSingleByChannel",
    'TEXT("R13_StadionOsterAuthoritative")',
    'TEXT("R13_StadionOsterSiteRoot")',
    'TEXT("StadionOsterMainPitch")',
    'TEXT("StadionOsterRunningSurface")',
    'TEXT("StadionOsterPitchLines")',
    'TEXT("StadionOsterSportsMetal")',
    'TEXT("StadionOsterFootpaths")',
    'TEXT("StadionOsterEntranceBlue")',
    'TEXT("StadionOsterEntranceYellow")',
    'TEXT("StadionOsterEntranceText")',
    'TEXT("СТАДІОН ОСТЕР")',
    'TEXT("StadionOsterHouses01")',
    'TEXT("StadionOsterHouses02")',
    'TEXT("StadionOsterTrees01")',
    'TEXT("StadionOsterTrees04")',
    'TEXT("StadionOsterFences01")',
    'TEXT("StadionOsterFences03")',
    "RemoveInstancesNear",
    "LegacyFenceCleanupRadiusCm",
    "AddPathPolyline",
):
    require(stadium_cpp, needle, "authoritative stadium owner")

for forbidden in (
    "StadiumDelaySeconds",
    "AOCWorldSectorOster::StadiumAnchor()",
    'TEXT("StadionOsterGrassApron")',
    "SiteActor->SetActorEnableCollision(false);",
):
    if forbidden in stadium_cpp:
        fail(f"authoritative stadium owner contains obsolete/unsafe pattern {forbidden!r}")

if stadium_cpp.count("World.SpawnActor<AActor>") != 1:
    fail("authoritative stadium owner must spawn exactly one site actor")

for needle in (
    "StadiumGeometry->SetVisibility(false, true);",
    "StadiumGeometry->SetHiddenInGame(true, true);",
    "StadiumDetails->SetVisibility(false, true);",
    "StadiumDetails->SetHiddenInGame(true, true);",
):
    require(world_cpp, needle, "synchronous legacy stadium handoff")

for needle in (
    "Retired R13.6 compatibility shim",
    "intentionally owns no museum or stadium presentation",
):
    require(legacy_header, needle, "retired legacy header")
for needle in (
    "Retired compatibility subsystem",
    "schedules nothing and creates no geometry",
    "ApplyPhotoFidelity(InWorld);",
):
    require(legacy_cpp, needle, "retired legacy cpp")
for forbidden in ("PhotoFidelityDelaySeconds", "BuildStadium(", "SuppressLegacyStadiumPresentation(", "World.SpawnActor"):
    if forbidden in legacy_cpp:
        fail(f"retired legacy subsystem still owns stadium work: {forbidden!r}")

for index in range(1, 18):
    require(reference_index, f"{index:02d}_", "17-frame reference index")
for needle in ("CANONICAL_REFERENCE_SET", "50.94936", "30.88466"):
    require(reference_index, needle, "stadium reference index")

for needle in (
    "UOCR13StadiumRuntimeValidationSubsystem",
    "UTickableWorldSubsystem",
    "void FailValidation(const FString& Reason);",
):
    require(runtime_header, needle, "Pass 9 runtime validator header")

for needle in (
    "PASS9_STADION_OSTER_RUNTIME_FAIL",
    "PASS9_STADION_OSTER_READY",
    'TEXT("R13_StadionOsterAuthoritative")',
    'TEXT("StadionOsterMainPitch")',
    'TEXT("StadionOsterRunningSurface")',
    'TEXT("StadionOsterPitchLines")',
    'TEXT("StadionOsterSportsMetal")',
    'TEXT("StadionOsterFootpaths")',
    'TEXT("StadionOsterEntranceText")',
    'TEXT("StadionOsterHouses01")',
    'TEXT("StadionOsterTrees01")',
    'TEXT("StadionOsterFences01")',
    'TEXT("StadionOsterGrassApron")',
    "pitch_georef_error_cm_",
    "pitch_terrain_z_error_cm_",
    "legacy_visible_",
    "World->LineTraceSingleByChannel",
):
    require(runtime_cpp, needle, "Pass 9 runtime validator")

for needle in (
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "PASS9_STADION_OSTER_RUNTIME_FAIL",
    "PASS9_STADION_OSTER_READY",
    "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "Pass 9 Windows launcher")

for needle in (
    "Статус: `SOURCE VERIFIED / RUNTIME EVIDENCE PENDING`",
    "Історичний Draft PR `#14`: `CLOSED / SUPERSEDED`",
    "RUN_R14_STADION_RUNTIME_ACCEPTANCE.cmd",
    "PASS9_STADION_OSTER_READY",
    "UE 5.8 compile/build",
):
    require(status, needle, "implementation status")

print("R13 STADION OSTER VERIFY PASS")
print("- canonical hard-georeferenced stadium owner is present")
print("- terrain Z snap is required and obsolete giant grass apron is forbidden")
print("- legacy stadium presentation ownership is retired")
print("- 17-frame reference index and explicit payload integrity state are present")
print("- Pass 9 runtime evidence validates site components, georef XY, terrain Z and legacy visibility")
print("- strict Windows acceptance launcher requires PASS9_STADION_OSTER_READY")
print("STATUS: SOURCE VERIFIED / RUNTIME EVIDENCE PENDING")
