#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCRegionalGroundDetailSubsystem.cpp"

text = SOURCE.read_text(encoding="utf-8", errors="replace")
errors: list[str] = []


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


for needle in (
    "/Game/KiteDemo/Environments/Foliage/Leaves/SM_DeadLeaves.SM_DeadLeaves",
    "AuthoredDeciduousTrees",
    "FullMaxLeafInstances = 240",
    "LowCPUMaxLeafInstances = 96",
    "Leaves->SetCollisionEnabled(ECollisionEnabled::NoCollision)",
    "Leaves->SetCanEverAffectNavigation(false)",
    "Leaves->SetCastShadow(false)",
    "Leaves->SetCullDistances(250, 6500)",
    "FRandomStream RandomStream(0x4F535445)",
    "IsBlockedGroundDetailSurface(Hit)",
    "candidate_surface_guard=1",
    "water_surface_guard=1",
    "permanent_tick=0",
    "runtime_acceptance=0",
):
    require(needle in text, f"missing regional ground-detail contract: {needle}")

for term in (
    'TEXT("road")',
    'TEXT("path")',
    'TEXT("building")',
    'TEXT("water")',
    'TEXT("river")',
    'TEXT("lake")',
    'TEXT("pond")',
    'TEXT("canal")',
    'TEXT("reservoir")',
    'TEXT("NoFoliage")',
):
    require(term in text, f"missing blocked-surface term/tag: {term}")

require("Hit.ImpactNormal.Z < 0.82f" in text, "regional detail lost slope rejection")
require("BlockedSurfaceRejected" in text, "blocked-surface rejection is not observable")
require("PrimaryActorTick" not in text, "regional detail introduced an actor tick owner")
require("SetTimer(" in text and "0.30f" in text and "false);" in text,
        "regional detail is no longer a bounded one-shot deferred pass")

if errors:
    print("PASS45 BLOCK0 REGIONAL GROUND DETAIL: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 BLOCK0 REGIONAL GROUND DETAIL: PASS")
print("- sparse authored SM_DeadLeaves detail remains bounded and deterministic")
print("- LowCPU/full instance caps, culling and zero collision/navigation/tick ownership are protected")
print("- road/building/water/river/lake/canal/NoFoliage hits are rejected before AddInstance")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 runtime visual acceptance remains required")
