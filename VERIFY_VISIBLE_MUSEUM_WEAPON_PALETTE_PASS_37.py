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


def absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS37 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


spawn = read(SRC / "Private" / "OCTeamSpawnPoint.cpp")
spawn_guard = read(SRC / "Private" / "OCMuseumSpawnGuardSubsystem.cpp")
spawn_guard_h = read(SRC / "Public" / "OCMuseumSpawnGuardSubsystem.h")
fallback_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
fallback = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# Pass 45 deletes the old destructive visibility rebuild guard and the already-retired palette shell.
for path, label in (
    (SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h", "Pass37 Museum visibility guard"),
    (SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp", "Pass37 Museum visibility guard"),
    (SRC / "Public" / "OCWeaponPalettePass37Subsystem.h", "Pass37 weapon palette shell"),
    (SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp", "Pass37 weapon palette shell"),
):
    absent(path, label)

for needle in (
    "FVector(-1400.0f, -2400.0f, 120.0f)",
    "FVector(1400.0f, -2400.0f, 120.0f)",
    "RequiredRackWeaponCount = 11",
    "PASS37_BASE_RELOCATED_VISIBLE_MUSEUM_APPROACH",
    "PASS37_RUNTIME_BASE_RACK_NEAR_MUSEUM",
):
    require(spawn, needle, "canonical Museum BASE/rack")

# Current spawn correction validates an AOCCharacter once. Vehicle possession/unpossession cannot become a new BASE recovery.
for needle in (
    "ValidatedBaseDeploymentControllers",
    "AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn())",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "vehicle_revalidation=0",
):
    require(spawn_guard_h + spawn_guard, needle, "Pass45 initial-only Museum deployment")
forbid(spawn_guard_h + spawn_guard, "LastValidatedPawnByController", "legacy arbitrary-pawn deployment cache")

# Missing/default authored materials remain fail-visible in the bounded real-weapon audit. No palette owner survives.
for needle in (
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_READY",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "reason=material_gap_audited",
    "permanent_scan=0",
):
    require(fallback_h + fallback, needle, "truth-only bounded weapon audit")
for forbidden in (
    "UMaterialInstanceDynamic::Create",
    "Component->SetMaterial(Slot",
):
    forbid(fallback, forbidden, "weapon audit may not repaint authored slots")

# Acceptance must follow the current owner set, not demand deleted Pass37 visibility/palette markers.
for forbidden_marker in (
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
):
    forbid(acceptance, forbidden_marker, "stale runtime marker must not be required")
for marker in (
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"current runtime acceptance marker {marker}")

print("VISIBLE MUSEUM + WEAPON MATERIAL PASS 37/44 FORWARD-PORTED SOURCE CONTRACT PASS")
print("- destructive Museum visibility/rebuild guard is physically retired")
print("- retired weapon palette compatibility owner is physically retired")
print("- Museum BASE recovery is initial-character-only")
print("- authored material gaps remain fail-visible through the bounded real-weapon audit")
print("STATUS: SOURCE CONTRACT ONLY; actual UE 5.8 visual/runtime acceptance remains required")
