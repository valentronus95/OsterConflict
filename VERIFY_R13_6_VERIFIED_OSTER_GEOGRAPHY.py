from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13VerifiedOsterGeographySubsystem.h"
CPP = SRC / "Private" / "OCR13VerifiedOsterGeographySubsystem.cpp"
SOURCE = SRC / "Private" / "OCWorldSectorOster.cpp"
LEGACY = SRC / "Private" / "OCKrushelnytskaVisualSliceSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY FAIL: " + message)


for path in (H, CPP, SOURCE, LEGACY):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
source = SOURCE.read_text(encoding="utf-8", errors="replace")
legacy = LEGACY.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain final geography header include")

for token in [
    "GeographyDelaySeconds = 3.35f",
    "LegacySliceCenterX = -3400.0f",
    "LegacyStadiumAnchor(15000.0f, -1500.0f, 0.0f)",
    "FOCGeoReference::ToLocalCm(50.94936, 30.88466, 0.0)",
    "OpenStreetMap way 416516456",
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
    "Component->UpdateInstanceTransform",
    "GameMode->IsFrontendOnlySession()",
    "source geo corridor remains authoritative",
]:
    if token not in cpp:
        fail(f"verified geography marker missing: {token}")

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
        fail(f"geography correction must transform owned ISM instances, not spawn/move gameplay actors: {forbidden}")

print("R13.6 VERIFIED OSTER GEOGRAPHY VERIFY: PASS")
print("Checks deprecation of the old near-spawn fake Krushelnytska presentation, preservation of the source geo corridor and public-map relocation of stadium-owned instances.")
