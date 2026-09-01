#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROVENANCE = ROOT / "PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md"
INTAKE = ROOT / "PASS45_MANUAL_ACTION_AUDIO_INTAKE.py"
WORKFLOW = ROOT / ".github" / "workflows" / "pass45-manual-action-audio-intake.yml"
MANIFEST = ROOT / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio" / "MANIFEST.json"
GITATTRIBUTES = ROOT / ".gitattributes"
AUDIO_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWeaponAudioComponent.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


provenance = read(PROVENANCE)
intake = read(INTAKE)
workflow = read(WORKFLOW)
gitattributes = read(GITATTRIBUTES)
audio_cpp = read(AUDIO_CPP)
tz = read(TZ)
manifest_text = read(MANIFEST)

for needle in (
    "https://freesound.org/people/C-V/sounds/523401/",
    "Lever action cocking.wav",
    "Creative Commons 0 (CC0)",
    "0.958 s",
    "**not** evidence of an exact Stein/Marlin/Model-1894 recording",
    "https://freesound.org/people/rammbostein/sounds/263459/",
    "Mosin Nagant Bolt.wav",
    "6.500 s",
    "**not** an M700 recording",
    "https://opengameart.org/content/the-free-firearm-sound-library",
    "beb2f4041f3d6740fa0aeaf0e71159bd65a78c1b",
    "Folder names such as Model 1894 / Savage 10 / Mosin Nagant are **not sufficient evidence**",
):
    req(needle in provenance, f"manual-action audio provenance contract missing: {needle}")

pins = {
    "lever": {
        "transport_url": "https://cdn.freesound.org/previews/523/523401_8956746-lq.mp3",
        "transport_sha": "ae257485c6d55f4a4587f99389882cf74eae6779db807eaa0aa0f968e711f965",
        "derivative_file": "lever_action_cc0_preview_donor.wav",
        "derivative_sha": "417ba38e5e87b53ef3711784f821f1b3fc303ac8d4df19d9eda80fb776881542",
        "derivative_bytes": 92078,
    },
    "bolt": {
        "transport_url": "https://cdn.freesound.org/previews/263/263459_4174990-lq.mp3",
        "transport_sha": "d9f4ee7633275f911f3521b5b7b319d634022944aafb9e7f51660a8a342d3040",
        "derivative_file": "bolt_action_cc0_preview_donor.wav",
        "derivative_sha": "5e64820d532c11e91af3eedf96ab34a38df7b3dd066b0b1c9d67b3fe3f34c8a7",
        "derivative_bytes": 624078,
    },
}

for key, pin in pins.items():
    for needle in (pin["transport_url"], pin["transport_sha"]):
        req(str(needle) in provenance, f"manual-action provenance lost {key} transport identity: {needle}")
        req(str(needle) in intake, f"manual-action intake lost {key} transport identity: {needle}")
    req(pin["derivative_sha"] in provenance,
        f"manual-action provenance lost {key} derivative identity: {pin['derivative_sha']}")

for needle in (
    'return expected_url, "freesound_public_preview_pinned"',
    'write mode forbidden without pinned transport URL',
    'transport URL drift',
    'transport SHA256 changed',
    'current advertised candidates=',
):
    req(needle in intake, f"manual-action intake transport pin/guard missing: {needle}")
for stale in (
    "523401_9-lq.mp3",
    "7785b4db5b512cec45da227097789dab4510aafec1f7e5d9f260669f54ed75ab",
    "263459_3988807-lq.mp3",
    "635a4fd88454a032a476445237befb536ab532c1bdf573249653011bff4dde9e",
):
    req(stale not in intake, f"stale 404 manual-action transport is still pinned in intake: {stale}")
for forbidden in (
    'pinned public preview is no longer advertised by source page',
    'expected_canonical not in advertised',
    'return source_urls[0], "freesound_public_preview"',
):
    req(forbidden not in intake,
        f"manual-action intake still depends on mutable preview-page transport selection: {forbidden}")

for needle in (
    "Audit currently advertised preview candidates",
    "PASS45_AUDIO_CURRENT_CANDIDATE",
    "Stage and verify Git LFS pointers",
    "PASS45_AUDIO_LFS_POINTER_OK",
):
    req(needle in workflow, f"manual-action intake workflow guard missing: {needle}")

req("*.wav filter=lfs" in gitattributes,
    "repository no longer protects WAV payloads with Git LFS")
req("Do not bypass LFS" in provenance,
    "manual-action provenance no longer forbids bypassing the repository WAV/LFS policy")

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
    for key, pin in pins.items():
        donor = donors.get(key, {})
        req(donor.get("transport_url") == pin["transport_url"], f"{key} manifest transport URL drift")
        req(donor.get("transport_sha256") == pin["transport_sha"], f"{key} manifest transport SHA drift")
        req(donor.get("derivative_file") == pin["derivative_file"], f"{key} manifest derivative filename drift")
        req(donor.get("derivative_sha256") == pin["derivative_sha"], f"{key} manifest derivative SHA drift")
        req(donor.get("derivative_bytes") == pin["derivative_bytes"], f"{key} manifest derivative size drift")
        req(donor.get("runtime_ready") is False, f"{key} manifest falsely promotes runtime readiness")
        req(donor.get("ue_import_pending") is True, f"{key} manifest lost UE import pending truth")

# Source routing is allowed before UE import only when it remains fail-closed: LoadSound(null) must leave the
# fallback arrays empty. The exact repository-owned donor object paths are therefore source-prewired, while the
# missing .uasset files remain factual CONTENT GAP until the strict UE 5.8 import/fresh-load route succeeds.
for needle in (
    "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
    "/Game/R13/Audio/shotguncock.shotguncock",
    "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
    "if (USoundBase* Bolt = LoadSound",
    "RepositoryFallbackProfile->BoltCycle.Add(Bolt);",
    "if (USoundBase* Pump = LoadSound",
    "RepositoryFallbackProfile->PumpCycle.Add(Pump);",
    "if (USoundBase* Lever = LoadSound",
    "RepositoryFallbackProfile->LeverCycle.Add(Lever);",
    "PASS45_WEAPON_AUDIO_CONTENT_GAP",
):
    req(needle in audio_cpp, f"manual-action fail-closed source routing missing: {needle}")

for needle in (
    "Current repository-owned BoltCycle runtime asset: **CONTENT GAP**",
    "Current repository-owned LeverCycle runtime asset: **CONTENT GAP**",
    "No URL, title, tag, filename or folder name by itself counts as runtime content.",
    "Item 16 remains unchecked",
    "PR #94 remains OPEN / UNMERGED",
    "runtime_ready=false",
    "ue_import_pending=true",
):
    req(needle in provenance, f"manual-action audio fail-closed rule missing: {needle}")

req("RUNTIME REJECTED 2026-08-31" in tz,
    "canonical Pass45 TZ lost current factual runtime rejection")
req("Replace procedural manual-action fallback cues" in tz,
    "canonical item 16 text is no longer discoverable")
req("real bolt/pump/lever sound content" in tz,
    "canonical item 16 lost the real manual-action audio requirement")

if errors:
    print("PASS45 MANUAL-ACTION AUDIO PROVENANCE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 MANUAL-ACTION AUDIO PROVENANCE: PASS")
print("- current CC0 lever/bolt transports and repository-owned LFS derivative identities are pinned")
print("- current advertised preview bytes are audit-only and cannot auto-replace pinned bytes")
print("- manifest remains runtime_ready=0 / ue_import_pending=1 / item16_checked=0")
print("- bolt/pump/lever source routes are guarded by LoadSound; absent donor .uasset files remain fail-visible content gaps")
print("- item 16 stays open until UE SoundWave import/fresh-load, authored moving-part animation and UE 5.8 acceptance")
print("STATUS: SOURCE PAYLOAD + FAIL-CLOSED ROUTING VERIFIED; UE IMPORT / RUNTIME ACCEPTANCE PENDING")
