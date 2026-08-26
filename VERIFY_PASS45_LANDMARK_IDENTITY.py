from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict/Private"

MUSEUM = SRC / "OCR137MuseumPhotoModelSubsystem.cpp"
CULTURE = SRC / "OCR146CultureHousePhotoModelSubsystem.cpp"
SILPO_SHELL = SRC / "OCR140SilpoPhotoModelSubsystem.cpp"
SILPO_IDENTITY = SRC / "OCR143SilpoFacadeIdentitySubsystem.cpp"
COORDINATOR = SRC / "OCLandmarkStartupCoordinatorSubsystem.cpp"
SEPARATION = SRC / "OCR146LandmarkSeparationSubsystem.cpp"
GEO = SRC / "OCGeoReference.cpp"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
RUNTIME_LAUNCHER = ROOT / "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 LANDMARK IDENTITY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, where: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS45 LANDMARK IDENTITY FAIL: {where}: missing {needle!r}")


def forbid(text: str, needle: str, where: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS45 LANDMARK IDENTITY FAIL: {where}: forbidden {needle!r}")


museum = read(MUSEUM)
culture = read(CULTURE)
silpo_shell = read(SILPO_SHELL)
silpo_identity = read(SILPO_IDENTITY)
coordinator = read(COORDINATOR)
separation = read(SEPARATION)
geo = read(GEO)
runtime_evidence = read(RUNTIME_EVIDENCE)
runtime_launcher = read(RUNTIME_LAUNCHER)
tz = read(TZ)

# Canonical anchors remain distinct and source-owned by the shared georeference.
for needle in (
    'TEXT("MuseumSolonyna"), 50.948239, 30.883865',
    'TEXT("OsterCultureHouse"), 50.948694, 30.881435',
    'TEXT("SilpoOster"), 50.948833799986254, 30.87572244094098',
):
    require(geo, needle, "canonical Oster landmark georeference")

# Museum must read as the Solonyna house and must not contain the civic six-column identity.
for needle in (
    'Model->Tags.Add(TEXT("R137_MuseumPhotoModel"));',
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_SheetMetalRoof")',
    'const FVector Museum = AOCWorldSectorOster::MuseumAnchor();',
):
    require(museum, needle, "Museum authoritative exterior")
for forbidden in (
    'R146_CultureHouseAuthoritative',
    'R146Culture_Columns',
    'ColumnXs[]',
    'TEXT("Сільпо")',
):
    forbid(museum, forbidden, "Museum must not encode Culture House/Silpo identity")

# Culture House is the only current six-column civic facade and is rooted at its own geo anchor.
for needle in (
    'Model->Tags.Add(TEXT("R146_CultureHouseAuthoritative"));',
    'Model->Tags.Add(TEXT("CultureHouseOster_Hranovskoho3"));',
    'const FOCGeoReferencePoint Ref = FOCGeoReference::CultureHouse();',
    'Model->SetActorLocationAndRotation(CultureHouseAnchor()',
    'TEXT("R146Culture_Columns")',
    'const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };',
):
    require(culture, needle, "Culture House authoritative facade")
for forbidden in (
    'R137_MuseumPhotoModel',
    'TEXT("Сільпо")',
):
    forbid(culture, forbidden, "Culture House must not impersonate Museum/Silpo")

# Silpo shell and facade identity are both tied to the one canonical Silpo site.
for needle in (
    'const FOCGeoReferencePoint Ref = FOCGeoReference::Silpo();',
    'Model->Tags.Add(TEXT("R140_SilpoPhotoModel"));',
    'Model->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));',
    'Model->SetActorLocation(Site);',
):
    require(silpo_shell, needle, "Silpo authoritative shell")

for needle in (
    'const FOCGeoReferencePoint Ref = FOCGeoReference::Silpo();',
    'Identity->Tags.Add(TEXT("R143_SilpoFacadeIdentity"));',
    'Identity->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));',
    'Identity->SetActorLocation(Site);',
    'TEXT("R143Silpo_LogoText")',
    'SetText(FText::FromString(TEXT("Сільпо")))',
    'R14.3 Silpo facade identity pass built at',
):
    require(silpo_identity, needle, "Silpo facade identity/sign")

# One coordinated startup window owns the three location builds. The validator observes; it does not repair.
for needle in (
    'World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>()',
    'World.GetSubsystem<UOCR140SilpoPhotoModelSubsystem>()',
    'World.GetSubsystem<UOCR143SilpoFacadeIdentitySubsystem>()',
    'World.GetSubsystem<UOCR146CultureHousePhotoModelSubsystem>()',
    'PASS45_LANDMARK_STARTUP_COORDINATED_READY',
):
    require(coordinator, needle, "landmark startup coordinator")

for needle in (
    'MuseumOwnerTag(TEXT("R137_MuseumPhotoModel"))',
    'SilpoOwnerTag(TEXT("R140_SilpoPhotoModel"))',
    'CultureOwnerTag(TEXT("R146_CultureHouseAuthoritative"))',
    'CultureIdentityInstancesAtMuseum == 0',
    'MuseumIdentityInstancesAtCulture == 0',
    'SilpoIdentityInstancesAtMuseum == 0',
    'SilpoIdentityInstancesAtCulture == 0',
    'MuseumIdentityInstancesAtSilpo == 0',
    'CultureIdentityInstancesAtSilpo == 0',
    'CultureColumnShafts == 6',
    'PASS45_LANDMARK_IDENTITY_VALIDATION_READY',
    'PASS45_SILPO_IDENTITY_VALIDATION_READY',
    'mutation=0',
):
    require(separation, needle, "validation-only landmark separation")
for forbidden in (
    'RunStartupGuardPass',
    'SeparationStartupGuardPassCount',
    'RemoveInstance(',
    '->Destroy(',
):
    forbid(separation, forbidden, "landmark validator must not become a repair owner")

# Runtime acceptance must require both parcel/owner separation and the actual Silpo facade-identity stage.
for needle in (
    'PASS45_LANDMARK_SEPARATION_VALIDATION_READY',
    'PASS45_LANDMARK_IDENTITY_VALIDATION_READY',
    'PASS45_SILPO_IDENTITY_VALIDATION_READY',
    'R14.3 Silpo facade identity pass built at',
):
    require(runtime_evidence, needle, "strict runtime evidence")
    require(runtime_launcher, needle, "focused landmark runtime launcher")

# The canonical TZ must keep the visual hard-fail rules explicit.
for needle in (
    'six-column Culture-House facade at Museum site = hard FAIL;',
    'Silpo identity/sign belongs only to canonical Silpo site;',
    'each landmark needs separate runtime identity and screenshot evidence.',
):
    require(tz, needle, "canonical Pass45 TZ")

print("PASS45 LANDMARK IDENTITY SOURCE CONTRACT PASS")
print("- Museum retains the Solonyna-house identity and cannot encode the six-column Culture House facade")
print("- Culture House owns the six-column facade on its separate canonical geo anchor")
print("- Silpo shell and visible Сільпо facade identity are both tied to the canonical Silpo site")
print("- startup is coordinated; separation/identity validation is observation-only and cannot repair late")
print("- strict runtime acceptance requires Museum/Culture/Silpo separation plus the factual Silpo facade-identity stage")
print("STATUS: SOURCE CONTRACT ONLY; current-head local UE 5.8 screenshots remain mandatory")
