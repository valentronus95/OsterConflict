from pathlib import Path
import unreal

PROJECT_DIR = Path(__file__).resolve().parents[2]
RAW_ROOT = PROJECT_DIR / "Content" / "Raw" / "R13"
WEAPON_ROOT = RAW_ROOT / "Weapons" / "Kenney"
AUDIO_ROOT = RAW_ROOT / "Audio"
UI_ROOT = RAW_ROOT / "UI"


def import_file(source: Path, destination: str):
    if not source.exists():
        unreal.log_warning(f"R13 import missing source: {source}")
        return []
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    unreal.log(f"R13 imported {source.name} -> {destination}")
    return list(task.imported_object_paths)


weapons = [
    "machinegun.obj",
    "pistol.obj",
    "shotgun.obj",
    "sniper.obj",
    "uzi.obj",
    "rocketlauncherModern.obj",
    "grenade.obj",
]

for filename in weapons:
    import_file(WEAPON_ROOT / filename, "/Game/R13/Weapons")

for wav in sorted(AUDIO_ROOT.glob("*.wav")):
    import_file(wav, "/Game/R13/Audio")

import_file(UI_ROOT / "Oster_Menu_BG.jpg", "/Game/R13/UI")

unreal.EditorAssetLibrary.save_directory("/Game/R13", only_if_is_dirty=False, recursive=True)
unreal.log("R13 CONTENT IMPORT COMPLETE")
