#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
LOCAL_MODEL = PROJECT / "Saved" / "LocalModelInbox"
PRODUCTION_CACHE = PROJECT / "Saved" / "ProductionAssetImportCache"
PRODUCTION_REPORTS = PROJECT / "Saved" / "AutomationReports" / "ProductionModels"
OUT_DIR = PROJECT / "Saved" / "AssetStatus"
OUT_JSON = OUT_DIR / "LOCAL_ASSET_STATUS.json"
OUT_TEXT = OUT_DIR / "LOCAL_ASSET_STATUS.txt"
MANUAL_VISUAL_JSON = OUT_DIR / "MANUAL_VISUAL_ACCEPTANCE.json"
ZIP_CLEANUP_JSON = OUT_DIR / "ACCEPTED_ZIP_CLEANUP.json"

FILES = {
    "prepared_sources": LOCAL_MODEL / "prepared_sources.json",
    "runtime_bindings": LOCAL_MODEL / "runtime_bindings.json",
    "runtime_bindings_success": LOCAL_MODEL / "runtime_bindings_success.txt",
    "production_vehicles": PRODUCTION_CACHE / "production_import_success.txt",
    "production_weapons": PRODUCTION_CACHE / "production_weapon_import_result.txt",
    "local_inbox_runtime": PRODUCTION_REPORTS / "local_inbox_runtime_validation.txt",
    "local_world_runtime": PRODUCTION_REPORTS / "local_world_runtime_validation.txt",
    "gameplay_log": ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log",
    "material_log": ROOT / "Logs" / "PASS45_STRICT_MATERIAL_GATE.log",
    "runtime_evidence": ROOT / "Logs" / "PASS45_RUNTIME_ACCEPTANCE_EVIDENCE.txt",
}


def _read_text(path: Path) -> str:
    if not path.is_file():
        return ""
    return path.read_text(encoding="utf-8-sig", errors="replace")


def _read_json(path: Path) -> dict:
    if not path.is_file():
        return {}
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig", errors="replace"))
        return value if isinstance(value, dict) else {}
    except Exception:
        return {}


def _env_int(name: str) -> int | None:
    value = os.environ.get(name)
    if value is None or not value.strip():
        return None
    try:
        return int(value.strip())
    except ValueError:
        return None


def _kv_status(text: str) -> str:
    for line in text.splitlines():
        if line.strip().startswith("STATUS="):
            return line.partition("=")[2].strip().upper() or "UNKNOWN"
    return "MISSING" if not text else "UNKNOWN"


def _marker(text: str, marker: str) -> bool:
    return marker in text


def _category_counts(bindings: dict) -> dict[str, int]:
    counts: Counter[str] = Counter()
    for key in ("static_assets", "skeletal_assets"):
        for row in bindings.get(key, []) or []:
            if isinstance(row, dict):
                counts[str(row.get("category") or "UNCLASSIFIED")] += 1
    return dict(sorted(counts.items()))


def _prepared_counts(prepared: dict) -> dict[str, int | str]:
    return {
        "status": str(prepared.get("status") or "MISSING"),
        "archives": len(prepared.get("archives", []) or []),
        "loose_sources": len(prepared.get("loose_sources", []) or []),
        "ue_packages": len(prepared.get("ue_packages", []) or []),
        "raw_models": len(prepared.get("raw_models", []) or []),
        "hud_images": len(prepared.get("hud_images", []) or []),
        "conflicts": len(prepared.get("conflicts", []) or []),
    }


def _finalization_status(
    source_sha: str,
    import_stage: str,
    runtime_scope: str,
    runtime_stage: str,
    material_stage: str,
    evidence_stage: str,
) -> tuple[str, str, dict, dict]:
    manual = _read_json(MANUAL_VISUAL_JSON)
    cleanup = _read_json(ZIP_CLEANUP_JSON)

    automated_ready = all(
        (
            import_stage == "PASS",
            runtime_scope == "CURRENT_RUN_COMPLETED",
            runtime_stage == "PASS",
            material_stage == "PASS",
            evidence_stage == "PASS",
        )
    )

    visual_stage = "PENDING_MANUAL_OBSERVATION"
    if manual:
        manual_sha = str(manual.get("source_sha") or "")
        manual_status = str(manual.get("status") or "").upper()
        if manual_sha.lower() != source_sha.lower():
            visual_stage = "STALE_SOURCE"
        elif manual_status == "PASS" and automated_ready:
            visual_stage = "PASS"
        elif manual_status == "PASS":
            visual_stage = "INVALID_AUTOMATED_STATE"
        else:
            visual_stage = "FAIL_OR_INVALID"

    cleanup_stage = "PENDING_VISUAL_ACCEPTANCE"
    if visual_stage == "PASS":
        if not cleanup:
            cleanup_stage = "PENDING_CLEANUP"
        else:
            cleanup_sha = str(cleanup.get("source_sha") or "")
            cleanup_status = str(cleanup.get("status") or "").upper()
            if cleanup_sha.lower() != source_sha.lower():
                cleanup_stage = "STALE_SOURCE"
            elif cleanup_status == "PASS":
                cleanup_stage = "PASS"
            elif cleanup_status == "FAIL":
                cleanup_stage = "FAIL"
            else:
                cleanup_stage = "PENDING_CLEANUP"

    return visual_stage, cleanup_stage, manual, cleanup


def collect_snapshot(
    source_sha: str | None = None,
    runtime_result: int | None = None,
    import_result: int | None = None,
) -> tuple[Path, Path]:
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    texts = {name: _read_text(path) for name, path in FILES.items()}
    prepared = _read_json(FILES["prepared_sources"])
    bindings = _read_json(FILES["runtime_bindings"])

    missing_files = [str(path.relative_to(ROOT)) for path in FILES.values() if not path.is_file()]
    source_sha = source_sha or os.environ.get("PASS45_SOURCE_SHA", "unknown")
    if import_result is None:
        import_result = _env_int("PASS45_ASSET_IMPORT_RC")
    if runtime_result is None:
        runtime_result = _env_int("PASS45_RUNTIME_RC")

    vehicle_status = _kv_status(texts["production_vehicles"])
    weapon_status = _kv_status(texts["production_weapons"])
    binding_pass = _marker(texts["runtime_bindings_success"], "PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS")
    all_models_bound = bool(bindings.get("all_models_bound"))

    # Import sentinels/manifests can survive an older local run. Never mint a current import PASS
    # unless the caller supplies an explicit current import result code of zero.
    if import_result is None:
        import_stage = "PENDING_CURRENT_RUN"
    elif import_result != 0:
        import_stage = "FAIL"
    elif all((vehicle_status == "PASS", weapon_status == "PASS", binding_pass, all_models_bound)):
        import_stage = "PASS"
    else:
        import_stage = "PENDING_OR_GAP"

    # Runtime/material/evidence files can survive from an older local launch. An import-only snapshot
    # must never promote those stale files to PASS. Only a current explicit runtime_result authorizes
    # interpretation of runtime evidence for this snapshot.
    if runtime_result is None:
        runtime_scope = "IMPORT_ONLY"
        inbox_runtime = "PENDING_CURRENT_RUN"
        world_runtime = "PENDING_CURRENT_RUN"
        runtime_stage = "PENDING_CURRENT_RUN"
        material_stage = "PENDING_CURRENT_RUN"
        evidence_stage = "PENDING_CURRENT_RUN"
    elif runtime_result != 0:
        runtime_scope = "CURRENT_RUN_FAILED"
        inbox_runtime = (
            "PASS" if _marker(texts["local_inbox_runtime"], "PASS45_LOCAL_INBOX_RUNTIME=PASS")
            else "PENDING_OR_GAP"
        )
        world_runtime = (
            "PASS" if _marker(texts["local_world_runtime"], "PASS45_LOCAL_WORLD_RUNTIME=PASS")
            else "PENDING_OR_GAP"
        )
        runtime_stage = "FAIL"
        material_stage = "NOT_PASSED_CURRENT_RUN"
        evidence_stage = "NOT_PASSED_CURRENT_RUN"
    else:
        runtime_scope = "CURRENT_RUN_COMPLETED"
        inbox_runtime = (
            "PASS" if _marker(texts["local_inbox_runtime"], "PASS45_LOCAL_INBOX_RUNTIME=PASS")
            else "FAIL_OR_GAP"
        )
        world_runtime = (
            "PASS" if _marker(texts["local_world_runtime"], "PASS45_LOCAL_WORLD_RUNTIME=PASS")
            else "FAIL_OR_GAP"
        )
        runtime_stage = "PASS" if inbox_runtime == "PASS" and world_runtime == "PASS" else "FAIL_OR_GAP"

        material_stage = "PASS" if all(
            _marker(texts["material_log"], marker)
            for marker in (
                "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
                "PASS45_PRODUCTION_WEAPON_VISUALS_VALIDATED_READY",
            )
        ) else "FAIL_OR_GAP"

        evidence_stage = (
            "PASS" if _marker(texts["runtime_evidence"], "PASS45_RUNTIME_AUTOMATED_EVIDENCE=PASS")
            else "FAIL" if _marker(texts["runtime_evidence"], "PASS45_RUNTIME_AUTOMATED_EVIDENCE=FAIL")
            else "MISSING"
        )

    visual_stage, cleanup_stage, manual_record, cleanup_record = _finalization_status(
        source_sha,
        import_stage,
        runtime_scope,
        runtime_stage,
        material_stage,
        evidence_stage,
    )

    binding_summary = dict(bindings.get("summary") or {})
    binding_summary.setdefault("static_assets", len(bindings.get("static_assets", []) or []))
    binding_summary.setdefault("skeletal_assets", len(bindings.get("skeletal_assets", []) or []))
    binding_summary.setdefault("hud_textures", len(bindings.get("hud_textures", []) or []))
    binding_summary.setdefault("hud_widget_classes", len(bindings.get("hud_widget_classes", []) or []))
    binding_summary.setdefault("unbound_models", len(bindings.get("unbound_models", []) or []))

    unbound = [row for row in (bindings.get("unbound_models", []) or []) if isinstance(row, dict)]
    source_status = [row for row in (bindings.get("source_status", []) or []) if isinstance(row, dict)]
    source_status_counts = Counter(str(row.get("status") or "UNKNOWN") for row in source_status)

    report = {
        "schema": "oster-conflict-local-asset-status-v4",
        "source_sha": source_sha,
        "import_result_code": import_result,
        "runtime_result_code": runtime_result,
        "runtime_scope": runtime_scope,
        "stages": {
            "local_ue_import": import_stage,
            "live_runtime_hookup": runtime_stage,
            "strict_material_gate": material_stage,
            "automated_runtime_evidence": evidence_stage,
            "direct_visual_acceptance": visual_stage,
            "source_zip_cleanup": cleanup_stage,
        },
        "production": {
            "vehicles_status": vehicle_status,
            "weapons_status": weapon_status,
        },
        "runtime": {
            "local_inbox": inbox_runtime,
            "local_world": world_runtime,
        },
        "prepared": _prepared_counts(prepared),
        "bindings": {
            "all_models_bound": all_models_bound,
            "summary": binding_summary,
            "category_counts": _category_counts(bindings),
            "source_status_counts": dict(sorted(source_status_counts.items())),
            "unbound": unbound,
        },
        "finalization": {
            "manual_visual_record": manual_record,
            "zip_cleanup_record": cleanup_record,
        },
        "missing_files": missing_files,
        "files": {
            **{name: str(path.relative_to(ROOT)) for name, path in FILES.items()},
            "manual_visual_acceptance": str(MANUAL_VISUAL_JSON.relative_to(ROOT)),
            "accepted_zip_cleanup": str(ZIP_CLEANUP_JSON.relative_to(ROOT)),
        },
    }

    OUT_JSON.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    lines = [
        "OSTER CONFLICT — LOCAL ASSET STATUS",
        f"SOURCE_SHA={source_sha}",
        f"IMPORT_RESULT_CODE={import_result if import_result is not None else 'UNKNOWN'}",
        f"RUNTIME_RESULT_CODE={runtime_result if runtime_result is not None else 'UNKNOWN'}",
        f"RUNTIME_SCOPE={runtime_scope}",
        "",
        "STAGES",
        f"LOCAL_UE_IMPORT={import_stage}",
        f"LIVE_RUNTIME_HOOKUP={runtime_stage}",
        f"STRICT_MATERIAL_GATE={material_stage}",
        f"AUTOMATED_RUNTIME_EVIDENCE={evidence_stage}",
        f"DIRECT_VISUAL_ACCEPTANCE={visual_stage}",
        f"SOURCE_ZIP_CLEANUP={cleanup_stage}",
        "",
        "PRODUCTION",
        f"HMMWV_M2_BTR_STATUS={vehicle_status}",
        f"M249_REMINGTON870_STATUS={weapon_status}",
        "",
        "RUNTIME",
        f"LOCAL_INBOX={inbox_runtime}",
        f"LOCAL_WORLD={world_runtime}",
        "",
        "PREPARED",
    ]
    for key, value in report["prepared"].items():
        lines.append(f"{key.upper()}={value}")

    lines.extend(["", "BINDINGS"])
    for key, value in binding_summary.items():
        lines.append(f"{str(key).upper()}={value}")
    lines.append(f"ALL_MODELS_BOUND={1 if all_models_bound else 0}")

    lines.extend(["", "CATEGORY_COUNTS"])
    if report["bindings"]["category_counts"]:
        for key, value in report["bindings"]["category_counts"].items():
            lines.append(f"{key}={value}")
    else:
        lines.append("NONE=0")

    lines.extend(["", "SOURCE_STATUS_COUNTS"])
    if report["bindings"]["source_status_counts"]:
        for key, value in report["bindings"]["source_status_counts"].items():
            lines.append(f"{key}={value}")
    else:
        lines.append("NONE=0")

    lines.extend(["", "UNBOUND_OR_GAPS"])
    if unbound:
        for index, row in enumerate(unbound, start=1):
            reason = row.get("reason") or row.get("status") or "UNKNOWN"
            source = row.get("source") or row.get("asset") or "UNKNOWN"
            lines.append(f"{index}. {source} :: {reason}")
    else:
        lines.append("NONE")

    lines.extend(["", "FINALIZATION"])
    lines.append(f"MANUAL_VISUAL_RECORD={'PRESENT' if manual_record else 'MISSING'}")
    lines.append(f"ZIP_CLEANUP_RECORD={'PRESENT' if cleanup_record else 'MISSING'}")

    lines.extend(["", "MISSING_LOCAL_EVIDENCE"])
    if missing_files:
        lines.extend(f"- {path}" for path in missing_files)
    else:
        lines.append("NONE")

    OUT_TEXT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return OUT_JSON, OUT_TEXT


if __name__ == "__main__":
    json_path, text_path = collect_snapshot(
        import_result=_env_int("PASS45_ASSET_IMPORT_RC"),
        runtime_result=_env_int("PASS45_RUNTIME_RC"),
    )
    print(f"LOCAL ASSET STATUS JSON: {json_path}")
    print(f"LOCAL ASSET STATUS TEXT: {text_path}")
