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

for needle in (
    "TActorIterator<AOCWorldSectorOster>",
    "Attempts >= 20",
    "SetTimer(",
    "0.5f",
    "PASS17_WORLD_ISM_BUDGET_NOT_APPLIED",
    "PASS17_WORLD_ISM_BUDGET_READY",
    "GAME_RECOVERY_COMPACT_WORLD_BUDGET_READY",
):
    require(cpp, needle, "world-sector retry and evidence")

# Keep the verifier aligned with the current compact-map performance budget. Older Pass17 values kept detail,
# trees and grass visible much farther than the later GAME_RECOVERY budget and would reintroduce the FPS regression.
required_budgets = {
    "Roads": (0, 50000, "false"),
    "Sidewalks": (4000, 24000, "false"),
    "Buildings": (18000, 48000, "true"),
    "ResidentialRoofs": (16000, 36000, "false"),
    "ResidentialDetails": (3000, 14000, "false"),
    "LandmarkBlocks": (26000, 65000, "true"),
    "LandmarkRoofs": (22000, 50000, "false"),
    "LandmarkWindows": (3000, 18000, "false"),
    "LandmarkDetails": (5000, 22000, "false"),
    "Fences": (3000, 18000, "false"),
    "WoodFences": (3000, 18000, "false"),
    "MetalFences": (3000, 18000, "false"),
    "LightSheetFences": (3000, 18000, "false"),
    "AuthoredDeciduousTrees": (7000, 24000, "false"),
    "AuthoredPine01Trees": (7000, 26000, "false"),
    "AuthoredPine03Trees": (7000, 26000, "false"),
    "GrassMown": (0, 7000, "false"),
    "GrassRough": (0, 8000, "false"),
    "GrassWetland": (0, 9000, "false"),
    "StadiumGeometry": (0, 38000, "false"),
    "StadiumDetails": (3000, 18000, "false"),
    "ParkGeometry": (0, 36000, "false"),
    "ParkDetails": (3000, 18000, "false"),
    "Waterways": (0, 42000, "false"),
    "Bridges": (18000, 52000, "false"),
    "ReferenceMarkers": (0, 2500, "false"),
}
for name, (start, end, shadow) in required_budgets.items():
    pattern = rf'\{{\s*TEXT\("{re.escape(name)}"\),\s*{start},\s*{end},\s*{shadow}\s*\}}'
    if not re.search(pattern, cpp):
        raise SystemExit(f"PASS17 VERIFY FAIL: compact render budget mismatch for {name}")

for stale_tree in (
    "TreeTrunks", "TreeCrowns", "SovietPoplarTrunks", "SovietPoplarCrowns",
    "BirchTrunks", "BirchCrowns", "PineTrunks", "PineCrowns",
):
    forbid(cpp, f'TEXT("{stale_tree}")', "retired primitive tree budget returned")

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

forbid(cpp, "SetCollisionProfileName(", "Pass 17/45 changing collision profiles")
forbid(cpp, "SetCollisionEnabled(", "Pass 17/45 changing collision state")
forbid(cpp, "DestroyComponent", "Pass 17/45 deleting world components")

# Gameplay collision still exists for solid authored vegetation, but it is now owned by real tree meshes.
for needle in (
    'Buildings = MakeISM(TEXT("Buildings"), TEXT("BlockAll"))',
    'Fences = MakeISM(TEXT("Fences"), TEXT("BlockAll"))',
    'WoodFences = MakeISM(TEXT("WoodFences"), TEXT("BlockAll"))',
    'AuthoredDeciduousTrees = MakeISM(TEXT("AuthoredDeciduousTrees"), TEXT("BlockAll"))',
    'AuthoredPine01Trees = MakeISM(TEXT("AuthoredPine01Trees"), TEXT("BlockAll"))',
    'AuthoredPine03Trees = MakeISM(TEXT("AuthoredPine03Trees"), TEXT("BlockAll"))',
):
    require(world, needle, "gameplay collision remains in world source")

for forbidden in (
    '/Engine/BasicShapes/Cylinder.Cylinder',
    '/Engine/BasicShapes/Sphere.Sphere',
):
    forbid(world, forbidden, "rejected primitive vegetation source returned")

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
print(f"- all {len(required_budgets)} current source-world ISM families use the compact 960x940 m cull budget")
print("- retired primitive tree budget entries are absent; authored deciduous/pine families are budgeted directly")
print("- historical 1200-1300 m broad family ranges stay retired")
print("- current GAME_RECOVERY compact cull ranges stay authoritative over obsolete wider Pass17 values")
print("- detail/fence/grass ranges are local while important silhouettes and authored trees remain longer")
print("- NoCollision decoration is removed from dynamic navigation participation")
print("- gameplay collision profiles are not modified or disabled by the render-budget subsystem")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 frontend/gameplay FPS and pop-in remain runtime-only")
