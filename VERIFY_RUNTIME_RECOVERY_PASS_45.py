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
perf = text(SRC / "OCPerformanceSampleSubsystem.cpp")
landmark_validation = text(SRC / "OCR146LandmarkSeparationSubsystem.cpp")
landmark_validation_h = text(PUB / "OCR146LandmarkSeparationSubsystem.h")
startup = text(SRC / "OCLandmarkStartupCoordinatorSubsystem.cpp")
spawn_guard = text(SRC / "OCMuseumSpawnGuardSubsystem.cpp")
spawn_guard_h = text(PUB / "OCMuseumSpawnGuardSubsystem.h")
pickup = text(SRC / "OCPickupGunTruck.cpp")
btr = text(SRC / "OCBTR.cpp")
tactical = text(SRC / "OCTacticalMapVisual.cpp")
foliage = text(SRC / "OCFoliageRuntimeGuardSubsystem.cpp")
foliage_h = text(PUB / "OCFoliageRuntimeGuardSubsystem.h")
budget = text(SRC / "OCWorldRenderBudgetPass17Subsystem.cpp")
world = text(SRC / "OCWorldSectorOster.cpp")
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
    "RemoveInstance(",
    "->Destroy()",
    "AddOnActorSpawnedHandler",
    "SetActorLocation(",
    "SetActorTransform(",
):
    req(forbidden not in landmark_validation,
        f"landmark separation again mutates runtime state instead of validating: {forbidden}")

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
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
    "nonuniform_stretch=0",
    "PASS45_M2_MOUNT_ALIGNMENT_READY",
    "bottom_on_mount=1",
):
    req(needle in pickup, f"HMMWV/M2 corrective transform contract missing: {needle}")
for needle in (
    "PASS45_BTR4_PROPORTIONAL_VISUAL_READY",
    "nonuniform_stretch=0",
):
    req(needle in btr, f"BTR4 corrective transform contract missing: {needle}")
for forbidden in (
    "DesiredSizeCm.X / NativeSize.X",
    "DesiredSizeCm.Y / NativeSize.Y",
    "DesiredSizeCm.Z / NativeSize.Z",
):
    req(forbidden not in pickup and forbidden not in btr,
        f"legacy non-uniform production vehicle stretch returned: {forbidden}")

# Tactical map topology must come from the retained compact user reference, not source blockout ISMs.
for needle in (
    "Pass45ReferenceWidthPx = 640.0f",
    "Pass45ReferenceHeightPx = 630.0f",
    "Pass45ReferenceRoads[]",
    "ReferencePixelToSectorLocal",
    "PASS45_TACTICAL_REFERENCE_TOPOLOGY_READY",
    "procedural_road_ism=0",
    "procedural_sidewalk_ism=0",
    "procedural_building_ism=0",
    "FOCGeoReference::Museum()",
    "FOCGeoReference::CultureHouse()",
    "FOCGeoReference::Silpo()",
    "FOCGeoReference::Stadium()",
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

# Primitive source trees stay audit-visible but hidden from normal gameplay; verified pine stays real, oak stays unverified.
for family in (
    "TreeTrunks", "TreeCrowns", "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
):
    req(f'TEXT("{family}")' in foliage, f"foliage guard does not own primitive family retirement: {family}")
req("RetireSourceTreeProxies" in foliage and "bTreeProxyRetirementObserved" in foliage_h,
    "primitive tree retirement path is incomplete")
req("PASS45_PRIMITIVE_TREE_PROXIES_RETIRED" in foliage and "cylinder_sphere_visible=0" in foliage,
    "primitive tree visual retirement evidence missing")
req("oak_asset_verified=0" in foliage, "Pass45 must not invent a verified oak asset")
req(has_all(trees, ["SM_Pine_Tree_01", "SM_Pine_Tree_03"]), "known real pine assets are no longer referenced")
req("/Engine/BasicShapes/Cylinder" in world and "/Engine/BasicShapes/Sphere" in world,
    "historical source proxy authoring unexpectedly vanished before primary-authoring migration")

# Every required weapon gets mesh -> material -> texture dependency truth in fresh NullRHI preflight.
for needle in (
    "DEPENDENCY_REPORT_PATH",
    "required_weapon_material_texture_dependencies.json",
    "DEPENDENCY_SUCCESS_SENTINEL",
    "unreal.MaterialEditingLibrary.get_used_textures(material)",
    "is_placeholder_texture",
    "missing_texture_dependency",
    "placeholder_texture_dependency",
    "TEXTURE_DEPENDENCY_SUMMARY=",
    "texture_dependency_result",
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
print("- latest runtime rejection remains authoritative and legacy mutating owners stay physically retired")
print("- landmark startup is coordinated once; parcel separation is validation-only, never late repair")
print("- Museum BASE correction is initial-character-only")
print("- HMMWV/BTR preserve native proportions and M2 uses bottom-on-mount alignment")
print("- compact reference tactical topology / render budget / bounded foliage contracts remain")
print("- all required weapons emit mesh/material/texture dependency truth")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
