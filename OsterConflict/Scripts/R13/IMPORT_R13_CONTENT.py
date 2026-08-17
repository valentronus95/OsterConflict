from pathlib import Path
import unreal

PROJECT_DIR = Path(__file__).resolve().parents[2]
RAW_ROOT = PROJECT_DIR / "Content" / "Raw" / "R13"
KENNEY_ROOT = RAW_ROOT / "Weapons" / "Kenney"
STEIN_ROOT = RAW_ROOT / "Weapons" / "SteinClassicWeapons" / "WeaponsPack"
AUDIO_ROOT = RAW_ROOT / "Audio"
UI_ROOT = RAW_ROOT / "UI"
LOCAL_MENU_SOURCE = PROJECT_DIR / "Content" / "R13" / "UI" / "Oster_Menu_BG.jpg"
MENU_ASSET = "/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG"


def run_import_task(task: unreal.AssetImportTask, source: Path, destination: str):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.imported_object_paths)
    if not imported:
        raise RuntimeError(f"R13 importer returned no assets for required source: {source}")
    unreal.log(f"R13 imported {source.name} -> {destination}: {', '.join(imported)}")
    return imported


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
    return run_import_task(task, source, destination)


def import_stein_static_mesh(source: Path, destination: str):
    if not source.exists():
        raise RuntimeError(f"Stein Classic Weapons FBX is missing: {source}")

    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    static_data = options.get_editor_property("static_mesh_import_data")
    if static_data:
        static_data.set_editor_property("combine_meshes", True)

    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    task.options = options
    return run_import_task(task, source, destination)


# Keep the small CC0 Kenney set as fallback for classes Stein does not cover yet.
kenney_files = [
    "machinegun.obj",
    "pistol.obj",
    "shotgun.obj",
    "sniper.obj",
    "uzi.obj",
    "rocketlauncherModern.obj",
    "grenade.obj",
]
for filename in kenney_files:
    import_required_file(KENNEY_ROOT / filename, "/Game/R13/Weapons")

# Stein Games Classic Weapons Pack, CC0 1.0. Import every committed FBX as a static gameplay mesh.
stein_weapons = {
    "1911": "SKM_1911.fbx",
    "AK47": "SKM_AK47.fbx",
    "LeverAction": "SKM_LeverAction.fbx",
    "M14": "SKM_M14.fbx",
    "M700": "SKM_M700.fbx",
    "MP5": "SKM_MP5.fbx",
    "Mac10": "SKM_Mac10.fbx",
    "Tec9": "SKM_Tec9.fbx",
}
for folder, filename in stein_weapons.items():
    import_stein_static_mesh(STEIN_ROOT / folder / filename, f"/Game/R13/Weapons/Stein/{folder}")

required_audio = sorted(AUDIO_ROOT.glob("*.wav"))
if not required_audio:
    raise RuntimeError(f"R13 required audio directory contains no WAV files: {AUDIO_ROOT}")
for wav in required_audio:
    import_required_file(wav, "/Game/R13/Audio")

# Prefer the player-supplied artwork over the fallback museum photo. Reimport in place rather than deleting the
# existing texture package: deleting an asset that is already referenced by the runtime UI can fail in commandlet
# mode. AssetImportTask with replace_existing=True safely refreshes the texture while preserving its object path.
menu_source = LOCAL_MENU_SOURCE if LOCAL_MENU_SOURCE.exists() else (UI_ROOT / "Oster_Menu_BG.jpg")
if not menu_source.exists():
    raise RuntimeError(f"R13 required menu background is missing: {menu_source}")
unreal.log(f"R13 menu background source: {menu_source}")
import_required_file(menu_source, "/Game/R13/UI")
unreal.EditorAssetLibrary.save_directory("/Game/R13", only_if_is_dirty=False, recursive=True)

# Runtime code uses string LoadObject paths, so verify exact package/object names before declaring PASS.
expected_assets = [
    "/Game/R13/Weapons/machinegun.machinegun",
    "/Game/R13/Weapons/shotgun.shotgun",
    "/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern",
    "/Game/R13/Weapons/grenade.grenade",
    "/Game/R13/Weapons/Stein/1911/SKM_1911.SKM_1911",
    "/Game/R13/Weapons/Stein/AK47/SKM_AK47.SKM_AK47",
    "/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction.SKM_LeverAction",
    "/Game/R13/Weapons/Stein/M14/SKM_M14.SKM_M14",
    "/Game/R13/Weapons/Stein/M700/SKM_M700.SKM_M700",
    "/Game/R13/Weapons/Stein/MP5/SKM_MP5.SKM_MP5",
    "/Game/R13/Weapons/Stein/Mac10/SKM_Mac10.SKM_Mac10",
    "/Game/R13/Weapons/Stein/Tec9/SKM_Tec9.SKM_Tec9",
    MENU_ASSET,
]
missing_assets = [asset for asset in expected_assets if not unreal.EditorAssetLibrary.does_asset_exist(asset)]
if missing_assets:
    raise RuntimeError("R13 import finished but runtime-required assets are missing: " + ", ".join(missing_assets))

unreal.log(f"R13 CONTENT IMPORT COMPLETE: verified {len(expected_assets)} runtime-required assets including Stein CC0 pack")
