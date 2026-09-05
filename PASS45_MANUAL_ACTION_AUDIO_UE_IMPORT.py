#!/usr/bin/env python3
"""UE 5.8 editor-side importer for the repository-owned PASS45 manual-action audio donors.

This script deliberately does not promote runtime acceptance. It only turns the already
provenance-pinned, Git-LFS-owned WAV derivatives into deterministic SoundWave asset paths
that the weapon-audio fallback can reference.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import wave

try:
    import unreal
except ImportError as exc:  # pragma: no cover - must run inside Unreal Editor Python
    raise SystemExit("PASS45 MANUAL ACTION AUDIO UE IMPORT: run inside Unreal Editor 5.8 Python") from exc

DESTINATION_PATH = "/Game/PASS45/Audio/ManualAction"
ASSET_NAMES = {
    "bolt": "SW_PASS45_BoltAction_CC0_Donor",
    "lever": "SW_PASS45_LeverAction_CC0_Donor",
}
EXPECTED_SHA256 = {
    "bolt": "5e64820d532c11e91af3eedf96ab34a38df7b3dd066b0b1c9d67b3fe3f34c8a7",
    "lever": "417ba38e5e87b53ef3711784f821f1b3fc303ac8d4df19d9eda80fb776881542",
}
EXPECTED_WAV = {
    "bolt": {"channels": 1, "sample_rate_hz": 48000, "sample_width_bytes": 2, "frames": 312000},
    "lever": {"channels": 1, "sample_rate_hz": 48000, "sample_width_bytes": 2, "frames": 46000},
}
LFS_POINTER_PREFIX = b"version https://git-lfs.github.com/spec/v1\n"


def fail(message: str) -> None:
    raise RuntimeError(f"PASS45 MANUAL ACTION AUDIO UE IMPORT: {message}")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_wav(path: Path, expected: dict[str, int]) -> None:
    with wave.open(str(path), "rb") as wav_file:
        actual = {
            "channels": wav_file.getnchannels(),
            "sample_rate_hz": wav_file.getframerate(),
            "sample_width_bytes": wav_file.getsampwidth(),
            "frames": wav_file.getnframes(),
        }
    if actual != expected:
        fail(f"unexpected WAV contract for {path.name}: expected={expected} actual={actual}")


def import_soundwave(source: Path, asset_name: str) -> str:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", DESTINATION_PATH)
    task.set_editor_property("destination_name", asset_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{DESTINATION_PATH}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        fail(f"SoundWave import did not create {asset_path}")

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        fail(f"cannot load imported asset {asset_path}")
    if asset.get_class().get_name() != "SoundWave":
        fail(f"imported asset is not SoundWave: {asset_path} class={asset.get_class().get_name()}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        fail(f"cannot save imported SoundWave {asset_path}")

    return f"{asset_path}.{asset_name}"


def main() -> None:
    project_dir = Path(unreal.Paths.project_dir()).resolve()
    repo_root = project_dir.parent
    source_dir = repo_root / "SOURCE_ASSETS" / "PASS45" / "ManualActionAudio"
    manifest_path = source_dir / "MANIFEST.json"
    if not manifest_path.is_file():
        fail(f"missing manifest: {manifest_path}")

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("runtime_ready") is not False:
        fail("manifest must remain runtime_ready=false before factual runtime acceptance")
    if manifest.get("ue_import_pending") is not True:
        fail("manifest no longer records the pre-acceptance ue_import_pending=true state")
    if manifest.get("item16_checked") is not False:
        fail("item 16 must remain unchecked before authored animation + runtime acceptance")

    imported_paths: dict[str, str] = {}
    for donor_key in ("bolt", "lever"):
        donor = manifest.get("donors", {}).get(donor_key)
        if not isinstance(donor, dict):
            fail(f"missing donor manifest entry: {donor_key}")
        if donor.get("derivative_sha256") != EXPECTED_SHA256[donor_key]:
            fail(f"manifest SHA drift for {donor_key}")
        if donor.get("runtime_ready") is not False or donor.get("ue_import_pending") is not True:
            fail(f"donor {donor_key} must remain pre-runtime-acceptance in the source manifest")

        source_path = source_dir / str(donor.get("derivative_file", ""))
        if not source_path.is_file():
            fail(f"missing hydrated Git LFS payload: {source_path}")
        prefix = source_path.read_bytes()[: len(LFS_POINTER_PREFIX)]
        if prefix == LFS_POINTER_PREFIX:
            fail(f"Git LFS pointer is present but payload is not hydrated: {source_path.name}")
        if source_path.stat().st_size != int(donor.get("derivative_bytes", -1)):
            fail(f"payload size mismatch for {source_path.name}")
        if sha256(source_path) != EXPECTED_SHA256[donor_key]:
            fail(f"payload SHA-256 mismatch for {source_path.name}")

        expected_audio = EXPECTED_WAV[donor_key]
        manifest_audio = donor.get("derivative_audio", {})
        for key, value in expected_audio.items():
            if int(manifest_audio.get(key, -1)) != value:
                fail(f"manifest WAV field drift for {donor_key}.{key}")
        validate_wav(source_path, expected_audio)
        imported_paths[donor_key] = import_soundwave(source_path, ASSET_NAMES[donor_key])

    print("PASS45 MANUAL ACTION AUDIO UE IMPORT: IMPORTED")
    print(f"- BoltCycle donor SoundWave: {imported_paths['bolt']}")
    print(f"- LeverCycle donor SoundWave: {imported_paths['lever']}")
    print("- identity scope: action-family CC0 donors only; no exact M700/Stein identity claim")
    print("STATUS: UE ASSET IMPORTED; authored animation/audibility/timing runtime acceptance still pending")
    print("runtime_acceptance=0 item16_checked=0")


if __name__ == "__main__":
    main()
