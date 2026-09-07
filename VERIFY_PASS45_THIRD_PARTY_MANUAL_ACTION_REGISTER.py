#!/usr/bin/env python3
"""Fail closed if PASS45 external manual-action donors lose mandatory third-party registration."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REGISTER = ROOT / "_DOCS" / "THIRD_PARTY_CODE_AND_ASSET_REGISTER.md"
PROVENANCE = ROOT / "PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md"
MANIFEST = ROOT / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio" / "MANIFEST.json"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


register = read(REGISTER)
provenance = read(PROVENANCE)
manifest_text = read(MANIFEST)

records = {
    "lever": {
        "id": "PASS45-3P-AUDIO-001",
        "source_url": "https://freesound.org/people/C-V/sounds/523401/",
        "transport_sha": "ae257485c6d55f4a4587f99389882cf74eae6779db807eaa0aa0f968e711f965",
        "derivative_sha": "417ba38e5e87b53ef3711784f821f1b3fc303ac8d4df19d9eda80fb776881542",
        "derivative_file": "lever_action_cc0_preview_donor.wav",
        "identity_limit": "not proof of exact Stein/Marlin/Model-1894 identity",
    },
    "bolt": {
        "id": "PASS45-3P-AUDIO-002",
        "source_url": "https://freesound.org/people/rammbostein/sounds/263459/",
        "transport_sha": "d9f4ee7633275f911f3521b5b7b319d634022944aafb9e7f51660a8a342d3040",
        "derivative_sha": "5e64820d532c11e91af3eedf96ab34a38df7b3dd066b0b1c9d67b3fe3f34c8a7",
        "derivative_file": "bolt_action_cc0_preview_donor.wav",
        "identity_limit": "not proof of exact M700 identity",
    },
}

for needle in (
    "## Actual-import records",
    "Creative Commons Zero (CC0) 1.0",
    "https://creativecommons.org/publicdomain/zero/1.0/",
    "acquisition workflow run 33501795799 SUCCESS",
    "runtime_ready=false / ue_import_pending=true",
    "The existing pump-cycle source fallback",
    "is project-owned repository content and is not an external import record",
):
    req(needle in register, f"third-party register contract missing: {needle}")

for key, record in records.items():
    for needle in (
        record["id"],
        record["source_url"],
        record["transport_sha"],
        record["derivative_sha"],
        record["derivative_file"],
        "STATUS: PILOT",
        record["identity_limit"],
    ):
        req(needle in register, f"{key} third-party import record missing: {needle}")
    req(record["source_url"] in provenance, f"{key} provenance/source URL drift against register")
    req(record["transport_sha"] in provenance, f"{key} provenance/transport SHA drift against register")
    req(record["derivative_sha"] in provenance, f"{key} provenance/derivative SHA drift against register")

if manifest_text:
    try:
        manifest = json.loads(manifest_text)
    except json.JSONDecodeError as exc:
        errors.append(f"manual-action manifest is invalid JSON: {exc}")
        manifest = {}

    req(manifest.get("runtime_ready") is False, "manual-action manifest falsely promotes runtime_ready")
    req(manifest.get("ue_import_pending") is True, "manual-action manifest lost ue_import_pending=true")
    req(manifest.get("item16_checked") is False, "manual-action manifest falsely checks item16")

    donors = manifest.get("donors", {})
    for key, record in records.items():
        donor = donors.get(key, {})
        req(donor.get("source_page") == record["source_url"], f"{key} manifest source URL drift")
        req(donor.get("transport_sha256") == record["transport_sha"], f"{key} manifest transport SHA drift")
        req(donor.get("derivative_sha256") == record["derivative_sha"], f"{key} manifest derivative SHA drift")
        req(donor.get("derivative_file") == record["derivative_file"], f"{key} manifest derivative filename drift")
        req(donor.get("source_license") == "CC0", f"{key} manifest license is no longer CC0")
        req(donor.get("runtime_ready") is False, f"{key} manifest falsely promotes runtime readiness")
        req(donor.get("ue_import_pending") is True, f"{key} manifest lost UE import pending truth")

if errors:
    print("PASS45 THIRD-PARTY MANUAL-ACTION REGISTER: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 THIRD-PARTY MANUAL-ACTION REGISTER: PASS")
print("- lever/bolt CC0 donor import records are pinned to source, transport and derivative identities")
print("- register, provenance and manifest agree on donor identity and fail-closed runtime state")
print("- item 16 remains open; this check is provenance governance, not UE 5.8 runtime acceptance")
