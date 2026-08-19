from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13MuseumStadiumPhotoFidelitySubsystem.h"
CPP = SRC / "Private" / "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp"
GEO_H = SRC / "Public" / "OCR13VerifiedOsterGeographySubsystem.h"
GEO_CPP = SRC / "Private" / "OCR13VerifiedOsterGeographySubsystem.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 MUSEUM/STADIUM PHOTO FIDELITY VERIFY FAIL: " + message)


for path in (H, CPP, GEO_H, GEO_CPP, WORLD):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
geo_h = GEO_H.read_text(encoding="utf-8", errors="replace")
geo_cpp = GEO_CPP.read_text(encoding="utf-8", errors="replace")
world = WORLD.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final header include")

required = [
    "PhotoFidelityDelaySeconds = 4.15f",
    "UOCR13VerifiedOsterGeographySubsystem::VerifiedStadiumAnchor()",
    "SuppressLegacyMuseumPresentation",
    "SuppressLegacyStadiumPresentation",
    "R13_MuseumPhotoBrickBody",
    "R13_MuseumPhotoWoodUpper",
    "R13_MuseumPhotoMetalRoof",
    "R13_MuseumPhotoGlass",
    "R13_MuseumPhotoConcrete",
    "R13_MuseumPhotoGasPipe",
    "R13_MuseumPhotoEntranceSign",
    "R13_MuseumPhotoPine01",
    "R13_MuseumPhotoDeciduous01",
    "R13_StadiumPhotoOpenGrass",
    "R13_StadiumPhotoGoals",
    "R13_StadiumPhotoExerciseBars",
    "for (int32 Slab = 0; Slab < 26; ++Slab)",
    "for (int32 Step = 0; Step < 6; ++Step)",
    "AddFrontWindow(Trim, Glass, Museum, -2120.0f",
    "AddFrontWindow(Trim, Glass, Museum,  2120.0f",
    "AddBox(Wood, Museum + FVector(0.0f, -1710.0f",
    "AddBox(Wood, Museum + FVector(-3300.0f, 120.0f",
    "AddBox(Annex, Museum + FVector(3350.0f, 720.0f",
    "old track/stands intentionally removed",
]
for token in required:
    if token not in cpp:
        fail(f"photo-fidelity marker missing: {token}")

# Compatibility accessor is read-only and must delegate to the canonical source-world anchor.
if "static FVector VerifiedStadiumAnchor();" not in geo_h:
    fail("read-only stadium compatibility accessor missing")
if "return AOCWorldSectorOster::StadiumAnchor();" not in geo_cpp:
    fail("stadium compatibility accessor does not delegate to canonical source-world anchor")
for token in [
    "FVector AOCWorldSectorOster::StadiumAnchor()",
    "FOCGeoReference::Stadium()",
]:
    if token not in world:
        fail(f"canonical stadium source marker missing: {token}")

if cpp.index("SuppressLegacyMuseumPresentation(World);") > cpp.index("BuildMuseum(World);"):
    fail("legacy museum presentation must be removed before final museum build")
if cpp.index("SuppressLegacyStadiumPresentation(World);") > cpp.index("BuildStadium(World);"):
    fail("legacy stadium presentation must be removed before final stadium build")

for forbidden in [
    "SM_Forest_Path.SM_Forest_Path",
    "R13_StadiumTrack",
    "R13_StadiumStand",
    "LegacyStadiumAnchor",
    "FOCGeoReference::ToLocalCm(50.94936",
]:
    if forbidden in cpp:
        fail(f"old/duplicated site presentation leaked into photo fidelity pass: {forbidden}")

print("R13.6 MUSEUM/STADIUM PHOTO FIDELITY VERIFY: PASS")
print("Checks final museum presentation from supplied photos, simplified adjacent stadium, cleanup-before-build ordering and canonical location-first stadium ownership.")
