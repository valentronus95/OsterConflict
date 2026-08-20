from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13MuseumProtectionSubsystem.h"
CPP = SRC / "Private" / "OCR13MuseumProtectionSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 MUSEUM/STADIUM PROTECTION VERIFY FAIL: " + message)


if not H.is_file() or not CPP.is_file():
    fail("museum protection header/source missing")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "ProtectionDelaySeconds = 2.65f",
    "LegacyMuseumWindowRadiusCm = 7200.0f",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "Delta.X / 6500.0f",
    "Delta.Y / 5200.0f",
    "Delta.X / 7200.0f",
    "Delta.Y / 5200.0f",
    "FMath::Abs(Delta.X - 1180.0f) <= 1750.0f",
    "Delta.Y <= -900.0f && Delta.Y >= -9200.0f",
    "IsInsidePhotoStadiumProtection",
    'Text.StartsWith(TEXT("R13_DenseGrass"))',
    'Text.StartsWith(TEXT("R13_GroundPlant"))',
    'Text.StartsWith(TEXT("R13_CompanionTree"))',
    'Text.StartsWith(TEXT("R13_ExplicitPine"))',
    'Text.StartsWith(TEXT("R13_Shrub"))',
    'Text.StartsWith(TEXT("R13_Yard"))',
    'Name == TEXT("R13_LandmarkWindowGlass")',
    'Name == TEXT("R13_LandmarkWindowFrames")',
    "IsInsideLegacyMuseumWindowZone",
    "bLegacySharedWindow",
    "bStadiumRemoval",
    "RemovedLegacyWindowInstances",
    "RemovedStadiumDressingInstances",
    "for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)",
    "Component->RemoveInstance(Index)",
    "Component->MarkRenderStateDirty()",
    "GameMode->IsFrontendOnlySession()",
    "college windows and dedicated final museum/stadium/civic art untouched",
]
for token in required:
    if token not in cpp:
        fail(f"missing site-protection marker: {token}")

# This pass may prune the old shared landmark-window bridge only by museum radius and generic dressing only inside
# museum/stadium protection zones. Dedicated final photo/civic families must never become broad prune targets.
for forbidden in [
    'Text.StartsWith(TEXT("R13_MuseumPine"))',
    'Text.StartsWith(TEXT("R13_Museum"))',
    'Text.StartsWith(TEXT("R13_Stadium"))',
    'Text.StartsWith(TEXT("R13_Civic"))',
    'Name == TEXT("R13_LandmarkWindowGlass") || Name == TEXT("R13_LandmarkWindowFrames") || Name == TEXT("R13_LandmarkWindow',
    "DestroyComponent()",
    "DestroyActor",
]:
    if forbidden in cpp:
        fail(f"site protection could remove curated/non-target art too broadly: {forbidden}")

print("R13.6 MUSEUM/STADIUM PROTECTION VERIFY: PASS")
print("Checks generic-dressing cleanup around the museum and adjacent photo stadium plus museum-radius-only pruning of the old shared landmark-window bridge, while preserving college and dedicated final photo/civic art.")
