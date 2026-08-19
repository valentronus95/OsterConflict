from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13VerifiedOsterGeographySubsystem.h"
CPP = SRC / "Private" / "OCR13VerifiedOsterGeographySubsystem.cpp"
SOURCE = SRC / "Private" / "OCWorldSectorOster.cpp"
LEGACY = SRC / "Private" / "OCKrushelnytskaVisualSliceSubsystem.cpp"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"
S01_H = SRC / "Public" / "OCLocationSectorS01Data.h"
S01_CPP = SRC / "Private" / "OCLocationSectorS01Data.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY FAIL: " + message)


for path in (H, CPP, SOURCE, LEGACY, GEO_H, GEO_CPP, S01_H, S01_CPP):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
source = SOURCE.read_text(encoding="utf-8", errors="replace")
legacy = LEGACY.read_text(encoding="utf-8", errors="replace")
geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
s01_h = S01_H.read_text(encoding="utf-8", errors="replace")
s01_cpp = S01_CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain final geography header include")

for token in [
    "LegacySlicePurgeDelaySeconds = 1.82f",
    "SuppressLegacyNearSpawnSlice",
    'Value.StartsWith(TEXT("R12_House"))',
    'Value.StartsWith(TEXT("R12_Fence"))',
    'Value.StartsWith(TEXT("R12_Tree"))',
    'Value.StartsWith(TEXT("R13_KrushelnytskaGrass"))',
    'Value.StartsWith(TEXT("R13_KrushelnytskaPine"))',
    'Value.StartsWith(TEXT("R13_KrushelnytskaUtility"))',
    "Component->DestroyComponent();",
    "GameMode->IsFrontendOnlySession()",
    "no geography was relocated",
    "FVector UOCR13VerifiedOsterGeographySubsystem::VerifiedStadiumAnchor()",
    "return AOCWorldSectorOster::StadiumAnchor();",
]:
    if token not in cpp:
        fail(f"migration cleanup marker missing: {token}")

for token in [
    "static FVector VerifiedStadiumAnchor();",
    "Permanent geography belongs to FOCGeoReference",
]:
    if token not in h:
        fail(f"read-only geography contract missing: {token}")

for token in [
    "static FOCGeoReferencePoint Stadium();",
    "static FOCGeoReferencePoint SolonynaEstatePark();",
]:
    if token not in geo_h:
        fail(f"canonical geo declaration missing: {token}")
for token in [
    'TEXT("StadionOster")', "50.94936", "30.88466",
    'TEXT("SolonynaEstatePark")', "EOCReferenceConfidence::B",
    "~140 m east of museum", "~130 m SE of stadium",
]:
    if token not in geo_cpp:
        fail(f"canonical museum-site geo marker missing: {token}")
for token in [
    "FVector AOCWorldSectorOster::StadiumAnchor()",
    "const FOCGeoReferencePoint Ref = FOCGeoReference::Stadium();",
    "FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ)",
    "FOCLocationSectorS01Data::ProvisionalResidentialPlots()",
    "FOCLocationSectorS01Data::ProvisionalFrontages()",
    "FOCLocationSectorS01Data::ProvisionalServiceRoads()",
]:
    if token not in source:
        fail(f"source world does not consume canonical/registry geography: {token}")

# S01 is now explicit registry data, not legacy slot arithmetic.
for token in [
    "FOCS01ResidentialPlotSeed",
    "FOCS01FrontageSeed",
    "FOCS01RoadSeed",
    "ProvisionalResidentialPlots()",
    "ProvisionalFrontages()",
    "ProvisionalServiceRoads()",
]:
    if token not in s01_h + s01_cpp:
        fail(f"S01 registry contract missing: {token}")
if s01_cpp.count('TEXT("S01_KR_W_') != 8 or s01_cpp.count('TEXT("S01_KR_E_') != 8:
    fail("S01 registry must keep eight west and eight east addressable plots")
if s01_cpp.count('TEXT("S01_KR_FRONT_') != 8:
    fail("S01 registry must keep eight explicit frontages")
for token in ['TEXT("S01_KR_SERVICE_W")', 'TEXT("S01_KR_SERVICE_E")']:
    if token not in s01_cpp:
        fail(f"S01 service-road registry missing: {token}")

for token in [
    "StreetCenterX = -3400.0f",
    "keep the first visual slice around the normal gameplay spawn",
]:
    if token not in legacy:
        fail(f"deprecated near-spawn slice signature unexpectedly changed: {token}")

# Old direct arithmetic must not return to the authoritative world builder.
for forbidden in [
    "WestHouseX", "EastHouseX", "const float StartY = 20500.0f",
    "static_cast<float>(Slot) * 4800.0f",
]:
    if forbidden in source:
        fail(f"legacy S01 arithmetic returned to source world: {forbidden}")

# Migration cleanup must never become a second permanent geography author.
for forbidden in [
    "LegacyStadiumAnchor", "RelocateStadiumPresentation", "UpdateInstanceTransform",
    "SpawnActor", "SetActorLocation", "FOCGeoReference::ToLocalCm(50.94936",
]:
    if forbidden in cpp:
        fail(f"migration cleanup regained permanent geography ownership: {forbidden}")

print("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY: PASS")
print("Checks early purge of the fake near-spawn slice, explicit S01 registry ownership, canonical stadium/Solonyna-estate references and read-only geography compatibility for late presentation passes.")
