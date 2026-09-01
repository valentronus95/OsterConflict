#!/usr/bin/env python3
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def read(rel: str) -> str:
    path = ROOT / rel
    if not path.is_file():
        raise SystemExit(f"PASS45 REMINGTON870 SOURCE INTAKE: FAIL\n[FAIL] missing file: {rel}")
    return path.read_text(encoding="utf-8", errors="replace")


errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


contract = read("_DOCS/PASS45_REMINGTON870_SOURCE_INTAKE.md")
variants = read("OsterConflict/Source/OsterConflict/Private/OCWeaponVariants.cpp")

for needle in (
    "/Game/Production/Weapons/Remington870/SM_Remington870.SM_Remington870",
    "PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=Remington870 primitive_visible=0 real_fallback_pending=1",
):
    req(needle in variants, f"canonical Remington fail-closed production contract missing: {needle}")

for needle in (
    "SOURCE CANDIDATE AUDITED / BINARY NOT ACQUIRED / UE 5.8 RUNTIME UNACCEPTED",
    "1b6c11ef58904fab992c6cdffaada309",
    "6db0ad4764d14eee8f063eea3600071b",
    "d33cef14f47b054845f9f447249dfd412a51163b",
    "3a4fb99a3dfb19a8dfdbb73a0ecafb6089723797",
    "SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json",
    "static-only geometry presented as completed skeletal/manual-action content -> reject",
    "runtime_ready",
    "ue58_import_pending",
    "item16_checked",
    "accepted_remington870_source=0 tracked_production_package=0 ue58_runtime_acceptance=0 item16_checked=0",
):
    req(needle in contract, f"Remington source-intake contract missing fail-closed evidence: {needle}")

production_asset = ROOT / "OsterConflict" / "Content" / "Production" / "Weapons" / "Remington870" / "SM_Remington870.uasset"
manifest_path = ROOT / "SOURCE_ASSETS" / "PASS45" / "Remington870" / "MANIFEST.json"

if production_asset.is_file():
    req(manifest_path.is_file(),
        "tracked Remington870 production asset exists without mandatory SOURCE_ASSETS/PASS45/Remington870/MANIFEST.json")

if manifest_path.is_file():
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"Remington870 manifest is invalid JSON: {exc}")
        manifest = {}

    req(manifest.get("schema") == 1, "Remington870 manifest schema must be 1")
    req(manifest.get("weapon") == "Remington870", "Remington870 manifest weapon identity mismatch")
    req(manifest.get("status") == "APPROVED_FOR_UE_IMPORT",
        "Remington870 manifest must be APPROVED_FOR_UE_IMPORT before production package introduction")

    for key in ("source_name", "source_url", "source_model_id", "license_id", "license_url", "attribution", "derivative_notes"):
        value = manifest.get(key)
        req(isinstance(value, str) and bool(value.strip()), f"Remington870 manifest missing non-empty {key}")

    req(manifest.get("public_repo_allowed") is True,
        "Remington870 source is not confirmed redistributable in this public repository")
    source_sha = manifest.get("source_sha256", "")
    req(isinstance(source_sha, str) and re.fullmatch(r"[0-9a-f]{64}", source_sha) is not None,
        "Remington870 manifest requires exact lowercase SHA-256 of acquired source bytes")
    req(manifest.get("rigged_or_articulated") is True,
        "Remington870 accepted source must prove rigged/articulated moving-part capability")
    req(manifest.get("animation_capable") is True,
        "Remington870 accepted source must prove animation capability")

    clips = manifest.get("intended_fp_clips")
    req(isinstance(clips, list), "Remington870 intended_fp_clips must be a list")
    if isinstance(clips, list):
        normalized = {str(item).strip().lower() for item in clips}
        req({"ironsight", "fire", "reload", "dryfire"}.issubset(normalized),
            "Remington870 manifest must preserve intended ironsight/fire/reload/dryfire FP coverage")

    # Intake approval is deliberately not runtime acceptance.
    req(manifest.get("runtime_ready") is False,
        "Remington870 intake manifest may not claim runtime_ready before current-head UE 5.8 acceptance")
    req(manifest.get("ue58_import_pending") is True,
        "Remington870 intake manifest must remain ue58_import_pending until factual UE 5.8 import acceptance")
    req(manifest.get("item16_checked") is False,
        "Remington870 intake alone may not close PASS45 item 16")

if errors:
    print("PASS45 REMINGTON870 SOURCE INTAKE: FAIL")
    for error in errors:
        print(f"[FAIL] {error}")
    raise SystemExit(1)

if production_asset.is_file():
    print("PASS45 REMINGTON870 SOURCE INTAKE: PASS manifest_guard=1 production_asset=1 runtime_acceptance=0")
else:
    print("PASS45 REMINGTON870 SOURCE INTAKE: PASS manifest_guard=1 production_asset=0 content_gap=1 runtime_acceptance=0")
