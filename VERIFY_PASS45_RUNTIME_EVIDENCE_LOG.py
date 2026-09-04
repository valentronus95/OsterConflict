#!/usr/bin/env python3
from __future__ import annotations

import os
import sys
from pathlib import Path

import COLLECT_LOCAL_ASSET_STATUS as asset_status

ROOT = Path(__file__).resolve().parent
DEFAULT_GAMEPLAY_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"
DEFAULT_MATERIAL_LOG = ROOT / "Logs" / "PASS45_STRICT_MATERIAL_GATE.log"
DEFAULT_WEAPON_REPORT = ROOT / "OsterConflict" / "Saved" / "AutomationReports" / "ProductionModels" / "weapon_runtime_validation.txt"
EVIDENCE_OUT = ROOT / "Logs" / "PASS45_RUNTIME_ACCEPTANCE_EVIDENCE.txt"


def read_required(path: Path, label: str) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 RUNTIME EVIDENCE: FAIL: missing {label}: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, marker: str, errors: list[str], label: str) -> None:
    if marker not in text:
        errors.append(f"missing {label}: {marker}")


def require_any(text: str, markers: tuple[str, ...], errors: list[str], label: str) -> None:
    if not any(marker in text for marker in markers):
        errors.append(f"missing {label}: one of {', '.join(markers)}")


def forbid(text: str, marker: str, errors: list[str], label: str) -> None:
    if marker in text:
        errors.append(f"forbidden {label}: {marker}")


def write_asset_snapshot(source_sha: str, runtime_result: int) -> None:
    try:
        json_path, text_path = asset_status.collect_snapshot(source_sha=source_sha, runtime_result=runtime_result)
        print("Asset status snapshot:", text_path)
        print("Asset status JSON:", json_path)
    except Exception as exc:
        print(f"[WARN] LOCAL_ASSET_STATUS snapshot failed: {type(exc).__name__}: {exc}")


def main() -> int:
    gameplay_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_GAMEPLAY_LOG
    material_path = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_MATERIAL_LOG
    weapon_report_path = Path(sys.argv[3]) if len(sys.argv) > 3 else DEFAULT_WEAPON_REPORT

    gameplay = read_required(gameplay_path, "gameplay log")
    material = read_required(material_path, "strict material log")
    weapon_report = read_required(weapon_report_path, "weapon dependency report")
    errors: list[str] = []

    # One canonical acceptance verifier replaces the old pile of per-pass BAT/CMD log scanners.
    required_runtime = (
        "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
        "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
        "PASS14_HOST_TRAVEL_BEGIN",
        "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
        "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
        "PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY",
        "PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY",
        "PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY",
        "PASS44_COMPACT_PLAYABLE_AREA_READY",
        "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
        "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
        "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
        "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
        "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
        "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
        "PASS45_MUSEUM_LAYER_VALIDATION_READY",
        "PASS42_BASE_RACK_GROUNDED_READY",
        "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
        "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
        "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
        "PASS45_BTR4_PROPORTIONAL_VISUAL_READY",
        "PASS45_M2_MOUNT_ALIGNMENT_READY",
        "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
        "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
        "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
        "PASS45_GUNNER_EXIT_TRANSFORM_READY",
        "PASS31_GAMEPLAY_INPUT_READY",
        "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
        "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
        "PASS36_WEAPON_MATERIAL_AUDIT_READY",
        "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
        "PASS40_UI_STABILIZER_BUDGET_READY",
        "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
        "PASS14_PERF_SAMPLE",
        "PASS14_PERF_30FPS_READY",
        "PASS7_MUSEUM_BASES_READY",
    )
    for marker in required_runtime:
        require(gameplay, marker, errors, "runtime evidence")

    require_any(
        gameplay,
        ("PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE"),
        errors,
        "initial BASE deployment evidence",
    )
    if "PASS31_GAMEPLAY_INPUT_READY" in gameplay and not any(
        "PASS31_GAMEPLAY_INPUT_READY" in line and "moveIgnored=0" in line and "lookIgnored=0" in line
        for line in gameplay.splitlines()
    ):
        errors.append("gameplay input stayed ignored after possession")

    forbidden_runtime = (
        "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL",
        "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
        "PASS44_COMPACT_PLAYABLE_AREA_FAIL",
        "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
        "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL",
        "PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL",
        "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
        "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
        "PASS42_BASE_RACK_GROUNDING_INCOMPLETE",
        "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
        "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
        "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
        "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
        "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
        "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
        "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
        "PASS10_FOLIAGE_RUNTIME_FAIL",
        "PASS14_PERF_BELOW_TARGET",
    )
    for marker in forbidden_runtime:
        forbid(gameplay, marker, errors, "runtime failure")

    # Authored production materials/dependencies remain a separate headless gate.
    for marker in (
        "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
        "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
        "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY",
    ):
        require(material, marker, errors, "material readiness")
    for marker in (
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
        "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
        "PASS45_PRODUCTION_WEAPON_CONTENT_GAP",
    ):
        forbid(material, marker, errors, "material/content gap")

    # Exact production weapon dependency report.
    for marker in (
        "PASS45 dependency contract:",
        "SUMMARY=11/11 production weapon classes PASS",
        "materialGaps=0",
        "unexpectedOverrides=0",
        "authoredMaterial=",
        "runtimeMaterial=",
        "textureCount=",
        "textures=",
    ):
        require(weapon_report, marker, errors, "weapon dependency report")
    forbid(weapon_report, "placeholder=1", errors, "placeholder weapon material")
    forbid(weapon_report, "RESULT=FAIL", errors, "weapon runtime result")

    EVIDENCE_OUT.parent.mkdir(parents=True, exist_ok=True)
    source_sha = os.environ.get("PASS45_SOURCE_SHA", "unknown")
    if errors:
        EVIDENCE_OUT.write_text(
            "PASS45_RUNTIME_AUTOMATED_EVIDENCE=FAIL\n"
            "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION\n"
            f"SOURCE_SHA={source_sha}\n"
            + "\n".join(f"FAIL={error}" for error in errors)
            + "\n",
            encoding="utf-8",
        )
        write_asset_snapshot(source_sha, 1)
        print("PASS45 RUNTIME EVIDENCE: FAIL")
        for error in errors:
            print("[FAIL]", error)
        print("Evidence:", EVIDENCE_OUT)
        return 1

    EVIDENCE_OUT.write_text(
        "PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS\n"
        "VISUAL_ACCEPTANCE=PENDING_MANUAL_OBSERVATION\n"
        f"SOURCE_SHA={source_sha}\n"
        "BASE_INITIAL_ONLY=PASS\n"
        "DRIVER_ENTER_EXIT_TRANSFORM=PASS\n"
        "M2_GUNNER_PITCH_AND_EXIT=PASS\n"
        "PRODUCTION_VEHICLE_MATERIALS=PASS\n"
        "PRODUCTION_WEAPON_MATERIALS=PASS\n"
        "WEAPON_MATERIAL_TEXTURE_DEPENDENCIES=PASS\n"
        "PERFORMANCE_30FPS_GATE=PASS\n",
        encoding="utf-8",
    )
    write_asset_snapshot(source_sha, 0)
    print("PASS45 RUNTIME EVIDENCE: PASS")
    print("- all canonical runtime, material, interaction and 30 FPS gates passed")
    print("- one LOCAL_ASSET_STATUS snapshot now consolidates asset counts, GAPs and missing evidence")
    print("- visual acceptance remains PENDING until direct observation satisfies the TZ")
    print("Evidence:", EVIDENCE_OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
