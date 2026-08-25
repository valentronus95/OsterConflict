#!/usr/bin/env python3
from __future__ import annotations

import os
import sys
from pathlib import Path

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


def main() -> int:
    gameplay_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_GAMEPLAY_LOG
    material_path = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_MATERIAL_LOG
    weapon_report_path = Path(sys.argv[3]) if len(sys.argv) > 3 else DEFAULT_WEAPON_REPORT

    gameplay = read_required(gameplay_path, "gameplay log")
    material = read_required(material_path, "strict material log")
    weapon_report = read_required(weapon_report_path, "weapon dependency report")
    errors: list[str] = []

    # Baseline deployment must occur once for the character and must not revive the vehicle-possession teleport bug.
    require(gameplay, "PASS7_MUSEUM_BASES_READY", errors, "Museum BASE readiness")
    require_any(
        gameplay,
        ("PASS45_INITIAL_BASE_DEPLOYMENT_VALIDATED_ONCE", "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERED_ONCE"),
        errors,
        "initial BASE deployment evidence",
    )
    forbid(gameplay, "PASS45_INITIAL_BASE_DEPLOYMENT_RECOVERY_FAIL", errors, "BASE recovery failure")

    # A strict acceptance run is incomplete until the tester actually enters and exits a vehicle.
    require(gameplay, "PASS45_VEHICLE_ENTER_TRANSFORM_READY", errors, "driver enter transform evidence")
    require(gameplay, "PASS45_VEHICLE_EXIT_TRANSFORM_READY", errors, "driver exit transform evidence")
    forbid(gameplay, "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL", errors, "driver enter transform failure")
    forbid(gameplay, "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL", errors, "driver exit transform failure")

    # The M2 vertical-aim regression cannot be accepted without an actual gunner session and exit.
    require(gameplay, "PASS45_M2_GUNNER_PITCH_CONTRACT_READY", errors, "M2 gunner pitch evidence")
    require(gameplay, "PASS45_GUNNER_EXIT_TRANSFORM_READY", errors, "gunner exit transform evidence")
    forbid(gameplay, "PASS45_GUNNER_EXIT_TRANSFORM_FAIL", errors, "gunner exit transform failure")

    # Production authored materials must pass the separate headless gate after the gameplay run.
    require(material, "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY", errors, "vehicle material readiness")
    require(material, "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY", errors, "production material bypass")
    require(material, "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY", errors, "weapon material readiness")
    for marker in (
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
        "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
        "PASS45_PRODUCTION_WEAPON_CONTENT_GAP",
    ):
        forbid(material, marker, errors, "material/content gap")

    # The weapon report itself must prove exact slot/material/runtime-material/texture dependency inspection.
    require(weapon_report, "PASS45 dependency contract:", errors, "weapon dependency report header")
    require(weapon_report, "SUMMARY=11/11 production weapon classes PASS", errors, "weapon dependency summary")
    require(weapon_report, "materialGaps=0", errors, "zero material gaps")
    require(weapon_report, "unexpectedOverrides=0", errors, "zero material overrides")
    require(weapon_report, "authoredMaterial=", errors, "authored material paths")
    require(weapon_report, "runtimeMaterial=", errors, "runtime material paths")
    require(weapon_report, "textureCount=", errors, "used texture counts")
    require(weapon_report, "textures=", errors, "used texture paths")
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
        "WEAPON_MATERIAL_TEXTURE_DEPENDENCIES=PASS\n",
        encoding="utf-8",
    )
    print("PASS45 RUNTIME EVIDENCE: PASS")
    print("- initial BASE deployment is character-only and no recovery failure was logged")
    print("- driver enter/exit and M2 gunner exit transforms were exercised without teleport failures")
    print("- authored HMMWV/M2/BTR and weapon material gates passed with exact dependency reporting")
    print("- visual acceptance remains PENDING until screenshots/direct observation satisfy the TZ")
    print("Evidence:", EVIDENCE_OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
