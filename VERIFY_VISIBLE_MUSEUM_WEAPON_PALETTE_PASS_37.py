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
layer_guard = read(SRC / "Private" / "OCMuseumLayerPerformanceGuardSubsystem.cpp")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")

# Pass45 deletes the old destructive visibility rebuild guard and the already-retired palette shell.
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

# Spawn correction validates the initial AOCCharacter once. Vehicle possession cannot become a new BASE recovery.
for needle in (
    "ValidatedBaseDeploymentControllers",
    "AOCCharacter* Character = Cast<AOCCharacter>(PC->GetPawn())",
    "PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE",
    "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE",
    "vehicle_revalidation=0",
):
    require(spawn_guard_h + spawn_guard, needle, "Pass45 initial-only Museum deployment")
forbid(spawn_guard_h + spawn_guard, "LastValidatedPawnByController", "legacy arbitrary-pawn deployment cache")

# Missing/default authored materials remain fail-visible in a bounded audit. Generic look-alike substitution is
# retired: exact identity belongs to the imported/local bridge. A hard budget stop is a FAIL marker, not READY.
for needle in (
    "PASS44_WEAPON_AUTHORED_MATERIAL_GAP",
    "PASS44_WEAPON_AUTHORED_MATERIAL_READY",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "permanent_scan=0",
    "generic_substitution=0",
    "PASS45_GENERIC_WEAPON_FALLBACK_RETIRED",
    "Wrong-identity replacement is intentionally forbidden.",
):
    require(fallback_h + fallback, needle, "truth-only bounded exact-identity weapon audit")
for forbidden in (
    "UMaterialInstanceDynamic::Create",
    "Component->SetMaterial(Slot",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "reason=material_gap_audited",
    "/Game/AK-47/Mesh/SM_AK-47.SM_AK-47",
    "R13 real SMG temporary MP5 fallback",
):
    forbid(fallback, forbidden, "weapon audit may not repaint slots or restore wrong-identity fallback")

# Museum layer observation must fail visibly instead of rebuilding/hiding the scene.
for needle in (
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "mutation=0",
    "primary_authoring_fix_required=1",
):
    require(layer_guard, needle, "Pass45 Museum validation-only ownership")

# Acceptance follows the current owner set. It accepts either legitimate initial BASE terminal marker through
# the PASS45_INITIAL_BASE_DEPLOYMENT_ prefix. Normal completion requires the material audit READY marker; hitting
# the hard bounded scan budget must be treated as failure rather than required success evidence.
for forbidden_marker in (
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS42_PRODUCTION_MATERIALS_RESTORED",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
):
    forbid(acceptance, forbidden_marker, "stale runtime marker must not be required")
for marker in (
    'findstr /C:"PASS45_INITIAL_BASE_DEPLOYMENT_" "%LOG%"',
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    'findstr /C:"PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP" "%LOG%"',
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"current runtime acceptance marker {marker}")

print("VISIBLE MUSEUM + WEAPON MATERIAL PASS37/PASS45 SOURCE CONTRACT PASS")
print("- destructive Museum visibility/rebuild and weapon palette owners stay physically retired")
print("- Museum BASE recovery is initial-character-only and acceptance permits either factual terminal result")
print("- Museum ownership validation cannot mutate the scene")
print("- authored material gaps remain fail-visible; wrong-identity weapon substitution stays retired")
print("- bounded weapon audit exhaustion is a runtime failure, while normal completion uses the material-audit READY marker")
print("STATUS: SOURCE CONTRACT ONLY; actual UE 5.8 visual/runtime acceptance remains required")
