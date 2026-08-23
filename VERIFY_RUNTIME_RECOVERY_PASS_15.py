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
museum = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
foliage = read(SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp")
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

for needle in (
    "PASS15_MUSEUM_BASES_WEAPONS_READY", "RequiredRackWeaponCount = 11", "ValidateBaseDeployments()",
    "GetRequestedDeploymentSpawn()", "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM", "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL", "Pawn->SetActorLocationAndRotation",
):
    require(museum, needle, "Museum BASE and actual pawn recovery")

for needle in (
    "RequiredRackWeaponCount = 11", "AOCWeapon_M14::StaticClass()", "AOCWeapon_Mac10::StaticClass()",
    "AOCWeapon_Tec9::StaticClass()", "AOCWeapon_LeverAction::StaticClass()", "AOCAntiArmorLauncher::StaticClass()",
    "Weapon->Tags.Add(RuntimeBaseRackTag)",
):
    require(spawn, needle, "physical Museum weapon rack")

grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
if not grid or float(grid.group(1)) < 2000.0:
    raise SystemExit("PASS15 VERIFY FAIL: foliage grid exceeds low-cost recovery density")
for needle in ("PASS15_PERF_PROBE", "PASS15_EMERGENCY_PERF_PROFILE_APPLIED", "PASS15_PERF_SAMPLE", "PASS15_PERF_BELOW_TARGET", "PASS15_PERF_30FPS_READY"):
    require(perf, needle, "adaptive performance recovery")

# Focused recovery proves playability, not exact final art. Pass 7 exact-production certification is a separate strict gate.
for needle in (
    "PASS15_FRONTEND_FIELDS_OPAQUE_READY", "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM", "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS19_PLAYABLE_WEAPON_SET_READY", "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY", "PASS15_PERF_SAMPLE", "PASS15_PERF_BELOW_TARGET", "PASS15_PERF_30FPS_READY",
    "exact production-art certification", "R14_CURRENT_GAMEPLAY.log",
):
    require(launcher, needle, "focused runtime launcher")
forbid(launcher, "PASS7_PRODUCTION_WEAPONS_READY", "focused launcher must not certify exact production art")
forbid(launcher, 'findstr /C:"PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL"', "focused launcher must not block on separate exact-art gate")

print("RUNTIME RECOVERY PASS 15 SOURCE CONTRACT PASS")
print("- frontend/server recovery, Museum BASE, physical rack and FPS evidence remain required")
print("- focused recovery now requires Pass 19 playable real-mesh rack readiness")
print("- Pass 7 exact-production-art certification remains separate and may correctly fail")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime acceptance still required")
