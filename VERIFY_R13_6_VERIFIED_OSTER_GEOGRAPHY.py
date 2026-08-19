from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13VerifiedOsterGeographySubsystem.h"
CPP = SRC / "Private" / "OCR13VerifiedOsterGeographySubsystem.cpp"
SOURCE = SRC / "Private" / "OCWorldSectorOster.cpp"
LEGACY = SRC / "Private" / "OCKrushelnytskaVisualSliceSubsystem.cpp"
GEO_H = SRC / "Public" / "OCGeoReference.h"
GEO_CPP = SRC / "Private" / "OCGeoReference.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY FAIL: " + message)


for path in (H, CPP, SOURCE, LEGACY, GEO_H, GEO_CPP):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
source = SOURCE.read_text(encoding="utf-8", errors="replace")
legacy = LEGACY.read_text(encoding="utf-8", errors="replace")
geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain final geography header include")

# Migration subsystem is intentionally narrow: purge the obsolete near-spawn visual shortcut before styling.
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

# Canonical public-map stadium coordinate lives only in FOCGeoReference and is consumed by the source world anchor.
if "static FOCGeoReferencePoint Stadium();" not in geo_h:
    fail("canonical stadium geo declaration missing")
for token in ['TEXT("StadionOster")', "50.94936", "30.88466"]:
    if token not in geo_cpp:
        fail(f"canonical stadium geo definition missing: {token}")
for token in [
    "FVector AOCWorldSectorOster::StadiumAnchor()",
    "const FOCGeoReferencePoint Ref = FOCGeoReference::Stadium();",
    "FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, GroundTopZ)",
]:
    if token not in source:
        fail(f"source world does not consume canonical stadium reference: {token}")

# Keep the real source Krushelnytska corridor distinct from the deprecated near-spawn art shortcut.
for token in [
    "WestHouseX = -39200.0f",
    "EastHouseX = -27800.0f",
    "StartY = 20500.0f",
    "BuildSolomiiKrushelnytskoiStreet()",
]:
    if token not in source:
        fail(f"source Krushelnytska geography marker missing: {token}")
for token in [
    "StreetCenterX = -3400.0f",
    "keep the first visual slice around the normal gameplay spawn",
]:
    if token not in legacy:
        fail(f"legacy near-spawn slice signature unexpectedly changed: {token}")

# This subsystem must never become a second geography author again.
for forbidden in [
    "LegacyStadiumAnchor",
    "RelocateStadiumPresentation",
    "UpdateInstanceTransform",
    "SpawnActor",
    "SetActorLocation",
    "FOCGeoReference::ToLocalCm(50.94936",
    "30.88466, 0.0",
]:
    if forbidden in cpp:
        fail(f"migration cleanup regained permanent geography ownership: {forbidden}")

print("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY: PASS")
print("Checks early purge of the fake near-spawn Krushelnytska slice, canonical FOCGeoReference/AOCWorldSectorOster stadium ownership and read-only compatibility access for late presentation passes.")
