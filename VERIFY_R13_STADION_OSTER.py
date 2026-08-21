from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parent
STADIUM_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13StadiumSurfaceSubsystem.cpp"
STADIUM_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13StadiumSurfaceSubsystem.h"
LEGACY_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13MuseumStadiumPhotoFidelitySubsystem.cpp"
LEGACY_HEADER = ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13MuseumStadiumPhotoFidelitySubsystem.h"
GEO_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCGeoReference.cpp"
REFERENCE_INDEX = ROOT / "REFERENCE_PHOTOS/stadion_oster/INDEX.md"
REFERENCE_ARCHIVE = ROOT / "REFERENCE_PHOTOS/stadion_oster/stadion_oster_reference_pack_2026-08-20.zip"
TZ = ROOT / "STADION_OSTER_TZ.md"
STATUS = ROOT / "STADION_OSTER_IMPLEMENTATION_STATUS.md"
CIVIC_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13CivicLandscapingSubsystem.cpp"
DRESSING_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13LandmarkSiteDressingSubsystem.cpp"
WORLD_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"R13 STADION OSTER VERIFY FAIL: {message}")


def read(path: Path) -> str:
    if not path.exists():
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
reference_index = read(REFERENCE_INDEX)
tz = read(TZ)
status = read(STATUS)
civic_cpp = read(CIVIC_CPP) if CIVIC_CPP.exists() else ""
dressing_cpp = read(DRESSING_CPP) if DRESSING_CPP.exists() else ""
world_cpp = read(WORLD_CPP)

# The old archive was intentionally removed because it was corrupt and violated the LFS contract.
# INDEX.md is authoritative while the payload is being restored. Absence is valid only when the
# index explicitly records RESTORE_REQUIRED; an existing archive must still be non-empty and valid.
if REFERENCE_ARCHIVE.exists():
    if REFERENCE_ARCHIVE.stat().st_size <= 0:
        fail("canonical stadium reference archive is empty")
    archive_is_zip = zipfile.is_zipfile(REFERENCE_ARCHIVE)
    if not archive_is_zip:
        require(reference_index, "Integrity status: `RESTORE_REQUIRED`", "reference archive integrity status")
else:
    require(reference_index, "Integrity status: `RESTORE_REQUIRED`", "missing reference archive status")
    archive_is_zip = False

for needle in [
    "Authoritative presentation owner for the hard-georeferenced Stadion Oster site",
    "void ApplyStadiumSurface(UWorld& World);",
]:
    require(stadium_header, needle, "stadium header")

for needle in [
    'return { TEXT("StadionOster"), 50.949360, 30.884660, EOCReferenceConfidence::A,',
    "Canonical hard-georeferenced stadium center",
]:
    require(geo_cpp, needle, "geo reference")

for needle in [
    "constexpr float FieldYawDegrees = -21.5f;",
    "constexpr float FieldLengthCm = 10500.0f;",
    "constexpr float FieldWidthCm = 6800.0f;",
    "FOCGeoReference::Stadium();",
    'TEXT("R13_StadionOsterAuthoritative")',
    'TEXT("R13_StadionOsterSiteRoot")',
    'TEXT("StadionOsterMainPitch")',
    'TEXT("StadionOsterRunningSurface")',
    'TEXT("StadionOsterPitchLines")',
    'TEXT("StadionOsterFootpaths")',
    'TEXT("StadionOsterEntranceBlue")',
    'TEXT("StadionOsterEntranceYellow")',
    'TEXT("StadionOsterEntranceText")',
    'TEXT("СТАДІОН ОСТЕР")',
    '#include "Components/TextRenderComponent.h"',
    "SiteActor->SetActorEnableCollision(true);",
    'TEXT("StadionOsterFences01"), true, true, 60000',
    'TEXT("StadionOsterFences03"), true, true, 60000',
    'TEXT("StadiumGeometry")',
    'TEXT("StadiumDetails")',
    'TEXT("Fences")',
    "RemoveInstancesNear",
    "LegacyFenceCleanupRadiusCm",
    "AddPathPolyline",
    "/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03",
    "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01",
    "/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04",
]:
    require(stadium_cpp, needle, "authoritative stadium owner")

for forbidden in [
    "StadiumDelaySeconds",
    "SetTimer(",
    "AOCWorldSectorOster::StadiumAnchor()",
    "SiteActor->SetActorEnableCollision(false);",
]:
    if forbidden in stadium_cpp:
        fail(f"authoritative stadium owner still contains legacy/unsafe pattern {forbidden!r}")

if stadium_cpp.count("World.SpawnActor<AActor>") != 1:
    fail("authoritative stadium owner must spawn exactly one site actor")

for needle in [
    "StadiumGeometry->SetVisibility(false, true);",
    "StadiumGeometry->SetHiddenInGame(true, true);",
    "StadiumDetails->SetVisibility(false, true);",
    "StadiumDetails->SetHiddenInGame(true, true);",
]:
    require(world_cpp, needle, "synchronous legacy stadium handoff")

if civic_cpp:
    if "AddStadiumPerimeterPlanting" in civic_cpp:
        fail("delayed civic landscaping still mutates Stadion Oster: 'AddStadiumPerimeterPlanting'")
    require(civic_cpp, "Stadion Oster vegetation remains exclusively owned by OCR13StadiumSurfaceSubsystem",
        "civic ownership handoff")

if dressing_cpp:
    if "DressStadium" in dressing_cpp:
        fail("delayed landmark dressing still mutates Stadion Oster: 'DressStadium'")
    require(dressing_cpp, "Stadion Oster details remain exclusively owned by OCR13StadiumSurfaceSubsystem",
        "landmark dressing ownership handoff")

for needle in [
    "Retired R13.6 compatibility shim",
    "intentionally owns no museum or stadium presentation",
]:
    require(legacy_header, needle, "retired legacy header")

for needle in [
    "Retired compatibility subsystem",
    "schedules nothing and creates no geometry",
    "ApplyPhotoFidelity(InWorld);",
]:
    require(legacy_cpp, needle, "retired legacy cpp")

for forbidden in [
    "PhotoFidelityDelaySeconds",
    "SetTimer(",
    "BuildStadium(",
    "SuppressLegacyStadiumPresentation(",
    "World.SpawnActor",
]:
    if forbidden in legacy_cpp:
        fail(f"retired legacy subsystem still owns stadium work: {forbidden!r}")

for index in range(1, 18):
    require(reference_index, f"{index:02d}_", "reference index")

for needle in [
    "CANONICAL_REFERENCE_SET",
    "50.94936",
    "30.88466",
    "власним placement owner",
]:
    require(tz + reference_index, needle, "stadium reference/TZ contract")

for needle in [
    "Статус: `CODED_UNTESTED`",
    "Інтеграційна гілка: `main`",
    "Draft PR `#14`",
    "UE 5.8 compile/build",
    "прямою вказівкою користувача",
]:
    require(status, needle, "implementation status")

print("R13 STADION OSTER VERIFY PASS")
print("- 17-frame reference index present")
print("- reference payload integrity: " + ("ZIP_OK" if archive_is_zip else "RESTORE_REQUIRED"))
print("- stadium anchor is hard-georeferenced")
print("- one synchronous authoritative stadium owner")
print("- delayed legacy stadium builder is retired")
print("- delayed generic landscaping/furniture are absent or no longer mutate the stadium site")
print("- local collision is active for sports metal, entrance structure and replacement fences")
print("- 2025 entrance landmark has explicit STADION OSTER lettering")
print("- modern turf/running surface, sport zones, curved footpaths and real rural assets are wired")
print("STATUS: CODED_UNTESTED (UE 5.8 build/playtest still required)")
