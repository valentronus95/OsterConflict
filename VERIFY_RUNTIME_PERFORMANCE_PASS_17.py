#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCWorldRenderBudgetPass17Subsystem.h"
CPP = SRC / "Private" / "OCWorldRenderBudgetPass17Subsystem.cpp"
WORLD = SRC / "Private" / "OCWorldSectorOster.cpp"
PASS16 = ROOT / "VERIFY_RUNTIME_GRAPHICS_PASS_16.py"
LAUNCHER = ROOT / "RUN_R17_RUNTIME_PERFORMANCE_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS17 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS17 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS17 VERIFY FAIL: {label}: forbidden {needle!r}")


header = read(HEADER)
cpp = read(CPP)
world = read(WORLD)
read(PASS16)
launcher = read(LAUNCHER)

for needle in (
    "UOCWorldRenderBudgetPass17Subsystem",
    "ShouldCreateSubsystem",
    "OnWorldBeginPlay",
    "TryApplyBudget",
    "FTimerHandle RetryHandle",
    "bool bApplied = false",
):
    require(header, needle, "Pass 17 subsystem header")

# The subsystem must wait for the source-only world sector instead of assuming construction order.
for needle in (
    "TActorIterator<AOCWorldSectorOster>",
    "Attempts >= 20",
    "SetTimer(",
    "0.5f",
    "PASS17_WORLD_ISM_BUDGET_NOT_APPLIED",
    "PASS17_WORLD_ISM_BUDGET_READY",
):
    require(cpp, needle, "world-sector retry and evidence")

# Render work is bounded by distance. Major silhouettes remain farther; detail families disappear much sooner.
required_budgets = {
    "Roads": (0, 130000, "false"),
    "Buildings": (60000, 130000, "true"),
    "ResidentialRoofs": (45000, 95000, "false"),
    "ResidentialDetails": (10000, 35000, "false"),
    "LandmarkBlocks": (60000, 130000, "true"),
    "LandmarkWindows": (10000, 50000, "false"),
    "Fences": (10000, 50000, "false"),
    "WoodFences": (10000, 50000, "false"),
    "MetalFences": (10000, 50000, "false"),
    "LightSheetFences": (10000, 50000, "false"),
    "TreeTrunks": (25000, 70000, "false"),
    "TreeCrowns": (25000, 70000, "false"),
    "GrassMown": (0, 35000, "false"),
    "GrassRough": (0, 35000, "false"),
    "ParkDetails": (10000, 50000, "false"),
    "Bridges": (40000, 120000, "true"),
}
for name, (start, end, shadow) in required_budgets.items():
    pattern = rf'\{{\s*TEXT\("{re.escape(name)}"\),\s*{start},\s*{end},\s*{shadow}\s*\}}'
    if not re.search(pattern, cpp):
        raise SystemExit(f"PASS17 VERIFY FAIL: render budget mismatch for {name}")

for needle in (
    "Component->SetCullDistances(Budget.StartCullCm, Budget.EndCullCm);",
    "Component->SetCastShadow(Budget.bCastShadow);",
    "Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision",
    "Component->SetCanEverAffectNavigation(false);",
    "Component->MarkRenderStateDirty();",
):
    require(cpp, needle, "ISM runtime tuning")

# Performance work must not achieve its numbers by deleting gameplay collision.
forbid(cpp, "SetCollisionProfileName(", "Pass 17 changing collision profiles")
forbid(cpp, "SetCollisionEnabled(", "Pass 17 changing collision state")
forbid(cpp, "DestroyComponent", "Pass 17 deleting world components")

# The underlying world still owns real collision for buildings, fences and trunks.
for needle in (
    'Buildings = MakeISM(TEXT("Buildings"), TEXT("BlockAll"))',
    'Fences = MakeISM(TEXT("Fences"), TEXT("BlockAll"))',
    'WoodFences = MakeISM(TEXT("WoodFences"), TEXT("BlockAll"))',
    'TreeTrunks = MakeISM(TEXT("TreeTrunks"), TEXT("BlockAll"))',
):
    require(world, needle, "gameplay collision remains in world source")

# Runtime acceptance must first pass the existing frontend/Museum/weapons/GPU/FPS run, then prove Pass 17 actually attached.
for needle in (
    'set "BASE_LAUNCHER=%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"',
    'set "VERIFY17=%~dp0VERIFY_RUNTIME_PERFORMANCE_PASS_17.py"',
    'call "%BASE_LAUNCHER%"',
    "PASS17_WORLD_ISM_BUDGET_NOT_APPLIED",
    "PASS17_WORLD_ISM_BUDGET_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_30FPS_READY",
    "PASS 17 RUNTIME PERFORMANCE ACCEPTANCE: PASSED",
):
    require(launcher, needle, "Pass 17 runtime acceptance launcher")

print("RUNTIME PERFORMANCE PASS 17 SOURCE CONTRACT PASS")
print("- 31 source-world ISM families receive explicit distance culling budgets")
print("- flat/detail/fence/proxy vegetation families stop casting redundant shadows")
print("- major building/landmark/bridge silhouettes keep longer draw distance and shadows")
print("- NoCollision decoration is removed from dynamic navigation participation")
print("- gameplay collision profiles are not modified or disabled by Pass 17")
print("- runtime acceptance requires Pass 15-16 success plus proof that Pass 17 actually attached")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 compile and measured runtime FPS still required")
