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

# Museum must read as the Solonyna house, use committed building modules and never resurrect Engine primitives.
for needle in (
    'Model->Tags.Add(TEXT("R137_MuseumPhotoModel"));',
    'Model->Tags.Add(TEXT("MuseumSolonyna_ReferenceExterior"));',
    'TEXT("R137Museum_BrickBody")',
    'TEXT("R137Museum_BlueGreyTimber")',
    'TEXT("R137Museum_SheetMetalRoof")',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m',
    'PASS45_MUSEUM_AUTHORED_SHELL_READY',
    'basicshape_structural=0',
    'basicshape_material=0',
    'museum_columns=0',
    'const FVector Museum = AOCWorldSectorOster::MuseumAnchor();',
):
    require(museum, needle, "Museum authoritative authored exterior")
for forbidden in (
    '/Engine/BasicShapes/Cube.Cube',
    '/Engine/BasicShapes/Cylinder.Cylinder',
    '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial',
    'R146_CultureHouseAuthoritative',
    'R146Culture_Columns',
    'ColumnXs[]',
    'TEXT("Сільпо")',
):
    forbid(museum, forbidden, "Museum must not use primitive structure or encode Culture House/Silpo identity")

# Culture House is the only current six-column civic facade and is rooted at its own geo anchor.
for needle in (
    'Model->Tags.Add(TEXT("R146_CultureHouseAuthoritative"));',
    'Model->Tags.Add(TEXT("CultureHouseOster_Hranovskoho3"));',
    'const FOCGeoReferencePoint Ref = FOCGeoReference::CultureHouse();',
    'Model->SetActorLocationAndRotation(CultureHouseAnchor()',
    'TEXT("R146Culture_Columns")',
    'const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };',
    'PASS45_CULTURE_HOUSE_AUTHORED_SHELL_READY',
    'basicshape_visible=0',
    'basicshape_material=0',
    'bool bLegacyOwnerPresent = false;',
    'Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel"))',
    'reason=legacy_owner_present legacy_owner_mutation=0 primary_authoring_fix_required=1 runtime_acceptance=0',
    'legacy_owner_mutation=0 runtime_visual_acceptance=pending',
):
    require(culture, needle, "Culture House authoritative authored facade")
for forbidden in (
    '/Engine/BasicShapes/Cube.Cube',
    '/Engine/BasicShapes/Cylinder.Cylinder',
    '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial',
    'R137_MuseumPhotoModel',
    'TEXT("Сільпо")',
    'if (Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel"))) Existing->Destroy();',
):
    forbid(culture, forbidden, "Culture House must not use primitive structure, impersonate Museum/Silpo or repair legacy ownership late")

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

# GAME_RECOVERY owns staged pre-spawn startup. The validator observes; it does not repair.
for needle in (
    'World.GetSubsystem<UOCR137MuseumPhotoModelSubsystem>()',
    'World.GetSubsystem<UOCR140SilpoPhotoModelSubsystem>()',
    'World.GetSubsystem<UOCR143SilpoFacadeIdentitySubsystem>()',
    'World.GetSubsystem<UOCR146CultureHousePhotoModelSubsystem>()',
    'GAME_RECOVERY_WORLD_PREP_BEGIN',
    'pre_spawn=1 tick_when_paused=1 staged_materialization=1',
    'GAME_RECOVERY_WORLD_PREP_TIMERS_CANCELLED',
    'duplicate_startup_timers=0',
    'GAME_RECOVERY_WORLD_READY',
    'pre_spawn=1 post_spawn_landmark_materialization=0',
):
    require(coordinator, needle, "landmark startup coordinator")
forbid(coordinator, 'PASS45_LANDMARK_STARTUP_COORDINATED_READY',
       "retired landmark startup compatibility marker")

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

# Runtime acceptance must require parcel/owner separation, authored Museum/Culture shells and Silpo facade identity.
for needle in (
    'PASS45_MUSEUM_AUTHORED_SHELL_READY',
    'PASS45_CULTURE_HOUSE_AUTHORED_SHELL_READY',
    'PASS45_LANDMARK_SEPARATION_VALIDATION_READY',
    'PASS45_LANDMARK_IDENTITY_VALIDATION_READY',
    'PASS45_SILPO_IDENTITY_VALIDATION_READY',
    'R14.3 Silpo facade identity pass built at',
):
    require(runtime_evidence, needle, "strict runtime evidence")

for needle in (
    'PASS45_LANDMARK_SEPARATION_VALIDATION_READY',
    'PASS45_LANDMARK_IDENTITY_VALIDATION_READY',
    'PASS45_SILPO_IDENTITY_VALIDATION_READY',
    'R14.3 Silpo facade identity pass built at',
):
    require(runtime_launcher, needle, "focused landmark runtime launcher")

# The canonical Pass45 TZ remains semantic authority for detailed historical identity obligations.
for needle in (
    'Source-close Museum/Culture House/Silpo identity ownership',
    'Bind Museum, Silpo and Culture House as separate Gate E/K reference contracts.',
    'Museum/Culture/Silpo separated and identified',
    'direct landmark screenshot sets accepted',
    'Gate K passes.',
):
    require(tz, needle, "canonical Pass45 TZ")

print("PASS45 LANDMARK IDENTITY SOURCE CONTRACT PASS")
print("- Museum uses committed authored wall/window/roof/foundation modules; Engine BasicShape structure is forbidden")
print("- Museum retains the Solonyna-house identity and cannot encode the six-column Culture House facade")
print("- Culture House owns the six-column authored facade on its separate canonical geo anchor")
print("- Culture House refuses unexpected R13 ownership fail-closed; no late actor destruction may hide a startup/source regression")
print("- Silpo shell and visible Сільпо facade identity are both tied to the canonical Silpo site")
print("- GAME_RECOVERY staged startup owns readiness; separation/identity validation is observation-only and cannot repair late")
print("- strict runtime acceptance requires authored Museum/Culture shells plus landmark separation and Silpo facade identity")
print("STATUS: SOURCE CONTRACT ONLY; current-head local UE 5.8 screenshots remain mandatory")