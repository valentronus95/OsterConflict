#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS37 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS37 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS37 VERIFY FAIL: {label}: forbidden {needle!r}")


spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
spawn_guard = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
museum_h = read(SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h")
museum = read(SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp")
palette_h = read(SRC / "Public" / "OCWeaponPalettePass37Subsystem.h")
palette = read(SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# Latest runtime rejected the 41 m BASE as visually too far. Canonical placement must now be the
# closer front-side approach and must face back toward the museum.
for needle in (
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
    "FVector(-2300.0f, -3100.0f, 120.0f)",
    "FVector(2300.0f, -3100.0f, 120.0f)",
    "Team == EOCTeam::TeamTwo ? 120.0f : 60.0f",
    "PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH",
    "PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM",
):
    require(spawn, needle, "closer canonical museum BASE")

for needle in (
    "MuseumNoSpawnRadiusCm = 2000.0f",
    "PrimaryBaseRadiusCm = 4500.0f",
    "BaseDeploymentAcceptanceRadiusCm = 5000.0f",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
):
    require(spawn_guard, needle, "deployment guard aligned with closer BASE")

# An R13.8 owner tag is no longer enough. We require visible registered structural components near
# MuseumAnchor and keep polling beyond the old delayed R13.8 5.35 s startup to retire duplicates.
require(museum_h, "UOCMuseumVisibilityPass37Subsystem", "visible museum guard class")
for needle in (
    "MinVisibleStructuralComponents = 12",
    "LateStartupSettleSeconds = 6.40f",
    'MuseumStructuralTag(TEXT("MuseumStructural"))',
    "Component->IsRegistered()",
    "Component->IsVisible()",
    "Component->Bounds.Origin",
    "RunAuthoritativeUpgradeNow(*World)",
    "RetireOtherArchitectureOwners",
    "PASS37_MUSEUM_VISIBLE_CORE_REBUILD",
    "PASS37_MUSEUM_DUPLICATE_ARCHITECTURE_RETIRED",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
):
    require(museum, needle, "actual visible museum structural proof")

# Runtime screenshot proves non-null restored Stein materials can still be visually blank. Pass 37
# explicitly palettes those known incomplete restored payloads while preserving the already-correct AK.
require(palette_h, "UOCWeaponPalettePass37Subsystem", "weapon palette class")
for needle in (
    "IsRestoredSteinPayload",
    'Name.Contains(TEXT("MP5")',
    'Name.Contains(TEXT("M1911")',
    'Name.Contains(TEXT("M700")',
    'Name.Contains(TEXT("M14")',
    'Name.Contains(TEXT("MAC-10")',
    'Name.Contains(TEXT("TEC-9")',
    'Name.Contains(TEXT("Lever")',
    "IsClearlyPlaceholderMaterial",
    "BasicShapeMaterial",
    "WorldGridMaterial",
    "if (IsAK(Name))",
    "PASS37_WEAPON_VISIBLE_PALETTE_APPLIED",
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
):
    require(palette, needle, "visible weapon palette recovery")

forbid(palette, 'Name.Contains(TEXT("AK-47")', "AK force-palette path must not exist")

# Full test must fail closed on the exact three repeated complaints, while retaining the 30 FPS floor.
for marker in (
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS37_BASE_DEPLOYMENT_VISIBLE_MUSEUM_APPROACH",
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"Pass 37 runtime acceptance marker {marker}")
require(acceptance, "20-45 m museum approach", "explicit closer spawn acceptance")
require(acceptance, "30 FPS acceptance target", "performance floor remains 30 FPS")

print("VISIBLE MUSEUM + WEAPON PALETTE PASS 37 SOURCE CONTRACT PASS")
print("- primary BASE is ~27.8 m from MuseumAnchor and faces the museum")
print("- museum acceptance proves visible structural components near the anchor, not actor tags")
print("- late duplicate R13.8 owners are retired through the delayed startup window")
print("- known blank restored Stein weapon payloads receive a visible weapon-specific palette; AK remains authored")
print("- runtime acceptance still fails below 30 FPS")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 visual/runtime acceptance remains required")
