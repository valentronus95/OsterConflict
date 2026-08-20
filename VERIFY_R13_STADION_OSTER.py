from pathlib import Path

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

if not REFERENCE_ARCHIVE.exists():
    fail("missing canonical stadium reference archive")
if REFERENCE_ARCHIVE.stat().st_size <= 0:
    fail("canonical stadium reference archive is empty")

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
]:
    if forbidden in stadium_cpp:
        fail(f"authoritative stadium owner still contains legacy/delayed pattern {forbidden!r}")

if stadium_cpp.count("World.SpawnActor<AActor>") != 1:
    fail("authoritative stadium owner must spawn exactly one site actor")

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
    "exactly one placement owner",
]:
    require(tz + reference_index, needle, "stadium reference/TZ contract")

for needle in [
    "Статус: `CODED_UNTESTED`",
    "Draft PR: `#14`",
    "UE 5.8 compile/build",
    "PR #14 залишається Draft",
]:
    require(status, needle, "implementation status")

print("R13 STADION OSTER VERIFY PASS")
print("- canonical 17-frame reference set present")
print("- stadium anchor is hard-georeferenced")
print("- one synchronous authoritative stadium owner")
print("- delayed legacy stadium builder is retired")
print("- legacy stadium visuals/local shared fences are cleaned without global fence removal")
print("- modern turf/running surface, sport zones, curved footpaths and real rural assets are wired")
print("STATUS: CODED_UNTESTED (UE 5.8 build/playtest still required)")