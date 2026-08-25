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

# Construction-order tolerance remains bounded and one-shot after the sector appears.
for needle in (
    "TActorIterator<AOCWorldSectorOster>",
    "Attempts >= 20",
    "SetTimer(",
    "0.5f",
    "PASS17_WORLD_ISM_BUDGET_NOT_APPLIED",
    "PASS17_WORLD_ISM_BUDGET_READY",
    "PASS45_COMPACT_WORLD_CULL_BUDGET_READY",
):
    require(cpp, needle, "world-sector retry and evidence")

# Pass 45 supersedes the old 700-1300 m family budgets. These values are the current compact 960x940 m
# source contract and may not be forward-ported back to historical long-distance numbers just to satisfy CI.
required_budgets = {
    "Roads": (0, 90000, "false"),
    "Sidewalks": (8000, 42000, "false"),
    "Buildings": (40000, 78000, "true"),
    "ResidentialRoofs": (30000, 58000, "false"),
    "ResidentialDetails": (6000, 24000, "false"),
    "LandmarkBlocks": (50000, 95000, "true"),
    "LandmarkRoofs": (40000, 76000, "false"),
    "LandmarkWindows": (6000, 30000, "false"),
    "LandmarkDetails": (10000, 40000, "false"),
    "Fences": (6000, 28000, "false"),
    "WoodFences": (6000, 28000, "false"),
    "MetalFences": (6000, 28000, "false"),
    "LightSheetFences": (6000, 28000, "false"),
    "TreeTrunks": (12000, 36000, "false"),
    "TreeCrowns": (12000, 36000, "false"),
    "SovietPoplarTrunks": (12000, 36000, "false"),
    "SovietPoplarCrowns": (12000, 36000, "false"),
    "BirchTrunks": (12000, 36000, "false"),
    "BirchCrowns": (12000, 36000, "false"),
    "PineTrunks": (12000, 36000, "false"),
    "PineCrowns": (12000, 36000, "false"),
    "GrassMown": (0, 16000, "false"),
    "GrassRough": (0, 18000, "false"),
    "GrassWetland": (0, 20000, "false"),
    "StadiumGeometry": (0, 55000, "false"),
    "StadiumDetails": (6000, 32000, "false"),
    "ParkGeometry": (0, 52000, "false"),
    "ParkDetails": (6000, 30000, "false"),
    "Waterways": (0, 60000, "false"),
    "Bridges": (30000, 75000, "true"),
    "ReferenceMarkers": (0, 3000, "false"),
}
for name, (start, end, shadow) in required_budgets.items():
    pattern = rf'\{{\s*TEXT\("{re.escape(name)}"\),\s*{start},\s*{end},\s*{shadow}\s*\}}'
    if not re.search(pattern, cpp):
        raise SystemExit(f"PASS17 VERIFY FAIL: compact render budget mismatch for {name}")

for forbidden_distance in ("130000", "120000"):
    forbid(cpp, forbidden_distance, "historical broad cull distance returned")

for needle in (
    "Component->SetCullDistances(Budget.StartCullCm, Budget.EndCullCm);",
    "Component->SetCastShadow(Budget.bCastShadow);",
    "Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision",
    "Component->SetCanEverAffectNavigation(false);",
    "Component->MarkRenderStateDirty();",
):
    require(cpp, needle, "ISM runtime tuning")

# Performance work must not achieve its numbers by deleting gameplay collision.
forbid(cpp, "SetCollisionProfileName(", "Pass 17/45 changing collision profiles")
forbid(cpp, "SetCollisionEnabled(", "Pass 17/45 changing collision state")
forbid(cpp, "DestroyComponent", "Pass 17/45 deleting world components")

# Underlying source still owns gameplay collision. Pass 45's tree guard may hide primitive visual families,
# but the render-budget subsystem itself must not silently rewrite collision contracts.
for needle in (
    'Buildings = MakeISM(TEXT("Buildings"), TEXT("BlockAll"))',
    'Fences = MakeISM(TEXT("Fences"), TEXT("BlockAll"))',
    'WoodFences = MakeISM(TEXT("WoodFences"), TEXT("BlockAll"))',
    'TreeTrunks = MakeISM(TEXT("TreeTrunks"), TEXT("BlockAll"))',
):
    require(world, needle, "gameplay collision remains in world source")

# Historical runtime wrapper remains usable for evidence; Pass 45 adds stricter frontend/gameplay evidence elsewhere.
for needle in (
    'set "BASE_LAUNCHER=%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"',
    'set "VERIFY17=%~dp0VERIFY_RUNTIME_PERFORMANCE_PASS_17.py"',
    'set "VERIFY18=%~dp0VERIFY_RUNTIME_DIAGNOSTICS_PASS_18.py"',
    'call "%BASE_LAUNCHER%"',
    "PASS17_WORLD_ISM_BUDGET_NOT_APPLIED",
    "PASS17_WORLD_ISM_BUDGET_READY",
    "PASS16_RUNTIME_GRAPHICS_IDENTITY",
    "PASS15_PERF_30FPS_READY",
    "PASS 17-18 RUNTIME PERFORMANCE ACCEPTANCE: PASSED",
):
    require(launcher, needle, "Pass 17 runtime acceptance launcher")

print("RUNTIME PERFORMANCE PASS 17/45 SOURCE CONTRACT PASS")
print("- all 31 source-world ISM families use the current compact 960x940 m cull budget")
print("- historical 1200-1300 m broad family ranges stay retired")
print("- detail/fence/grass/proxy vegetation ranges are local while important silhouettes remain longer")
print("- NoCollision decoration is removed from dynamic navigation participation")
print("- gameplay collision profiles are not modified or disabled by the render-budget subsystem")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 frontend/gameplay FPS and pop-in remain runtime-only")
