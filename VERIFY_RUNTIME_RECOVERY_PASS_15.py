#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS15 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS15 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS15 VERIFY FAIL: {label}: forbidden {needle!r}")


recovery_h = read(SRC / "Public" / "OCRuntimeRecoveryPass15Subsystem.h")
recovery = read(SRC / "Private" / "OCRuntimeRecoveryPass15Subsystem.cpp")
museum_h = read(SRC / "Public" / "OCMuseumSpawnGuardSubsystem.h")
museum = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
foliage = read(SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp")
foliage_guard = read(SRC / "Private" / "OCFoliageRuntimeGuardSubsystem.cpp")
perf_h = read(SRC / "Public" / "OCPerformanceSampleSubsystem.h")
perf = read(SRC / "Private" / "OCPerformanceSampleSubsystem.cpp")
launcher = read(ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd")

for needle in ("ApplyFrontendRepairs", "ApplyJoinPendingOverlay", "RunHostTravelFallback"):
    require(recovery_h, needle, "recovery subsystem header")
for needle in (
    "PASS15_FRONTEND_FIELDS_OPAQUE_READY", "Style.SetBackgroundColor", "Field->SetWidgetStyle(Style);",
    "FLinearColor(0.025f, 0.030f, 0.035f, 0.96f)", "PASS15_DIRECT_CONNECT_OVERLAY_BEGIN",
    "PASS15_CONNECTION_FAILURE_RETURN_FRONTEND", "PASS15_HOST_OPENLEVEL_FALLBACK", "UGameplayStatics::OpenLevel",
):
    require(recovery, needle, "frontend/host/join recovery")

# Pass45 supersedes the old pawn-pointer BASE recovery. Validate one initial character deployment per controller;
# vehicle possession/unpossession must never retrigger Museum correction.
for needle in (
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "RequiredRackWeaponCount = 11",
    "ValidateBaseDeployments()",
    "GetRequestedDeploymentSpawn()",
    "ValidatedBaseDeploymentControllers",
    "AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn())",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "vehicle_revalidation=0",
    "Character->SetActorLocationAndRotation",
):
    require(museum_h + museum, needle, "Museum BASE and actual initial character recovery")
for forbidden in (
    "LastValidatedPawnByController",
    "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM",
    "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL",
):
    forbid(museum_h + museum, forbidden, "retired arbitrary-pawn BASE recovery contract")

for needle in (
    "RequiredRackWeaponCount = 11", "AOCWeapon_M14::StaticClass()", "AOCWeapon_Mac10::StaticClass()",
    "AOCWeapon_Tec9::StaticClass()", "AOCWeapon_LeverAction::StaticClass()", "AOCAntiArmorLauncher::StaticClass()",
    "Weapon->Tags.Add(RuntimeBaseRackTag)",
):
    require(spawn, needle, "physical Museum weapon rack")

# Pass45 Block 0 supersedes the historical Museum-only/20m recovery foliage crop. LowCPU now reduces density
# and cull budget across the same compact 960x940m playable Oster bounds; it may not spatially erase the city.
for needle in (
    "CompactMinX = -78000.0f",
    "CompactMaxX =  18000.0f",
    "CompactMinY = -12000.0f",
    "CompactMaxY =  82000.0f",
    "FullGridStepCm = 1000.0f",
    "LowCPUGridStepCm = 1500.0f",
    "FullCellsPerBatch = 32",
    "LowCPUCellsPerBatch = 48",
    "ActiveGridStep = bLowCPUProfile ? LowCPUGridStepCm : FullGridStepCm",
    "PASS45_BLOCK0_FULL_MAP_GRASS_SCOPE_READY",
    "PASS45_BLOCK0_FOLIAGE_BUDGET_READY",
    "PASS45_BLOCK0_FULL_MAP_GRASS_READY",
    "full_playable_bounds=1",
    "museum_only=0",
):
    require(foliage, needle, "Pass45 full-map profile-aware foliage")

full_grid = re.search(r"FullGridStepCm\s*=\s*([0-9.]+)f", foliage)
low_grid = re.search(r"LowCPUGridStepCm\s*=\s*([0-9.]+)f", foliage)
if not full_grid or not low_grid:
    raise SystemExit("PASS15 VERIFY FAIL: profile-aware foliage grid constants missing")
if float(low_grid.group(1)) <= float(full_grid.group(1)):
    raise SystemExit("PASS15 VERIFY FAIL: LowCPU foliage no longer reduces density relative to Full profile")

for forbidden in (
    "LowCPUHalfExtentCm = 10000.0f",
    "PopulationMinX = Museum.X - LowCPUHalfExtentCm",
    "PopulationMaxX = Museum.X + LowCPUHalfExtentCm",
    "full_sector_population=0",
):
    forbid(foliage + foliage_guard, forbidden, "retired Museum-only foliage acceptance")

for needle in (
    'Block0PopulationCompleteTag(TEXT("OC_Block0FullMapGrassComplete"))',
    "ActorHasTag(Block0PopulationCompleteTag)",
    "full_map_foliage_population_incomplete",
    "full_sector_population=1",
    "population_complete=1",
    "density_policy_only=1",
):
    require(foliage_guard, needle, "full-map foliage runtime completion gate")

# Low FPS remains evidence, not permission for hidden graphics degradation.
for needle in (
    "PASS15_PERF_PROBE", "PASS39_LOW_FPS_PROBE_DIAGNOSTIC", "quality_mutation=0",
    "PASS15_PERF_SAMPLE", "PASS15_PERF_BELOW_TARGET", "PASS15_PERF_30FPS_READY",
    "ReportLowFpsProbe", "virtual bool IsTickable() const override { return !bFinished; }",
):
    require(perf_h + perf, needle, "diagnostic-only performance sampling")
for forbidden in (
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "UKismetSystemLibrary::ExecuteConsoleCommand",
    "r.ScreenPercentage 65",
    "sg.ShadowQuality 0",
):
    forbid(perf, forbidden, "performance sampler must not mutate graphics quality")

# Focused recovery proves playability, not exact final art. It must use current Pass45 deployment truth.
for needle in (
    '/C:"fix/pass45-runtime-rejection-"',
    "PASS15_FRONTEND_FIELDS_OPAQUE_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    'findstr /C:"PASS45_INITIAL_BASE_DEPLOYMENT_" "%LOG%"',
    'findstr /C:"vehicle_revalidation=0"',
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_SAMPLE", "PASS15_PERF_BELOW_TARGET", "PASS15_PERF_30FPS_READY",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "exact production-art certification", "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "focused Pass45 runtime launcher")
for forbidden in (
    "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM",
    "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS7_PRODUCTION_WEAPONS_READY",
    'findstr /C:"PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL"',
):
    forbid(launcher, forbidden, "retired focused recovery dependency")

print("RUNTIME RECOVERY PASS15/PASS45 SOURCE CONTRACT PASS")
print("- frontend/server recovery and physical 11-weapon Museum rack remain required")
print("- Museum BASE correction is initial-character-only and vehicle revalidation is forbidden")
print("- Pass45 foliage covers the compact 960x940m playable Oster area in both profiles; LowCPU reduces density instead of cropping the city")
print("- foliage runtime READY requires the full-map population-complete tag, preventing early partial-population false green")
print("- focused launcher accepts the current Pass45 terminal deployment evidence")
print("- low-FPS probe remains diagnostic-only and does not destroy graphics quality")
print("- exact production-art certification remains separate")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime acceptance still required")
