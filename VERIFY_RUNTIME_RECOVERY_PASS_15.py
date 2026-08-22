#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

RECOVERY_H = SRC / "Public" / "OCRuntimeRecoveryPass15Subsystem.h"
RECOVERY = SRC / "Private" / "OCRuntimeRecoveryPass15Subsystem.cpp"
MUSEUM_H = SRC / "Public" / "OCMuseumSpawnGuardSubsystem.h"
MUSEUM = SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp"
SPAWN = SRC / "Private" / "OCTeamSpawnPoint.cpp"
FOLIAGE = SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp"
PERF_H = SRC / "Public" / "OCPerformanceSampleSubsystem.h"
PERF = SRC / "Private" / "OCPerformanceSampleSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"


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


recovery_h = read(RECOVERY_H)
recovery = read(RECOVERY)
museum_h = read(MUSEUM_H)
museum = read(MUSEUM)
spawn = read(SPAWN)
foliage = read(FOLIAGE)
perf_h = read(PERF_H)
perf = read(PERF)
launcher = read(LAUNCHER)

# Server/network setup must be readable instead of the default white Slate fields over a translucent panel.
for needle in (
    "UOCRuntimeRecoveryPass15Subsystem",
    "ApplyFrontendRepairs",
    "ApplyJoinPendingOverlay",
    "RunHostTravelFallback",
):
    require(recovery_h, needle, "recovery subsystem header")
for needle in (
    "PASS15_FRONTEND_FIELDS_OPAQUE_READY",
    "FEditableTextBoxStyle Style = Field->GetWidgetStyle();",
    "Style.SetBackgroundColor",
    "Field->SetWidgetStyle(Style);",
    "FLinearColor(0.025f, 0.030f, 0.035f, 0.96f)",
    "PASS15_DIRECT_CONNECT_OVERLAY_BEGIN",
    "PASS15_CONNECTION_FAILURE_RETURN_FRONTEND",
    "PASS15_HOST_OPENLEVEL_FALLBACK",
    "UGameplayStatics::OpenLevel",
):
    require(recovery, needle, "frontend/host/join recovery")

# Museum BASE must no longer wait forever for the world-sector actor. A real authoritative GameMode is enough.
for needle in (
    "ValidationAccumulator",
    "LastValidatedPawnByController",
    "ValidateBaseDeployments",
):
    require(museum_h, needle, "persistent Museum deployment guard")
for needle in (
    "GameMode->IsFrontendOnlySession()",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "RequiredRackWeaponCount = 11",
    "CountRackWeaponsNear",
    "TeamOnePrimary->ConfigureServer",
    "TeamTwoPrimary->ConfigureServer",
    "ValidateBaseDeployments()",
    "GetRequestedDeploymentSpawn()",
    "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM",
    "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "Pawn->SetActorLocationAndRotation",
):
    require(museum, needle, "Museum BASE and actual pawn recovery")
forbid(museum, "bGameplaySectorPresent", "obsolete world-sector wait gate")

# Rack readiness must be based on 11 physical tagged weapon actors, not a tag on the spawn point.
for needle in (
    "RequiredRackWeaponCount = 11",
    "CollectRackWeaponsNear",
    "ExistingCount >= RequiredRackWeaponCount",
    "Existing->Destroy();",
    "AOCWeapon_M14::StaticClass()",
    "AOCWeapon_Mac10::StaticClass()",
    "AOCWeapon_Tec9::StaticClass()",
    "AOCWeapon_LeverAction::StaticClass()",
    "AOCAntiArmorLauncher::StaticClass()",
    "Weapon->Tags.Add(RuntimeBaseRackTag)",
    "Runtime BASE weapon rack rebuilt beside museum",
):
    require(spawn, needle, "physical Museum weapon rack")

# Pass 14 was still ~5 FPS. Pass 15 further lowers the static foliage budget.
grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
batch = re.search(r"constexpr\s+int32\s+CellsPerBatch\s*=\s*(\d+)\s*;", foliage)
if not grid or float(grid.group(1)) < 2000.0:
    raise SystemExit("PASS15 VERIFY FAIL: foliage grid did not move to the low-cost laptop recovery budget")
if not batch or not 1 <= int(batch.group(1)) <= 16:
    raise SystemExit("PASS15 VERIFY FAIL: foliage batch exceeds the Pass 15 CPU ceiling")
for needle in (
    "RandomStream.RandRange(1, 2)",
    "DenseGrass_%d",
    "9000",
    "PASS15_FOLIAGE_BUDGET_READY",
):
    require(foliage, needle, "Pass 15 foliage budget")

# Runtime performance evidence gets a real probe and an unsaved emergency profile only when <20 FPS.
for needle in (
    "ProbeSeconds",
    "ProbeFrames",
    "bEmergencyProfileApplied",
    "ApplyEmergencyPlaytestProfile",
):
    require(perf_h, needle, "adaptive performance header")
for needle in (
    "PASS15_PERF_PROBE",
    "ProbeFps < 20.0f",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "r.ScreenPercentage 65",
    "sg.ShadowQuality 0",
    "sg.GlobalIlluminationQuality 0",
    "sg.ReflectionQuality 0",
    "sg.FoliageQuality 0",
    "PASS15_PERF_SAMPLE",
    "PASS15_PERF_BELOW_TARGET",
    "PASS15_PERF_30FPS_READY",
):
    require(perf, needle, "adaptive performance recovery")

# Focused launcher is intentionally independent of production vehicle intake. It must prove the actual pawn spawn,
# physical/production weapon rack and measured FPS before accepting the recovery run.
for needle in (
    "PASS15_FRONTEND_FIELDS_OPAQUE_READY",
    "PASS15_MUSEUM_BASES_WEAPONS_READY",
    "PASS15_BASE_DEPLOYMENT_NEAR_MUSEUM",
    "PASS15_BASE_DEPLOYMENT_RECOVERED",
    "PASS15_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS7_PRODUCTION_WEAPONS_READY",
    "PASS7_PRODUCTION_WEAPON_RUNTIME_FAIL",
    "PASS15_PERF_SAMPLE",
    "PASS15_PERF_BELOW_TARGET",
    "PASS15_PERF_30FPS_READY",
    "R14_CURRENT_GAMEPLAY.log",
    "BTR/HMMWV production intake is intentionally NOT part",
):
    require(launcher, needle, "Pass 15 runtime launcher")

print("RUNTIME RECOVERY PASS 15 SOURCE CONTRACT PASS")
print("- server/network fields are dark and setup pages are effectively opaque")
print("- host has an OpenLevel fallback; direct-connect stays behind an opaque pending/error presentation")
print("- Museum BASE no longer depends on AOCWorldSectorOster actor timing")
print("- actual BASE-selected human pawns are verified/recovered to Museum, not merely BASE actors")
print("- Museum rack readiness requires 11 physical tagged weapon actors")
print("- foliage budget is reduced again after the measured ~5 FPS playtest")
print("- <20 FPS probe applies an unsaved emergency graphics profile before final sampling")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime acceptance still required")
