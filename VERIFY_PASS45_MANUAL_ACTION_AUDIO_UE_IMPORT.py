#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
UPROJECT = ROOT / "OsterConflict" / "OsterConflict.uproject"
IMPORTER = ROOT / "PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
PROFILE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWeaponAudioProfile.h"
MANIFEST = ROOT / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio" / "MANIFEST.json"
BOLT_WAV = ROOT / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio" / "bolt_action_cc0_preview_donor.wav"
LEVER_WAV = ROOT / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio" / "lever_action_cc0_preview_donor.wav"

EXPECTED = {
    "bolt": {
        "file": BOLT_WAV,
        "oid": "5e64820d532c11e91af3eedf96ab34a38df7b3dd066b0b1c9d67b3fe3f34c8a7",
        "size": 624078,
        "asset": "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
        "array": "RepositoryFallbackProfile->BoltCycle.Add(Bolt)",
    },
    "lever": {
        "file": LEVER_WAV,
        "oid": "417ba38e5e87b53ef3711784f821f1b3fc303ac8d4df19d9eda80fb776881542",
        "size": 92078,
        "asset": "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
        "array": "RepositoryFallbackProfile->LeverCycle.Add(Lever)",
    },
}

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def parse_lfs_pointer(path: Path) -> tuple[str, int] | None:
    text = read(path)
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    if len(lines) != 3 or lines[0] != "version https://git-lfs.github.com/spec/v1":
        errors.append(f"expected Git LFS pointer in source verification checkout: {path.relative_to(ROOT)}")
        return None
    if not lines[1].startswith("oid sha256:") or not lines[2].startswith("size "):
        errors.append(f"malformed Git LFS pointer: {path.relative_to(ROOT)}")
        return None
    try:
        return lines[1].removeprefix("oid sha256:"), int(lines[2].removeprefix("size "))
    except ValueError:
        errors.append(f"invalid Git LFS size: {path.relative_to(ROOT)}")
        return None


uproj_text = read(UPROJECT)
importer = read(IMPORTER)
cpp = read(CPP)
profile = read(PROFILE)
manifest_text = read(MANIFEST)
try:
    uproject = json.loads(uproj_text) if uproj_text else {}
except json.JSONDecodeError as exc:
    errors.append(f"invalid uproject JSON: {exc}")
    uproject = {}
try:
    manifest = json.loads(manifest_text) if manifest_text else {}
except json.JSONDecodeError as exc:
    errors.append(f"invalid manual-action manifest JSON: {exc}")
    manifest = {}

req(uproject.get("EngineAssociation") == "5.8", "manual-action importer contract requires UE EngineAssociation 5.8")
plugins = {item.get("Name"): item.get("Enabled") for item in uproject.get("Plugins", []) if isinstance(item, dict)}
req(plugins.get("PythonScriptPlugin") is True, "PythonScriptPlugin must remain enabled")
req(plugins.get("EditorScriptingUtilities") is True, "EditorScriptingUtilities must remain enabled")

req(manifest.get("runtime_ready") is False, "manifest must remain runtime_ready=false before local UE acceptance")
req(manifest.get("ue_import_pending") is True, "manifest must remain ue_import_pending=true until imported .uasset evidence exists")
req(manifest.get("item16_checked") is False, "item 16 must remain unchecked")

for donor_key, expected in EXPECTED.items():
    donor = manifest.get("donors", {}).get(donor_key, {})
    req(donor.get("derivative_sha256") == expected["oid"], f"manifest SHA drift for {donor_key}")
    req(donor.get("derivative_bytes") == expected["size"], f"manifest size drift for {donor_key}")
    req(donor.get("runtime_ready") is False, f"{donor_key} donor cannot claim runtime_ready")
    req(donor.get("ue_import_pending") is True, f"{donor_key} donor must remain ue_import_pending before factual UE import")
    pointer = parse_lfs_pointer(expected["file"])
    if pointer:
        req(pointer[0] == expected["oid"], f"LFS OID mismatch for {donor_key}")
        req(pointer[1] == expected["size"], f"LFS size mismatch for {donor_key}")
    req(expected["asset"] in cpp, f"runtime fallback asset path missing for {donor_key}")
    req(expected["array"] in cpp, f"runtime fallback route missing for {donor_key}")
    req(expected["oid"] in importer, f"importer SHA pin missing for {donor_key}")

for needle in (
    'DESTINATION_PATH = "/Game/PASS45/Audio/ManualAction"',
    '"bolt": "SW_PASS45_BoltAction_CC0_Donor"',
    '"lever": "SW_PASS45_LeverAction_CC0_Donor"',
    "unreal.AssetImportTask()",
    'task.set_editor_property("replace_existing", True)',
    'task.set_editor_property("save", True)',
    'asset.get_class().get_name() != "SoundWave"',
    "LFS_POINTER_PREFIX",
    "Git LFS pointer is present but payload is not hydrated",
    "validate_wav(source_path, expected_audio)",
    "runtime_acceptance=0 item16_checked=0",
):
    req(needle in importer, f"UE importer fail-closed contract missing: {needle}")

for forbidden in (
    "runtime_acceptance=1",
    "item16_checked=1",
    "RUNTIME ACCEPTED",
    "exact M700 donor",
    "exact Stein donor",
):
    req(forbidden not in importer, f"UE importer overclaims acceptance/identity: {forbidden}")

for needle in ("BoltCycle", "PumpCycle", "LeverCycle"):
    req(needle in profile, f"weapon audio profile lost manual-action route: {needle}")

if errors:
    print("PASS45 MANUAL ACTION AUDIO UE IMPORT CONTRACT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MANUAL ACTION AUDIO UE IMPORT CONTRACT: PASS")
print("- LFS donor OIDs/sizes match the canonical manifest")
print("- UE 5.8 importer is pinned, deterministic and rejects unhydrated/mutated donor payloads")
print("- BoltCycle/LeverCycle resolve only to repository-owned action-family donor SoundWave paths")
print("- no runtime acceptance or exact M700/Stein identity is claimed")
print("STATUS: SOURCE VERIFIED CONTRACT; actual UE import/audibility/timing and authored animations remain pending")
