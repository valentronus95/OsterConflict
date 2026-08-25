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


# Runtime-rejected, inert, temporary, or superseded historical owners must stay physically retired.
retired_paths = [
    SRC / "Public" / "OCWorldProductionVisualsSubsystem.h",
    SRC / "Private" / "OCWorldProductionVisualsSubsystem.cpp",
    SRC / "Public" / "OCWorldAssetModelsSubsystem.h",
    SRC / "Private" / "OCWorldAssetModelsSubsystem.cpp",
    SRC / "Public" / "OCAssetModelDecorator.h",
    SRC / "Private" / "OCAssetModelDecorator.cpp",
    SRC / "Public" / "OCRecoveredEnvironmentSubsystem.h",
    SRC / "Private" / "OCRecoveredEnvironmentSubsystem.cpp",
    SRC / "Public" / "OCRecoveredBuildingDetailsSubsystem.h",
    SRC / "Private" / "OCRecoveredBuildingDetailsSubsystem.cpp",
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
    SRC / "Public" / "OCR141MuseumWindowReplacementSubsystem.h",
    SRC / "Private" / "OCR141MuseumWindowReplacementSubsystem.cpp",
    SRC / "Public" / "OCWeaponPalettePass37Subsystem.h",
    SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp",
    ROOT / "VERIFY_PASS45_COMPLETION_AUDIT.py",
    ROOT / "VERIFY_OSTER_WORLD_MODELS_PASS.py",
    ROOT / ".github" / "workflows" / "pass45-completion-audit.yml",
    ROOT / ".github" / "workflows" / "oster-world-models-pass.yml",
    ROOT / ".github" / "workflows" / "pass45-targeted-source-patch.yml",
    ROOT / ".github" / "workflows" / "pass45-vehicle-transform-trace-patch.yml",
    ROOT / ".github" / "workflows" / "pass45-museum-ownership-cleanup-patch.yml",
]
for path in retired_paths:
    req(not path.exists(), f"stale/rejected runtime contract resurrected: {path.relative_to(ROOT)}")

retired_class_names = (
    "OCWorldProductionVisualsSubsystem",
    "OCWorldAssetModelsSubsystem",
    "OCAssetModelDecorator",
    "OCRecoveredEnvironmentSubsystem",
    "OCRecoveredBuildingDetailsSubsystem",
    "OCMuseumCoreRecoverySubsystem",
    "OCMuseumVisibilityPass37Subsystem",
    "OCLandmarkShellOwnershipGuardSubsystem",
    "OCR137MuseumSiteReplacementSubsystem",
    "OCR13MuseumStadiumPhotoFidelitySubsystem",
    "OCR141MuseumWindowReplacementSubsystem",
    "OCWeaponPalettePass37Subsystem",
)
for path in list(SRC.rglob("*.cpp")) + list(SRC.rglob("*.h")):
    text = path.read_text(encoding="utf-8", errors="replace")
    for class_name in retired_class_names:
        req(class_name not in text,
            f"retired runtime class referenced by active source: {class_name} in {path.relative_to(ROOT)}")

museum_spawn = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
museum_spawn_h = read(SRC / "Public" / "OCMuseumSpawnGuardSubsystem.h")
landmark_validation = read(SRC / "Private" / "OCR146LandmarkSeparationSubsystem.cpp")
r137 = read(SRC / "Private" / "OCR137MuseumPhotoModelSubsystem.cpp")
r138_h = read(SRC / "Public" / "OCR138MuseumInteractiveArchitectureSubsystem.h")
r138 = read(SRC / "Private" / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
r140 = read(SRC / "Private" / "OCR140MuseumFacadeDetailSubsystem.cpp")
r145 = read(SRC / "Private" / "OCR145MuseumTreeLayoutSubsystem.cpp")
museum_window = read(SRC / "Private" / "OCMuseumBreakableWindow.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
pickup_cpp = read(SRC / "Private" / "OCPickupGunTruck.cpp")
btr_cpp = read(SRC / "Private" / "OCBTR.cpp")
vehicle_base = read(SRC / "Private" / "OCVehicleBase.cpp")
armed_vehicle = read(SRC / "Private" / "OCArmedVehicleBase.cpp")
vehicle_guard_h = read(SRC / "Public" / "OCProductionVehicleVisualGuardSubsystem.h")
vehicle_guard = read(SRC / "Private" / "OCProductionVehicleVisualGuardSubsystem.cpp")
character = read(SRC / "Private" / "OCCharacter.cpp")
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
    req(needle in museum_spawn + museum_spawn_h, f"initial-only Museum deployment guard contract missing: {needle}")
req("LastValidatedPawnByController" not in museum_spawn + museum_spawn_h,
    "legacy pawn-pointer revalidation cache returned; vehicle enter/exit can be mistaken for deployment")
req("APawn* Pawn = PC->GetPawn()" not in museum_spawn,
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

# Museum ownership is explicit and single-purpose.
# R13.7 = one visible exterior and reference trim/grilles. No prototype trees, doors, glass or service gable.
for needle in (
    "PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY",
    "visible_shell_owner=R137",
    "static_glass=0",
    "prototype_doors=0",
    "prototype_trees=0",
    "prototype_service_gable=0",
    "breakable actor owns visible glass",
):
    req(needle in r137, f"R13.7 primary exterior ownership contract missing: {needle}")
for forbidden in (
    "R137Museum_RedTimberGable",
    "R137Museum_Pine01",
    "R137Museum_Pine03",
    "R137Museum_Deciduous01",
    "FVector(-62.0f, -672.0f, 205.0f)",
    "FVector(965.0f, -155.0f, 385.0f)",
):
    req(forbidden not in r137, f"R13.7 obsolete visible prototype returned: {forbidden}")

# R13.8 = hidden collision plus final breakable glass. It never draws a second shell or prototype door.
for needle in (
    "R13.7 is the single visible Museum exterior",
    "R13.8 must never suppress, repaint or replace",
    "ReleaseR137StructuralCollision",
    "BuildInteractionCollisionArchitecture",
    "SetVisibility(false, true)",
    "SetHiddenInGame(true, true)",
    "AOCMuseumBreakableWindow",
    "final_window_class=1",
    "prototype_doors=0",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "visible_owner=R137",
    "interaction_owner=R138",
    "visible_shell_duplication=0",
    "visibility_mutation=0",
    "material_mutation=0",
):
    req(needle in r138_h + r138, f"single visible Museum owner contract missing: {needle}")
for forbidden in (
    "SuppressSolidPrototype",
    "BuildSegmentedArchitecture",
    "MakeMuseumMID",
    "MakeDetailISM",
    "SpawnMuseumDoor",
    "AOCBreakableWindow* Window",
    'R138_MuseumHighFidelityArchitecture',
):
    req(forbidden not in r138_h + r138, f"old second-shell/prototype R13.8 behavior returned: {forbidden}")

# Interactive Museum windows render glass/debris only. R13.7 owns the visible frame/grille layer.
for needle in (
    "PASS45_MUSEUM_WINDOW_GLASS_ONLY_READY",
    "visible_frame_owner=R137",
    "interactive_frame_visible=0",
    "static_glass=0",
    "Component->SetVisibility(false, true)",
    "Component->SetHiddenInGame(true, true)",
):
    req(needle in museum_window, f"Museum glass-only interaction contract missing: {needle}")

# R14.0 and R14.5 only author their final details; they never hide/remove earlier Museum content.
for needle in (
    "PASS45_MUSEUM_R140_DETAIL_ONLY_READY",
    "late_r137_suppression=0",
    "instance_removal=0",
):
    req(needle in r140, f"R14.0 detail-only contract missing: {needle}")
for forbidden in (
    "SuppressIncorrectR137GableAndCanopy",
    "WrongCanopyTarget",
    "RemoveInstance(",
):
    req(forbidden not in r140, f"R14.0 late exterior mutation returned: {forbidden}")
for needle in (
    "PASS45_MUSEUM_TREE_SINGLE_OWNER_READY",
    "owner=R145",
    "r137_tree_pass=0",
    "late_hide=0",
):
    req(needle in r145, f"R14.5 single tree-owner contract missing: {needle}")
req("HideR137MuseumTrees" not in r145, "R14.5 late tree hide helper returned")

# Startup coordinator must not reference the physically retired Museum window replacement stage.
for needle in (
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "window_replacement_stage=0",
    "R138_collision_glass",
    "R139_R140_doors_facade",
):
    req(needle in startup, f"coordinated current Museum startup contract missing: {needle}")
req("OCR141MuseumWindowReplacementSubsystem" not in startup,
    "retired R14.1 Museum window replacement stage returned to coordinator")

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

# VehicleBase owns the primary material rule. Production meshes must bypass legacy BasicShape tinting.
for needle in (
    'AssetPath.StartsWith(TEXT("/Game/Production/"))',
    'PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY',
    'production_override=0',
    'legacy_tint_blockout_only=1',
):
    req(needle in vehicle_base, f"VehicleBase primary production-material rule missing: {needle}")

# The production guard is read-only validation. Never bring back polling or EmptyOverrideMaterials repair.
for needle in (
    "Pass45 read-only production vehicle visual validator",
    "ValidationDelaySeconds = 1.00f",
    "PASS45_PRODUCTION_VEHICLE_VALIDATION_SCHEDULED",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "validation_only=1",
    "mutation=0",
    "polling=0",
):
    req(needle in vehicle_guard_h + vehicle_guard, f"validation-only vehicle material contract missing: {needle}")
for forbidden in (
    "EmptyOverrideMaterials(",
    "Component->SetMaterial(",
    "MaxAuditPasses",
    "AuditIntervalSeconds",
    "PASS42_PRODUCTION_MATERIALS_RESTORED",
):
    req(forbidden not in vehicle_guard, f"legacy production material repair returned: {forbidden}")

# Driver/gunner transforms must remain local to the current vehicle, with fail-visible telemetry.
for needle in (
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "museum_respawn_path=0",
    "requested_exit=",
    "resulting_pawn=",
):
    req(needle in vehicle_base, f"driver vehicle transform evidence missing: {needle}")
for needle in (
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "museum_respawn_path=0",
    "requested_exit=",
    "resulting_pawn=",
):
    req(needle in armed_vehicle, f"gunner vehicle transform evidence missing: {needle}")

# M2 gunner vertical aim follows normal default mouse direction: invert OFF => mouse up raises pitch.
for needle in (
    "const float GunnerPitchSign = Settings->bInvertMouseY ? -1.0f : 1.0f;",
    "LocalVehicleGunnerPitch + Value.Get<float>()",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "default_invert=0",
    "mouse_up_raises=1",
):
    req(needle in character, f"M2 gunner pitch contract missing: {needle}")
req("LocalVehicleGunnerPitch - Value.Get<float>() * 1.15f" not in character,
    "old inverted gunner pitch expression returned")

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
print("- rejected B2, generic village/decorator, recovered environment/building and late Museum owners remain physically deleted")
print("- stale world-model verifier/workflow and temporary source-patch workflows stay retired")
print("- landmark separation is validation-only and cannot repair the world late")
print("- R13.7 is the one visible Museum exterior; R13.8 owns hidden collision + final breakable glass only")
print("- R14.0 is additive final facade detail; R14.5 is the sole current Museum tree owner")
print("- Museum BASE recovery is initial-character-only, not vehicle-possession-driven")
print("- HMMWV/BTR production meshes preserve native proportions")
print("- VehicleBase skips legacy tint for production assets at the primary source")
print("- production vehicle material validator is read-only, one-shot and fail-visible")
print("- driver/gunner enter-exit transforms emit fail-visible local vehicle evidence")
print("- M2 gunner pitch defaults to mouse-up raises aim")
print("- normal recovery route is fullscreen with 60 FPS thermal cap")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime remains authoritative")
