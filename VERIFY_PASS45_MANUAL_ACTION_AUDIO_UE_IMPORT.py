#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
UPROJECT = ROOT / "OsterConflict" / "OsterConflict.uproject"
COMMANDLET_UPROJECT = ROOT / "OsterConflict" / "OsterConflictPass45Commandlet.uproject"
IMPORTER = ROOT / "PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py"
FRESH_LOADER = ROOT / "PASS45_MANUAL_ACTION_AUDIO_UE_FRESH_LOAD.py"
WRAPPER = ROOT / "OsterConflict" / "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd"
START_HERE = ROOT / "START_HERE.cmd"
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


def parse_project(path: Path, label: str) -> dict:
    text = read(path)
    if not text:
        return {}
    try:
        return json.loads(text)
    except json.JSONDecodeError as exc:
        errors.append(f"invalid {label} JSON: {exc}")
        return {}


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


uproject = parse_project(UPROJECT, "runtime uproject")
commandlet_uproject = parse_project(COMMANDLET_UPROJECT, "commandlet uproject")
importer = read(IMPORTER)
fresh_loader = read(FRESH_LOADER)
wrapper = read(WRAPPER)
start_here = read(START_HERE)
cpp = read(CPP)
profile = read(PROFILE)
manifest_text = read(MANIFEST)
try:
    manifest = json.loads(manifest_text) if manifest_text else {}
except json.JSONDecodeError as exc:
    errors.append(f"invalid manual-action manifest JSON: {exc}")
    manifest = {}

for label, project in (("runtime", uproject), ("commandlet", commandlet_uproject)):
    req(project.get("EngineAssociation") == "5.8", f"{label} manual-action importer contract requires UE EngineAssociation 5.8")
    plugins = {item.get("Name"): item.get("Enabled") for item in project.get("Plugins", []) if isinstance(item, dict)}
    req(plugins.get("PythonScriptPlugin") is True, f"{label} PythonScriptPlugin must remain enabled")
    req(plugins.get("EditorScriptingUtilities") is True, f"{label} EditorScriptingUtilities must remain enabled")

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
    req(expected["asset"] in fresh_loader, f"fresh-load exact object path missing for {donor_key}")

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

for needle in (
    "unreal.EditorAssetLibrary.does_asset_exist",
    "unreal.EditorAssetLibrary.load_asset",
    'asset.get_class().get_name() != "SoundWave"',
    "asset.get_path_name() != object_path",
    "runtime_acceptance=0 item16_checked=0",
):
    req(needle in fresh_loader, f"fresh-load verifier contract missing: {needle}")

for needle in (
    "OsterConflictPass45Commandlet.uproject",
    "PASS45_MANUAL_ACTION_AUDIO_UE_IMPORT.py",
    "PASS45_MANUAL_ACTION_AUDIO_UE_FRESH_LOAD.py",
    "SW_PASS45_BoltAction_CC0_Donor.uasset",
    "SW_PASS45_LeverAction_CC0_Donor.uasset",
    "-run=pythonscript",
    "-nullrhi",
    "runtime_acceptance=0 item16_checked=0",
):
    req(needle in wrapper, f"manual-action UE 5.8 wrapper contract missing: {needle}")

req('call "%~dp0OsterConflict\\PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd"' in start_here,
    "START_HERE full runtime route no longer invokes manual-action audio import/fresh-load")
req("Пункт 1: тільки incremental C++ build + запуск гри. Без reimport HMMWV/M2/BTR/Stein/manual-action audio." in start_here,
    "START_HERE must keep manual-action import out of Quick Normal route")

for text, label in ((importer, "importer"), (fresh_loader, "fresh loader"), (wrapper, "wrapper")):
    for forbidden in (
        "runtime_acceptance=1",
        "item16_checked=1",
        "RUNTIME ACCEPTED",
        "exact M700 donor",
        "exact Stein donor",
    ):
        req(forbidden not in text, f"{label} overclaims acceptance/identity: {forbidden}")

for needle in ("BoltCycle", "PumpCycle", "LeverCycle"):
    req(needle in profile, f"weapon audio profile lost manual-action route: {needle}")

if errors:
    print("PASS45 MANUAL ACTION AUDIO UE IMPORT CONTRACT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MANUAL ACTION AUDIO UE IMPORT CONTRACT: PASS")
print("- LFS donor OIDs/sizes match the canonical manifest")
print("- runtime and isolated commandlet UE 5.8 projects expose the required Python/editor scripting plugins")
print("- strict full-runtime route imports and independently fresh-loads both donor SoundWaves; Quick Normal remains import-free")
print("- BoltCycle/LeverCycle resolve only to repository-owned action-family donor SoundWave paths")
print("- no runtime acceptance or exact M700/Stein identity is claimed")
print("STATUS: SOURCE VERIFIED CONTRACT; actual UE import/audibility/timing and authored animations remain pending")
