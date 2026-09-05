#!/usr/bin/env python3
from pathlib import Path
import re
from pass45_runtime_route_contract import ROOT, require, validate_runtime_route

SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
def src(rel: str) -> str:
    return (SRC / rel).read_text(encoding="utf-8", errors="replace")

recovery_h = src("Public/OCRuntimeRecoveryPass15Subsystem.h")
recovery = src("Private/OCRuntimeRecoveryPass15Subsystem.cpp")
museum_h = src("Public/OCMuseumSpawnGuardSubsystem.h")
museum = src("Private/OCMuseumSpawnGuardSubsystem.cpp")
spawn = src("Private/OCTeamSpawnPoint.cpp")
foliage = src("Private/OCDenseGroundFoliageSubsystem.cpp")
perf_h = src("Public/OCPerformanceSampleSubsystem.h")
perf = src("Private/OCPerformanceSampleSubsystem.cpp")
route = validate_runtime_route()

for needle in ("ApplyFrontendRepairs", "ApplyJoinPendingOverlay", "RunHostTravelFallback"):
    require(recovery_h, needle, "recovery header")
for needle in ("PASS15_FRONTEND_FIELDS_OPAQUE_READY", "PASS15_DIRECT_CONNECT_OVERLAY_BEGIN", "PASS15_CONNECTION_FAILURE_RETURN_FRONTEND", "PASS15_HOST_OPENLEVEL_FALLBACK", "UGameplayStatics::OpenLevel"):
    require(recovery, needle, "frontend/server recovery")
for needle in ("RequiredRackWeaponCount = 11", "ValidateBaseDeployments()", "ValidatedBaseDeploymentControllers", "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL", "vehicle_revalidation=0"):
    require(museum_h + museum, needle, "initial character BASE recovery")
for needle in ("RequiredRackWeaponCount = 11", "AOCWeapon_M14::StaticClass()", "AOCWeapon_Mac10::StaticClass()", "AOCWeapon_Tec9::StaticClass()", "AOCWeapon_LeverAction::StaticClass()", "AOCAntiArmorLauncher::StaticClass()"):
    require(spawn, needle, "physical weapon rack")
grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
if not grid or float(grid.group(1)) < 2000.0:
    raise SystemExit("PASS15 VERIFY FAIL: foliage grid exceeds recovery density")
for needle in ("PASS15_PERF_PROBE", "PASS39_LOW_FPS_PROBE_DIAGNOSTIC", "quality_mutation=0", "PASS15_PERF_BELOW_TARGET", "PASS15_PERF_30FPS_READY"):
    require(perf_h + perf, needle, "diagnostic performance sampling")
for needle in ("PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL", "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS14_PERF_30FPS_READY", "PASS14_PERF_BELOW_TARGET"):
    require(route["evidence"], needle, "canonical runtime evidence")

print("RUNTIME RECOVERY PASS15/PASS45 SOURCE CONTRACT PASS")
print("- runtime recovery stays intact while the packet runner owns strict acceptance")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime acceptance still required")
