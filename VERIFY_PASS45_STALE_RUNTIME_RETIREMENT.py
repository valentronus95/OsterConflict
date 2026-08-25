#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


# Runtime-rejected or inert historical owners must stay physically retired.
retired_paths = [
    SRC / "Public" / "OCWorldProductionVisualsSubsystem.h",
    SRC / "Private" / "OCWorldProductionVisualsSubsystem.cpp",
    SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h",
    SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp",
    SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h",
    SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp",
    SRC / "Public" / "OCLandmarkShellOwnershipGuardSubsystem.h",
    SRC / "Private" / "OCLandmarkShellOwnershipGuardSubsystem.cpp",
    SRC / "Public" / "OCR137MuseumSiteReplacementSubsystem.h",
    SRC / "Private" / "OCR137MuseumSiteReplacementSubsystem.cpp",
    SRC / "Public" / "OCR13MuseumStadiumPhotoFidelitySubsystem.h",
    SRC / "Private" / "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp",
    SRC / "Public" / "OCWeaponPalettePass37Subsystem.h",
    SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp",
    ROOT / "VERIFY_PASS45_COMPLETION_AUDIT.py",
    ROOT / ".github" / "workflows" / "pass45-completion-audit.yml",
]
for path in retired_paths:
    req(not path.exists(), f"stale/rejected runtime contract resurrected: {path.relative_to(ROOT)}")

retired_class_names = (
    "OCWorldProductionVisualsSubsystem",
    "OCMuseumCoreRecoverySubsystem",
    "OCMuseumVisibilityPass37Subsystem",
    "OCLandmarkShellOwnershipGuardSubsystem",
    "OCR137MuseumSiteReplacementSubsystem",
    "OCR13MuseumStadiumPhotoFidelitySubsystem",
    "OCWeaponPalettePass37Subsystem",
)
for path in list(SRC.rglob("*.cpp")) + list(SRC.rglob("*.h")):
    text = path.read_text(encoding="utf-8", errors="replace")
    for class_name in retired_class_names:
        req(class_name not in text,
            f"retired runtime class referenced by active source: {class_name} in {path.relative_to(ROOT)}")

museum_cpp = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
museum_h = read(SRC / "Public" / "OCMuseumSpawnGuardSubsystem.h")
landmark_validation = read(SRC / "Private" / "OCR146LandmarkSeparationSubsystem.cpp")
pickup_cpp = read(SRC / "Private" / "OCPickupGunTruck.cpp")
btr_cpp = read(SRC / "Private" / "OCBTR.cpp")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
agents = read(ROOT / "AGENTS.md")
tz = read(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")

# BASE recovery is initial-character-only. Vehicle possession must never become a deployment mutation again.
for needle in (
    "ValidatedBaseDeploymentControllers",
    "AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn())",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "vehicle_revalidation=0",
):
    req(needle in museum_cpp + museum_h, f"initial-only Museum deployment guard contract missing: {needle}")
req("LastValidatedPawnByController" not in museum_cpp + museum_h,
    "legacy pawn-pointer revalidation cache returned; vehicle enter/exit can be mistaken for deployment")
req("APawn* Pawn = PC->GetPawn()" not in museum_cpp,
    "Museum guard again validates arbitrary possessed pawns instead of AOCCharacter only")

# Landmark separation may observe/fail only; no late cleanup is allowed to mask primary-authoring errors.
for needle in (
    "PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "mutation=0",
    "primary_authoring_fix_required=1",
):
    req(needle in landmark_validation, f"validation-only landmark contract missing: {needle}")
for forbidden in (
    "RemoveInstance(",
    "->Destroy()",
    "AddOnActorSpawnedHandler",
):
    req(forbidden not in landmark_validation, f"late landmark mutation returned: {forbidden}")

# Production vehicles may not be independently stretched per axis to fit proxy boxes.
for name, text, marker in (
    ("HMMWV", pickup_cpp, "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY"),
    ("BTR4", btr_cpp, "PASS45_BTR4_PROPORTIONAL_VISUAL_READY"),
):
    req(marker in text, f"{name} proportional visual marker missing")
    req("SetRelativeScale3D(FVector(UniformScale))" in text,
        f"{name} production visual does not use uniform scale")
    req("nonuniform_stretch=0" in text, f"{name} runtime marker does not reject non-uniform stretch")

for forbidden in (
    "DesiredSizeCm.X / NativeSize.X",
    "DesiredSizeCm.Y / NativeSize.Y",
    "DesiredSizeCm.Z / NativeSize.Z",
):
    req(forbidden not in pickup_cpp, f"HMMWV legacy non-uniform stretch returned: {forbidden}")
    req(forbidden not in btr_cpp, f"BTR4 legacy non-uniform stretch returned: {forbidden}")

# Normal recovery route must not silently restore the rejected windowed/uncapped test behavior.
req(' -windowed ' not in launcher.lower(), "normal gameplay launcher restored forced -windowed mode")
req("-fullscreen" in launcher, "normal gameplay launcher no longer requests fullscreen recovery mode")
req('t.MaxFPS 60' in launcher, "thermal recovery 60 FPS cap missing")
req("PASS45_NORMAL_DISPLAY_THERMAL_GUARD" in launcher,
    "launcher lacks visible display/thermal recovery marker")

# Root rules/TZ must explicitly require deletion or retirement of obsolete mutating owners and stale verifiers.
for needle in (
    "Physical retirement beats inert resurrection",
    "No historical verifier may require a runtime-rejected owner",
    "legacy owner deletion",
):
    req(needle in agents + tz, f"stale-rule retirement policy missing: {needle}")

if errors:
    print("PASS45 STALE RUNTIME RETIREMENT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 STALE RUNTIME RETIREMENT: PASS")
print("- rejected B2 world visual owner is physically deleted")
print("- late Museum core-recovery / visibility-rebuild / duplicate-destroy guards are physically deleted")
print("- inert R13.7 site-replacement / R13 museum-stadium compatibility shells are physically deleted")
print("- retired weapon palette compatibility shell is physically deleted")
print("- landmark separation is validation-only and cannot repair the world late")
print("- stale B2 completion verifier/workflow cannot force the rejected owner back")
print("- Museum BASE recovery is initial-character-only, not vehicle-possession-driven")
print("- HMMWV/BTR production meshes preserve native proportions")
print("- normal recovery route is fullscreen with 60 FPS thermal cap")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime remains authoritative")
