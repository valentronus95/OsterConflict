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

# Runtime after Pass 37 showed a catastrophic 60 -> 5 FPS fall. Visible-component proof remains, but the
# destructive recovery path is limited to one attempt. Pass 42 moves normal R13.7/R13.8 construction earlier,
# so the guard starts after the normal build rather than waiting through the old 5.8 second settle window.
require(museum_h, "UOCMuseumVisibilityPass37Subsystem", "visible museum guard class")
for needle in (
    "MinVisibleStructuralComponents = 12",
    "FirstPollDelaySeconds = 1.45f",
    "PollIntervalSeconds = 0.35f",
    "LateStartupSettleSeconds = 2.20f",
    "MaxRebuildAttempts = 1",
    'MuseumStructuralTag(TEXT("MuseumStructural"))',
    "Component->IsRegistered()",
    "Component->IsVisible()",
    "Component->Bounds.Origin",
    "RebuildAttemptCount < MaxRebuildAttempts",
    "RunAuthoritativeUpgradeNow(*World)",
    "RetireOtherArchitectureOwners",
    "PASS37_MUSEUM_VISIBLE_CORE_REBUILD",
    "PASS37_MUSEUM_DUPLICATE_ARCHITECTURE_RETIRED",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS37_MUSEUM_VISIBLE_CORE_FAIL",
    "PASS38_MUSEUM_SINGLE_REBUILD_EXECUTED",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS42_MUSEUM_EARLY_VISIBILITY_READY",
):
    require(museum, needle, "bounded visible museum structural proof")

# Pass 37's forced recolouring is disproven by the flat orange Lever Action screenshot. Exact/source materials
# are preserved whenever they are not obvious placeholders; only placeholder slots may receive a recovery MID.
require(palette_h, "UOCWeaponPalettePass37Subsystem", "weapon palette class")
for needle in (
    "IsRestoredSteinPayload",
    "IsClearlyPlaceholderMaterial",
    "if (!bPlaceholder) continue;",
    "authored_materials_preserved=1",
    "if (IsAK(Name))",
    "PASS37_WEAPON_VISIBLE_PALETTE_APPLIED",
    "PASS37_WEAPON_VISIBLE_PALETTE_READY",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
):
    require(palette, needle, "placeholder-only weapon presentation recovery")
forbid(palette, "if (!bForceRestoredPalette && !bPlaceholder) continue;",
       "forced restored-payload material overwrite must stay removed")
forbid(palette, "forced_restored_slots=", "old forced-palette evidence is no longer valid")

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

print("VISIBLE MUSEUM + WEAPON PALETTE PASS 37/38/42 SOURCE CONTRACT PASS")
print("- primary BASE stays ~27.8 m from MuseumAnchor and faces the museum")
print("- museum visibility proof remains but destructive rebuilding is bounded to one attempt")
print("- Pass 42 starts visibility proof after the earlier one-shot museum build and settles by 2.20 seconds")
print("- non-placeholder imported weapon materials are preserved; only obvious placeholders may be recovered")
print("- runtime acceptance still fails below 30 FPS")
print("STATUS: SOURCE VERIFIED; actual UE 5.8 visual/runtime acceptance remains required")
