#!/usr/bin/env python3
"""Independent UE 5.8 fresh-load verifier for PASS45 manual-action donor SoundWaves."""
from __future__ import annotations

try:
    import unreal
except ImportError as exc:  # pragma: no cover - must run inside Unreal Editor Python
    raise SystemExit("PASS45 MANUAL ACTION AUDIO FRESH LOAD: run inside Unreal Editor 5.8 Python") from exc

ASSETS = {
    "bolt": (
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_BoltAction_CC0_Donor.SW_PASS45_BoltAction_CC0_Donor",
    ),
    "lever": (
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor",
        "/Game/PASS45/Audio/ManualAction/SW_PASS45_LeverAction_CC0_Donor.SW_PASS45_LeverAction_CC0_Donor",
    ),
}


def fail(message: str) -> None:
    raise RuntimeError(f"PASS45 MANUAL ACTION AUDIO FRESH LOAD: {message}")


def main() -> None:
    loaded: dict[str, str] = {}
    for donor_key, (asset_path, object_path) in ASSETS.items():
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            fail(f"missing imported SoundWave: {asset_path}")
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset is None:
            fail(f"cannot fresh-load imported SoundWave: {asset_path}")
        if asset.get_class().get_name() != "SoundWave":
            fail(f"fresh-loaded asset is not SoundWave: {asset_path} class={asset.get_class().get_name()}")
        if asset.get_path_name() != object_path:
            fail(f"fresh-loaded object path mismatch: expected={object_path} actual={asset.get_path_name()}")
        loaded[donor_key] = object_path

    print("PASS45 MANUAL ACTION AUDIO FRESH LOAD: PASS")
    print(f"- BoltCycle donor: {loaded['bolt']}")
    print(f"- LeverCycle donor: {loaded['lever']}")
    print("STATUS: FRESH-LOAD VALIDATED; audibility/timing/mix and authored animations remain pending")
    print("runtime_acceptance=0 item16_checked=0")


if __name__ == "__main__":
    main()
