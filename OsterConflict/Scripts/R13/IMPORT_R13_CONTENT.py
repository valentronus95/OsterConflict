from pathlib import Path
import unreal

PROJECT_DIR = Path(__file__).resolve().parents[2]
RAW_ROOT = PROJECT_DIR / "Content" / "Raw" / "R13"
WEAPON_ROOT = RAW_ROOT / "Weapons" / "Kenney"
AUDIO_ROOT = RAW_ROOT / "Audio"
UI_ROOT = RAW_ROOT / "UI"


def import_required_file(source: Path, destination: str):
    if not source.exists():
        raise RuntimeError(f"R13 required import source is missing: {source}")

    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    imported = list(task.imported_object_paths)
    if not imported:
        raise RuntimeError(f"R13 importer returned no assets for required source: {source}")

    unreal.log(f"R13 imported {source.name} -> {destination}: {', '.join(imported)}")
    return imported


weapon_files = [
    "machinegun.obj",
    "pistol.obj",
    "shotgun.obj",
    "sniper.obj",
    "uzi.obj",
    "rocketlauncherModern.obj",
    "grenade.obj",
]

for filename in weapon_files:
    import_required_file(WEAPON_ROOT / filename, "/Game/R13/Weapons")

required_audio = sorted(AUDIO_ROOT.glob("*.wav"))
if not required_audio:
    raise RuntimeError(f"R13 required audio directory contains no WAV files: {AUDIO_ROOT}")
for wav in required_audio:
    import_required_file(wav, "/Game/R13/Audio")

import_required_file(UI_ROOT / "Oster_Menu_BG.jpg", "/Game/R13/UI")
unreal.EditorAssetLibrary.save_directory("/Game/R13", only_if_is_dirty=False, recursive=True)

# Runtime code uses string LoadObject paths, so verify the exact package/object names it expects before declaring PASS.
expected_assets = [
    "/Game/R13/Weapons/machinegun.machinegun",
    "/Game/R13/Weapons/pistol.pistol",
    "/Game/R13/Weapons/shotgun.shotgun",
    "/Game/R13/Weapons/sniper.sniper",
    "/Game/R13/Weapons/uzi.uzi",
    "/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern",
    "/Game/R13/Weapons/grenade.grenade",
    "/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG",
]
missing_assets = [asset for asset in expected_assets if not unreal.EditorAssetLibrary.does_asset_exist(asset)]
if missing_assets:
    raise RuntimeError("R13 import finished but runtime-required assets are missing: " + ", ".join(missing_assets))

unreal.log(f"R13 CONTENT IMPORT COMPLETE: verified {len(expected_assets)} runtime-required assets")
