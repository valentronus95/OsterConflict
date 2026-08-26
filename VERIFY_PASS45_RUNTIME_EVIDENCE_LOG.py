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
    require(gameplay, "PASS45_DAYLIGHT_EXPOSURE_CONTRACT_READY", errors, "physical daylight/exposure contract")
    require(gameplay, "PASS12_WORLD_GEOMETRY_STABLE", errors, "world geometry stability")
    require(gameplay, "PASS45_WORLD_MATERIAL_STABLE", errors, "semantic world material stability")
    forbid(gameplay, "PASS12_WORLD_GEOMETRY_STABILITY_FAIL", errors, "world geometry/material stability failure")

    # Gate D must prove three distinct authoritative landmark identities, not merely empty generic parcels.
    require(gameplay, "PASS45_LANDMARK_SEPARATION_VALIDATION_READY", errors, "generic landmark parcel separation")
    require(gameplay, "PASS45_LANDMARK_IDENTITY_VALIDATION_READY", errors, "Museum/Culture House identity separation")
    require(gameplay, "PASS45_SILPO_IDENTITY_VALIDATION_READY", errors, "authoritative Silpo identity")
    forbid(gameplay, "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL", errors, "generic landmark parcel separation failure")
    forbid(gameplay, "PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL", errors, "Museum/Culture House identity failure")
    forbid(gameplay, "PASS45_SILPO_IDENTITY_VALIDATION_FAIL", errors, "Silpo identity failure")

    # Gate E must prove the final gameplay world did not resurrect retired procedural residences/fences or the
    # rejected generic/tower/shack presentation through another actor, mesh, or late startup owner.
    require(gameplay, "PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY", errors, "reference-driven residential runtime")
    forbid(gameplay, "PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL", errors, "generic residential/tower runtime failure")

    # Gate C/H must be actual UE state, not launcher intent.
    require(gameplay, "PASS45_THERMAL_CAP_RUNTIME_READY", errors, "runtime 60 FPS recovery cap")
    require(gameplay, "PASS45_FULLSCREEN_RUNTIME_READY", errors, "runtime fullscreen viewport")
    forbid(gameplay, "PASS45_THERMAL_CAP_RUNTIME_FAIL", errors, "runtime FPS cap failure")
    forbid(gameplay, "PASS45_FULLSCREEN_RUNTIME_FAIL", errors, "runtime fullscreen failure")

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

    # Gate F is required-available truth, not an impossible all-exact production claim.
    require(gameplay, "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY", errors, "required available weapon rack")
    require(gameplay, "PASS36_WEAPON_MATERIAL_AUDIT_READY", errors, "rack authored material audit")
    require(gameplay, "PASS45_PRIMITIVE_WEAPON_RUNTIME_READY", errors, "zero visible BasicShape weapon rack")
    forbid(gameplay, "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL", errors, "required available weapon failure")
    forbid(gameplay, "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP", errors, "rack authored material gap")
    forbid(gameplay, "PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL", errors, "visible BasicShape weapon")
    forbid(gameplay, "PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL", errors, "launcher production visual gap")

    # Pass45 ordnance is fail-closed too. A strict acceptance run must factually throw at least one grenade in a
    # valid open-space case so source-only spawn semantics cannot masquerade as gameplay acceptance.
    require(gameplay, "PASS45_GRENADE_PRODUCTION_VISUAL_READY", errors, "grenade production visual")
    forbid(gameplay, "PASS45_GRENADE_PRODUCTION_VISUAL_FAIL", errors, "grenade production visual failure")
    require(gameplay, "PASS45_GRENADE_THROW_COMMIT_READY", errors, "transactional grenade throw")
    require(gameplay, "PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY", errors, "grenade throw presentation event bridge")
    forbid(gameplay, "PASS45_GRENADE_SAFE_SPAWN_REJECTED", errors, "grenade spawn clearance rejection during acceptance throw")
    forbid(gameplay, "PASS45_GRENADE_SPAWN_FAIL", errors, "grenade projectile spawn failure")
    require(gameplay, "PASS45_SMOKE_VFX_RUNTIME_READY", errors, "authored smoke visual runtime acceptance")
    forbid(gameplay, "PASS45_SMOKE_VFX_CONTENT_GAP", errors, "missing authored smoke VFX")
    forbid(gameplay, "PASS45_SMOKE_GAMEPLAY_VOLUME_FAIL", errors, "smoke gameplay volume spawn failure")

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

    # The report itself must prove slot/material/runtime-material/texture inspection.
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
        "LANDMARK_IDENTITY_AUTOMATED_CONTRACT=PASS\n"
        "SILPO_IDENTITY_AUTOMATED_CONTRACT=PASS\n"
        "REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_CONTRACT=PASS\n"
        "THERMAL_CAP_RUNTIME_CONTRACT=PASS\n"
        "FULLSCREEN_RUNTIME_CONTRACT=PASS\n"
        "BASE_INITIAL_ONLY=PASS\n"
        "DRIVER_ENTER_EXIT_TRANSFORM=PASS\n"
        "M2_GUNNER_PITCH_AND_EXIT=PASS\n"
        "PRODUCTION_VEHICLE_MATERIALS=PASS\n"
        "REQUIRED_AVAILABLE_WEAPON_MATERIALS=PASS\n"
        "PRIMITIVE_WEAPON_VISUALS=PASS\n"
        "GRENADE_PRODUCTION_VISUAL=PASS\n"
        "GRENADE_TRANSACTIONAL_THROW=PASS\n"
        "GRENADE_PRESENTATION_EVENT_BRIDGE=PASS\n"
        "SMOKE_AUTHORED_VFX=PASS\n"
        "WEAPON_MATERIAL_TEXTURE_DEPENDENCIES=PASS\n"
        "EXACT_WEAPON_CONTENT_GAPS=ALLOWED_IF_EXPLICIT_FALLBACK_PASSES\n",
        encoding="utf-8",
    )
    print("PASS45 RUNTIME EVIDENCE: PASS")
    print("- physical daylight started and semantic Ground/Roads/Sidewalks materials stayed stable through Pass12 samples")
    print("- Museum, R14.0 Silpo and Culture House authoritative owners remained distinct and on their canonical sites")
    print("- generic residential/private-fence instances and rejected village/tower/shack presentation were absent after startup")
    print("- UE reported the 60 FPS recovery cap and a live fullscreen viewport after gameplay possession")
    print("- initial BASE deployment is character-only and no recovery failure was logged")
    print("- driver enter/exit and M2 gunner exit transforms were exercised without teleport failures")
    print("- authored HMMWV/M2/BTR materials passed")
    print("- all required available rack visuals passed material/texture dependency checks with zero visible BasicShape weapon proxies")
    print("- launcher production visual did not fall back to rejected primitive geometry")
    print("- grenade production visual loaded; a factual throw committed inventory only after spawn and emitted presentation bridge evidence")
    print("- smoke had accepted authored runtime VFX rather than a content gap")
    print("- visual acceptance remains PENDING until screenshots/direct observation satisfy the TZ")
    print("Evidence:", EVIDENCE_OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())