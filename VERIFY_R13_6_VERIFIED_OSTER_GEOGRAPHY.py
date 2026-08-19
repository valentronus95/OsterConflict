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

for token in [
    "LegacySlicePurgeDelaySeconds = 1.82f",
    "GeographyDelaySeconds = 3.35f",
    "LegacySliceCenterX = -3400.0f",
    "LegacyStadiumAnchor(15000.0f, -1500.0f, 0.0f)",
    "const FOCGeoReferencePoint Ref = FOCGeoReference::Stadium();",
    "FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0)",
    "SuppressLegacyNearSpawnSlice",
    "RemoveLegacySliceResidentialPresentation",
    "RelocateStadiumPresentation",
    'Value.StartsWith(TEXT("R12_House"))',
    'Value.StartsWith(TEXT("R13_KrushelnytskaGrass"))',
    'Value.StartsWith(TEXT("R13_KrushelnytskaUtility"))',
    'Value.StartsWith(TEXT("R13_OsterBrickHouse"))',
    'Name == TEXT("StadiumGeometry")',
    'Name == TEXT("StadiumDetails")',
    'Value.StartsWith(TEXT("R13_Stadium"))',
    "Component->DestroyComponent();",
    "Component->UpdateInstanceTransform",
    "GameMode->IsFrontendOnlySession()",
    "source geo corridor remains authoritative",
]:
    if token not in cpp:
        fail(f"verified geography marker missing: {token}")

if "static FOCGeoReferencePoint Stadium();" not in geo_h:
    fail("canonical stadium geo declaration missing")
for token in [
    'TEXT("StadionOster")',
    "50.94936",
    "30.88466",
]:
    if token not in geo_cpp:
        fail(f"canonical stadium geo definition missing: {token}")

# Migration subsystem must not re-author the public coordinate after FOCGeoReference became the single owner.
for forbidden in [
    "FOCGeoReference::ToLocalCm(50.94936",
    "30.88466, 0.0",
]:
    if forbidden in cpp:
        fail(f"stadium coordinate duplicated outside FOCGeoReference: {forbidden}")

# Keep the real source corridor visibly distinct from the old R12 near-spawn art shortcut.
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

for forbidden in [
    "SpawnActor",
    "SetActorLocation",
]:
    if forbidden in cpp:
        fail(f"geography correction must transform/destroy owned presentation components, not spawn/move gameplay actors: {forbidden}")

print("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY: PASS")
print("Checks early purge of the fake near-spawn Krushelnytska slice, canonical FOCGeoReference stadium ownership, source corridor preservation and migration-only stadium presentation relocation.")
