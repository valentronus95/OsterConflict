#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"
SCRIPTS = ROOT / "OsterConflict" / "Scripts"
errors = []


def text(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(cond: bool, message: str) -> None:
    if not cond:
        errors.append(message)


def absent(path: Path, label: str) -> None:
    req(not path.exists(), f"stale {label} resurrected: {path.relative_to(ROOT)}")


def has_all(haystack: str, needles) -> bool:
    return all(n in haystack for n in needles)


tz = text(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")
manifest = text(ROOT / "RUNTIME_EVIDENCE" / "2026-08-25_PASS45_REJECTED" / "MANIFEST.md")
ledger = text(ROOT / "OSTER_CONFLICT_WORK_LEDGER.md")
start = text(ROOT / "START_HERE.cmd")
launcher = text(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
acceptance = text(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")
perf = text(SRC / "OCPerformanceSampleSubsystem.cpp")
landmark_validation = text(SRC / "OCR146LandmarkSeparationSubsystem.cpp")
landmark_validation_h = text(PUB / "OCR146LandmarkSeparationSubsystem.h")
startup = text(SRC / "OCLandmarkStartupCoordinatorSubsystem.cpp")
spawn_guard = text(SRC / "OCMuseumSpawnGuardSubsystem.cpp")
spawn_guard_h = text(PUB / "OCMuseumSpawnGuardSubsystem.h")
r138 = text(SRC / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
r138_h = text(PUB / "OCR138MuseumInteractiveArchitectureSubsystem.h")
pickup = text(SRC / "OCPickupGunTruck.cpp")
btr = text(SRC / "OCBTR.cpp")
vehicle_base = text(SRC / "OCVehicleBase.cpp")
armed_vehicle = text(SRC / "OCArmedVehicleBase.cpp")
vehicle_guard = text(SRC / "OCProductionVehicleVisualGuardSubsystem.cpp")
vehicle_guard_h = text(PUB / "OCProductionVehicleVisualGuardSubsystem.h")
character = text(SRC / "OCCharacter.cpp")
tactical = text(SRC / "OCTacticalMapVisual.cpp")
foliage = text(SRC / "OCFoliageRuntimeGuardSubsystem.cpp")
foliage_h = text(PUB / "OCFoliageRuntimeGuardSubsystem.h")
budget = text(SRC / "OCWorldRenderBudgetPass17Subsystem.cpp")
world = text(SRC / "OCWorldSectorOster.cpp")
world_h = text(PUB / "OCWorldSectorOster.h")
trees = text(SRC / "OCR145MuseumTreeLayoutSubsystem.cpp")
weapon_preflight = text(SCRIPTS / "verify_required_weapon_assets.py")
pass43 = text(ROOT / "VERIFY_SLATE_RENDER_TARGET_STARTUP_PASS_43.py")
pass23 = text(ROOT / "VERIFY_DX11_SM5_RENDER_TARGET_PASS_23.py")

req("PASS 45 RUNTIME RECOVERY TZ" in tz, "canonical Pass 45 corrective TZ is missing")
req("RUNTIME REJECTED" in tz and "CODED_UNTESTED" in tz,
    "Pass45 TZ must remain runtime-rejected/source-untested until factual UE acceptance")
req("2026-08-25" in manifest and "RUNTIME REJECTED" in manifest,
    "latest Pass45 rejected runtime evidence manifest is missing classification/date")
req("RUNTIME REJECTED" in ledger and "Pass 45" in ledger,
    "ledger does not preserve current rejected-runtime Pass45 state")

# Physically retired mutation/compatibility owners must stay absent.
for path, label in (
    (PUB / "OCWorldProductionVisualsSubsystem.h", "B2 world visual owner"),
    (SRC / "OCWorldProductionVisualsSubsystem.cpp", "B2 world visual owner"),
    (PUB / "OCMuseumCoreRecoverySubsystem.h", "Museum core recovery owner"),
    (SRC / "OCMuseumCoreRecoverySubsystem.cpp", "Museum core recovery owner"),
    (PUB / "OCMuseumVisibilityPass37Subsystem.h", "Museum visibility/rebuild owner"),
    (SRC / "OCMuseumVisibilityPass37Subsystem.cpp", "Museum visibility/rebuild owner"),
    (PUB / "OCLandmarkShellOwnershipGuardSubsystem.h", "landmark shell destroy guard"),
    (SRC / "OCLandmarkShellOwnershipGuardSubsystem.cpp", "landmark shell destroy guard"),
    (PUB / "OCR137MuseumSiteReplacementSubsystem.h", "retired Museum site replacement shell"),
    (SRC / "OCR137MuseumSiteReplacementSubsystem.cpp", "retired Museum site replacement shell"),
    (PUB / "OCR13MuseumStadiumPhotoFidelitySubsystem.h", "retired museum-stadium compatibility shell"),
    (SRC / "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp", "retired museum-stadium compatibility shell"),
    (PUB / "OCWeaponPalettePass37Subsystem.h", "retired weapon palette shell"),
    (SRC / "OCWeaponPalettePass37Subsystem.cpp", "retired weapon palette shell"),
    (ROOT / ".github" / "workflows" / "pass45-targeted-source-patch.yml", "temporary targeted patch workflow"),
    (ROOT / ".github" / "workflows" / "pass45-vehicle-transform-trace-patch.yml", "temporary transform patch workflow"),
):
    absent(path, label)

# RHI A/B: normal DX11/SM5/no-HDR with normal threading, explicit compatibility-only -norhithread.
req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr"' in launcher, "normal RHI-thread baseline is missing")
req('if /I "%OC_RHI_COMPAT%"=="1"' in launcher, "explicit RHI compatibility selector is missing")
req('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -norhithread"' in launcher, "no-RHI-thread compatibility route is missing")
req("SAFE СУМІСНІСТЬ" in start and 'set "OC_RHI_COMPAT=1"' in start,
    "START_HERE does not expose the compatibility A/B route")
req("-d3d12" not in launcher.lower() and "-sm6" not in launcher.lower(),
    "Pass45 normal gameplay must not re-enable D3D12/SM6 during recovery")
req("-nullrhi" in launcher and "-run=pythonscript" in launcher,
    "isolated weapon preflight must remain NullRHI")
req("-fullscreen" in launcher and "-windowed" not in launcher.lower(),
    "normal recovery launcher must not force windowed mode")
req("t.MaxFPS 60" in launcher and "PASS45_NORMAL_DISPLAY_THERMAL_GUARD" in launcher,
    "normal recovery launcher must keep the explicit 60 FPS thermal guard")

# Frontend/gameplay evidence must be independent and must not mutate quality to hide performance.
for marker in (
    "PASS45_RHI_MODE",
    "PASS45_FRONTEND_PERF_BASELINE",
    "PASS45_FRONTEND_PERF_BELOW_TARGET",
    "PASS45_GAMEPLAY_PERF_BASELINE",
    "PASS45_GAMEPLAY_PERF_BELOW_TARGET",
):
    req(marker in perf, f"performance sampler missing {marker}")
req("quality_mutation=0" in perf, "Pass45 performance evidence must not mutate visual quality")
req("PC->GetPawn() == nullptr" in perf, "frontend performance sample is not pawn-less")

# Landmark mutation ordering: one coordinator runs stages; separation is validation-only and never repairs late.
for needle in (
    "Timers.ClearAllTimersForObject(Stage)",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "legacy_core_recovery=0",
    "destructive_visibility_rebuild=0",
):
    req(needle in startup, f"coordinated landmark startup missing: {needle}")
for needle in (
    "PASS45_LANDMARK_SEPARATION_VALIDATION_SCHEDULED",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_READY",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "mutation=0",
    "primary_authoring_fix_required=1",
):
    req(needle in landmark_validation_h + landmark_validation,
        f"validation-only landmark separation contract missing: {needle}")
for forbidden in (
    "RemoveInstance(", "->Destroy()", "AddOnActorSpawnedHandler", "SetActorLocation(", "SetActorTransform(",
):
    req(forbidden not in landmark_validation,
        f"landmark separation again mutates runtime state instead of validating: {forbidden}")

# Museum visible ownership: R13.7 remains visible; R13.8 owns hidden collision/interactions only.
for needle in (
    "R13.7 is the single visible Museum exterior",
    "R13.8 must never suppress, repaint or replace",
    "ReleaseR137StructuralCollision",
    "BuildInteractionCollisionArchitecture",
    "SetVisibility(false, true)",
    "SetHiddenInGame(true, true)",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "visible_owner=R137",
    "interaction_owner=R138",
    "visible_shell_duplication=0",
    "visibility_mutation=0",
    "material_mutation=0",
):
    req(needle in r138_h + r138, f"Museum ownership consolidation missing: {needle}")
for forbidden in (
    "SuppressSolidPrototype", "BuildSegmentedArchitecture", "MakeMuseumMID", "MakeDetailISM",
    'R138_MuseumHighFidelityArchitecture',
):
    req(forbidden not in r138_h + r138, f"old R13.8 second-visible-shell behavior returned: {forbidden}")

# Museum BASE recovery is initial-character-only; vehicle possession cannot trigger deployment correction.
for needle in (
    "ValidatedBaseDeploymentControllers",
    "AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn())",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "vehicle_revalidation=0",
):
    req(needle in spawn_guard_h + spawn_guard, f"initial-only Museum deployment contract missing: {needle}")
req("LastValidatedPawnByController" not in spawn_guard_h + spawn_guard,
    "legacy arbitrary-pawn BASE revalidation cache returned")

# Production vehicle visuals preserve mesh proportions and M2 is grounded to its mount plane.
for needle in (
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY", "nonuniform_stretch=0",
    "PASS45_M2_MOUNT_ALIGNMENT_READY", "bottom_on_mount=1",
):
    req(needle in pickup, f"HMMWV/M2 corrective transform contract missing: {needle}")
for needle in ("PASS45_BTR4_PROPORTIONAL_VISUAL_READY", "nonuniform_stretch=0"):
    req(needle in btr, f"BTR4 corrective transform contract missing: {needle}")
for forbidden in ("DesiredSizeCm.X / NativeSize.X", "DesiredSizeCm.Y / NativeSize.Y", "DesiredSizeCm.Z / NativeSize.Z"):
    req(forbidden not in pickup and forbidden not in btr,
        f"legacy non-uniform production vehicle stretch returned: {forbidden}")

# VehicleBase owns the material rule at source; /Game/Production meshes bypass legacy BasicShape tint.
for needle in (
    'AssetPath.StartsWith(TEXT("/Game/Production/"))',
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
    "production_override=0",
    "legacy_tint_blockout_only=1",
):
    req(needle in vehicle_base, f"VehicleBase production material bypass missing: {needle}")

# Production vehicle guard is one-shot read-only validation, never a runtime repair layer.
for needle in (
    "Pass45 read-only production vehicle visual validator",
    "ValidationDelaySeconds = 1.00f",
    "PASS45_PRODUCTION_VEHICLE_VALIDATION_SCHEDULED",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
    "validation_only=1", "mutation=0", "polling=0",
):
    req(needle in vehicle_guard_h + vehicle_guard, f"production vehicle validation-only contract missing: {needle}")
for forbidden in (
    "EmptyOverrideMaterials(", "Component->SetMaterial(", "MaxAuditPasses", "AuditIntervalSeconds",
    "PASS42_PRODUCTION_MATERIALS_RESTORED",
):
    req(forbidden not in vehicle_guard, f"legacy production vehicle material repair returned: {forbidden}")

# Driver/gunner possession must preserve current vehicle-local transforms and emit factual evidence.
for needle in (
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY", "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY", "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "museum_respawn_path=0", "VehicleLocationBeforeEnter", "VehicleLocationAtExit",
    "ResultingPawnLocation", "ExitErrorCm <= 100.0f",
):
    req(needle in vehicle_base, f"driver enter/exit transform evidence missing: {needle}")
for needle in (
    "PASS45_GUNNER_EXIT_TRANSFORM_READY", "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "VehicleLocationAtExit", "ResultingPawnLocation", "ExitErrorCm <= 100.0f", "museum_respawn_path=0",
):
    req(needle in armed_vehicle, f"gunner exit transform evidence missing: {needle}")

# M2 gunner vertical input: invert OFF must use direct positive pitch (mouse up raises aim).
for needle in (
    "const float GunnerPitchSign = Settings->bInvertMouseY ? -1.0f : 1.0f;",
    "LocalVehicleGunnerPitch + Value.Get<float>()",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "default_invert=0", "mouse_up_raises=1",
):
    req(needle in character, f"M2 gunner pitch contract missing: {needle}")
req("LocalVehicleGunnerPitch - Value.Get<float>() * 1.15f" not in character,
    "old inverted gunner pitch expression returned")

# Runtime acceptance must consume current markers, not old repair markers.
for marker in (
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
):
    req(marker in acceptance, f"current Pass45 runtime acceptance marker missing: {marker}")
for stale in ("PASS42_PRODUCTION_MATERIALS_RESTORED", "PASS42_PRODUCTION_VEHICLE_VISUALS_READY"):
    req(stale not in acceptance, f"runtime acceptance still requires obsolete repair marker: {stale}")

# Tactical map topology must come from retained compact user reference, not source blockout ISMs.
for needle in (
    "Pass45ReferenceWidthPx = 640.0f", "Pass45ReferenceHeightPx = 630.0f", "Pass45ReferenceRoads[]",
    "ReferencePixelToSectorLocal", "PASS45_TACTICAL_REFERENCE_TOPOLOGY_READY", "procedural_road_ism=0",
    "procedural_sidewalk_ism=0", "procedural_building_ism=0", "FOCGeoReference::Museum()",
    "FOCGeoReference::CultureHouse()", "FOCGeoReference::Silpo()", "FOCGeoReference::Stadium()",
    "FOCGeoReference::CentralPark()",
):
    req(needle in tactical, f"Pass45 tactical topology contract missing: {needle}")
map_build = re.search(r"void\s+UOCTacticalMapWidget::BuildProductionVisualLayer\(\)\s*\{(?P<body>.*)\n\}", tactical, re.S)
if not map_build:
    errors.append("could not isolate BuildProductionVisualLayer")
else:
    map_body = map_build.group("body")
    for forbidden_call in (
        "Sector->GetTacticalRoads()", "Sector->GetTacticalSidewalks()", "Sector->GetTacticalBuildings()",
        "Sector->GetTacticalResidentialRoofs()", "Sector->GetTacticalLandmarkBlocks()", "Sector->GetTacticalLandmarkRoofs()",
    ):
        req(forbidden_call not in map_body,
            f"tactical map still derives topology from procedural source ISM: {forbidden_call}")

# Compact render budget: no historical 1300 m family culls.
req("PASS45_COMPACT_WORLD_CULL_BUDGET_READY" in budget, "compact world cull marker missing")
req("130000" not in budget, "historical 1300 m source-family cull distance returned")
req('TEXT("GrassMown"),              0,  16000' in budget, "ground-cover cull was not reduced")
req('TEXT("ResidentialDetails"),  6000,  24000' in budget, "residential detail cull was not reduced")

# PASS45 item 26: primitive Cylinder/Sphere tree authoring is physically gone; tracked authored trees own vegetation.
primitive_tree_names = (
    "TreeTrunks", "TreeCrowns", "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
)
for family in primitive_tree_names:
    req(f'TEXT("{family}")' not in world and family not in world_h,
        f"rejected primitive tree authoring returned: {family}")
req("/Engine/BasicShapes/Cylinder" not in world and "/Engine/BasicShapes/Sphere" not in world,
    "Cylinder/Sphere tree source authoring returned")
for needle in (
    "AuthoredDeciduousTrees", "AuthoredPine01Trees", "AuthoredPine03Trees",
    "SM_Tree_Var01", "SM_Pine_Tree_01", "SM_Pine_Tree_03", "AddGroundedTree",
):
    req(needle in world + world_h, f"authored vegetation source contract missing: {needle}")
for needle in (
    "PASS45_AUTHORED_TREE_FAMILY_READY", "primitive_tree_components=0", "authored_tree_components=3",
    "basicshape_tree_meshes=0", "oak_asset_verified=0",
):
    req(needle in foliage, f"authored vegetation runtime guard missing: {needle}")
req("RetireSourceTreeProxies" not in foliage + foliage_h and "bTreeProxyRetirementObserved" not in foliage_h,
    "late-hide primitive tree retirement path must be physically retired after primary-authoring migration")
req(has_all(trees, ["SM_Pine_Tree_01", "SM_Pine_Tree_03"]), "known real Museum pine assets are no longer referenced")
req("SM_Oak" not in world + trees + foliage, "Pass45 must not invent an unverified oak asset")

# Every required weapon gets mesh -> material -> texture dependency truth in fresh NullRHI preflight.
for needle in (
    "DEPENDENCY_REPORT_PATH", "required_weapon_material_texture_dependencies.json", "DEPENDENCY_SUCCESS_SENTINEL",
    "unreal.MaterialEditingLibrary.get_used_textures(material)", "is_placeholder_texture", "missing_texture_dependency",
    "placeholder_texture_dependency", "TEXTURE_DEPENDENCY_SUMMARY=", "texture_dependency_result",
    "PASS45_WEAPON_DEPENDENCY_AUDIT_COMPLETE",
):
    req(needle in weapon_preflight, f"weapon material/texture dependency audit missing: {needle}")
req('("M16"' not in weapon_preflight and '("M4"' not in weapon_preflight,
    "weapon preflight invents unverified M16/M4 production payloads")
req("white/default slots are NOT production-ready" in weapon_preflight,
    "weapon preflight lost white/default material failure semantics")

# Stale renderer verifiers must not force -norhithread back into every normal run.
req("explicit Pass 45 compatibility A/B" in pass43, "Pass43 verifier not forward-ported for RHI A/B")
req("compatibility A/B route" in pass23, "Pass23 verifier not forward-ported for RHI A/B")

if errors:
    print("RUNTIME RECOVERY PASS 45: FAIL")
    for e in errors:
        print("[FAIL]", e)
    raise SystemExit(1)

print("RUNTIME RECOVERY PASS 45: PASS")
print("- latest runtime rejection remains authoritative and retired mutating owners stay physically absent")
print("- R13.7 is the one visible Museum exterior; R13.8 is hidden collision/interactivity only")
print("- VehicleBase owns production-material preservation at source; validation layer is read-only")
print("- driver/gunner transform evidence proves ordinary vehicle possession cannot silently respawn at Museum")
print("- M2 default gunner pitch is direct/non-inverted")
print("- compact reference tactical topology / render budget / authored vegetation contracts remain")
print("- all required weapons emit mesh/material/texture dependency truth")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
