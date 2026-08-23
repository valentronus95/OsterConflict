#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
PERF = SRC / "Private" / "OCPerformanceSampleSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS18 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS18 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS18 VERIFY FAIL: {label}: forbidden {needle!r}")


perf = read(PERF)
launcher = read(LAUNCHER)

# Diagnostics are captured once beside the settled final FPS sample, not every frame.
for needle in (
    '#include "Components/InstancedStaticMeshComponent.h"',
    '#include "EngineUtils.h"',
    '#include "HAL/PlatformMemory.h"',
    '#include "NavigationSystem.h"',
    "void LogPass18WorldDiagnostics(UWorld* World, APlayerController* PC)",
    "LogPass18WorldDiagnostics(World, PC);",
    "if (SampleSeconds < 8.0f) return;",
    "bFinished = true;",
):
    require(perf, needle, "settled one-shot diagnostics")

# Capture concrete world-density and memory evidence that can explain CPU/nav/world pressure.
for needle in (
    "TActorIterator<AActor>",
    "TInlineComponentArray<UInstancedStaticMeshComponent*>",
    "Component->GetInstanceCount()",
    "Component->IsVisible()",
    "Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision",
    "FPlatformMemory::GetStats()",
    "FNavigationSystem::GetCurrent<UNavigationSystemV1>(World)",
    "PC->GetPawn()->GetActorLocation()",
    "PASS18_WORLD_PERF_DIAGNOSTICS actors=%d ism_components=%d ism_instances=%d visible_ism_instances=%d colliding_ism_instances=%d nav_system=%d ram_used_mib=%.0f ram_available_mib=%.0f",
):
    require(perf, needle, "world performance evidence")

# Pass 18 must remain diagnostic-only: no runtime destruction, scalability mutation or collision changes here.
forbid(perf, "PASS18_EMERGENCY", "Pass 18 changing scalability")
forbid(perf, "PASS18_WORLD_PERF_DIAGNOSTICS_FAIL", "Pass 18 inventing a performance verdict")

# Keep the exact existing FPS acceptance markers intact.
for needle in (
    "PASS15_PERF_PROBE",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "PASS15_PERF_SAMPLE",
    "PASS15_PERF_BELOW_TARGET",
    "PASS15_PERF_30FPS_READY",
):
    require(perf, needle, "existing FPS contract remains intact")

# The existing R17 launcher is the single entry point and now requires/prints the diagnostic line.
for needle in (
    'set "VERIFY18=%~dp0VERIFY_RUNTIME_DIAGNOSTICS_PASS_18.py"',
    '%PY_CMD% "%VERIFY18%"',
    "PASS18_WORLD_PERF_DIAGNOSTICS",
    "Missing Pass 18 world performance diagnostics",
    "PASS 17-18 RUNTIME PERFORMANCE ACCEPTANCE: PASSED",
):
    require(launcher, needle, "single performance acceptance launcher")

print("RUNTIME DIAGNOSTICS PASS 18 SOURCE CONTRACT PASS")
print("- final settled FPS window also records actor and ISM world density")
print("- visible/colliding ISM instance counts expose render/collision pressure")
print("- navigation-system presence and physical RAM usage are captured")
print("- player world coordinates tie diagnostics to the sampled gameplay location")
print("- Pass 18 is diagnostic-only and does not change the existing 30 FPS verdict")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime log is required for actual measurements")
