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
perf_h = read(SRC / "Public" / "OCPerformanceSampleSubsystem.h")
perf = read(SRC / "Private" / "OCPerformanceSampleSubsystem.cpp")
start_here = read(ROOT / "START_HERE.cmd")
runtime_evidence = read(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")

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

grid = re.search(r"constexpr\s+float\s+GridStep\s*=\s*([0-9.]+)f\s*;", foliage)
if not grid or float(grid.group(1)) < 2000.0:
    raise SystemExit("PASS15 VERIFY FAIL: foliage grid exceeds low-cost recovery density")

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

# The retired RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd is no longer an authority. START_HERE owns the
# actual runtime launch and the canonical Pass45 evidence verifier carries forward the relevant Pass15
# outcomes together with the newer deployment, weapon-readiness and >=30 FPS gates.
for needle in (
    'set "CURRENT_GAMEPLAY=%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'set "EVIDENCE_VERIFY=%~dp0VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"',
    'set "OC_FORCE_ACCEPTANCE=1"',
    'call "%CURRENT_GAMEPLAY%"',
    '%PY_CMD% "%EVIDENCE_VERIFY%"',
):
    require(start_here, needle, "canonical Pass45 runtime route")
for needle in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS19_PLAYABLE_WEAPON_SET_READY",
    "PASS19_PLAYABLE_WEAPON_SET_FAIL",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
    "PASS14_PERF_BELOW_TARGET",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
):
    require(runtime_evidence, needle, "canonical runtime evidence carry-forward")
forbid(start_here, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "retired Pass15 launcher")

print("RUNTIME RECOVERY PASS15/PASS45 SOURCE CONTRACT PASS")
print("- frontend/server recovery and physical 11-weapon Museum rack remain required")
print("- Museum BASE correction is initial-character-only and vehicle revalidation is forbidden")
print("- START_HERE + canonical Pass45 evidence replace the deleted focused Pass15 launcher")
print("- low-FPS probe remains diagnostic-only and does not destroy graphics quality")
print("- playable weapon readiness and >=30 FPS remain fail-closed runtime gates")
print("- exact production-art certification remains separate")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile/runtime acceptance still required")
