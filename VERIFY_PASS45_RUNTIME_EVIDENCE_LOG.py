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

    # Pass45 P0 black-world acceptance is part of the strict main route, not an optional side launcher.
    # The daylight/exposure source contract must instantiate and the semantic Ground/Roads/Sidewalks MID
    # contract must remain intact through the 12s/16s/20s world-stability samples.
    require(gameplay, "PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY", errors, "physical daylight/exposure contract")
    require(gameplay, "PASS12_WORLD_GEOMETRY_STABLE", errors, "world geometry stability")
    require(gameplay, "PASS45_WORLD_MATERIAL_STABLE", errors, "semantic world material stability")
    forbid(gameplay, "PASS12_WORLD_GEOMETRY_STABILITY_FAIL", errors, "world geometry/material stability failure")

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

    # Gate F is required-available truth, not an impossible all-exact production claim. Every one of the 11
    # rack classes needs either exact production or an explicit real fallback and every visible path must pass
    # authored material audit. Exact payload gaps remain CONTENT GAP and are intentionally allowed.
    require(gameplay, "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY", errors, "required available weapon rack")
    require(gameplay, "PASS36_WEAPON_MATERIAL_AUDIT_READY", errors, "rack authored material audit")
    forbid(gameplay, "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL", errors, "required available weapon failure")
    forbid(gameplay, "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP", errors, "rack authored material gap")

    # Production vehicle authored materials remain a hard Gate G requirement.
    require(material, "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY", errors, "vehicle material readiness")
    require(material, "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY", errors, "production material bypass")
    for marker in (
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
        "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
        "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
    ):
        forbid(material, marker, errors, "vehicle material/content gap")

    # The separate headless weapon gate must validate every required available visual and dependency chain.
    require(material, "PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY", errors, "required available weapon material readiness")
    forbid(material, "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL", errors, "headless required available weapon failure")

    # The report itself must prove slot/material/runtime-material/texture inspection. CONTENT_GAP_FALLBACK_PASS is
    # permitted; RESULT=FAIL, a placeholder slot, or a missing/placeholder texture dependency is not.
    require(weapon_report, "PASS45 dependency contract:", errors, "weapon dependency report header")
    require(weapon_report, "required available weapon visuals PASS", errors, "required available weapon dependency summary")
    require(weapon_report, "materialGaps=0", errors, "zero material gaps")
    require(weapon_report, "textureGaps=0", errors, "zero texture dependency gaps")
    require(weapon_report, "unexpectedOverrides=0", errors, "zero material overrides")
    require(weapon_report, "authoredMaterial=", errors, "authored material paths")
    require(weapon_report, "runtimeMaterial=", errors, "runtime material paths")
    require(weapon_report, "textureCount=", errors, "used texture counts")
    require(weapon_report, "textureDependency=PASS", errors, "texture dependency readiness")
    require(weapon_report, "textures=", errors, "used texture paths")
    forbid(weapon_report, "placeholder=1", errors, "placeholder weapon material")
    forbid(weapon_report, "textureDependency=GAP", errors, "weapon texture dependency gap")
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
        "BLACK_WORLD_AUTOMATED_CONTRACT=PASS\n"
        "BASE_INITIAL_ONLY=PASS\n"
        "DRIVER_ENTER_EXIT_TRANSFORM=PASS\n"
        "M2_GUNNER_PITCH_AND_EXIT=PASS\n"
        "PRODUCTION_VEHICLE_MATERIALS=PASS\n"
        "REQUIRED_AVAILABLE_WEAPON_MATERIALS=PASS\n"
        "WEAPON_MATERIAL_TEXTURE_DEPENDENCIES=PASS\n"
        "EXACT_WEAPON_CONTENT_GAPS=ALLOWED_IF_EXPLICIT_FALLBACK_PASSES\n",
        encoding="utf-8",
    )
    print("PASS45 RUNTIME EVIDENCE: PASS")
    print("- physical daylight started and semantic Ground/Roads/Sidewalks materials stayed stable through Pass12 samples")
    print("- initial BASE deployment is character-only and no recovery failure was logged")
    print("- driver enter/exit and M2 gunner exit transforms were exercised without teleport failures")
    print("- authored HMMWV/M2/BTR materials passed")
    print("- all required available rack visuals passed material/texture dependency checks; exact payload gaps remain CONTENT GAP")
    print("- visual acceptance remains PENDING until screenshots/direct observation satisfy the TZ")
    print("Evidence:", EVIDENCE_OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
