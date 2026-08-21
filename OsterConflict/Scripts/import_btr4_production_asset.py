from pathlib import Path
import sys

import unreal

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from generate_btr4_game_visual import build_btr4_glb
from import_production_vehicle_assets import import_btr_fbx, import_glb_combined

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_DIR = PROJECT_DIR / "SourceAssets" / "Production" / "Vehicles" / "BTR4"
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "BTR4"
LOCAL_FBX = SOURCE_DIR / "BTR4_Bucephalus.fbx"
LOCAL_TEXTURES = SOURCE_DIR / "Textures"
GENERATED_SOURCE = CACHE_DIR / "btr4_bucephalus_oc_authored.glb"
DESTINATION = "/Game/Production/Vehicles/BTR4"
ASSET_NAME = "SM_BTR4_Bucephalus"
SENTINEL = CACHE_DIR / "btr4_import_success.txt"


def log(message):
    unreal.log(f"[OC BTR4 Import] {message}")


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()

    if LOCAL_FBX.exists():
        log(f"Using local user-selected BTR-4 FBX: {LOCAL_FBX}")
        imported_path = import_btr_fbx(LOCAL_FBX, LOCAL_TEXTURES, DESTINATION, ASSET_NAME)
        source_kind = "local_user_fbx"
        source = LOCAL_FBX
    else:
        build_btr4_glb(GENERATED_SOURCE)
        if not GENERATED_SOURCE.exists() or GENERATED_SOURCE.stat().st_size <= 0:
            raise RuntimeError("Authored BTR-4 GLB generation failed.")
        log(
            "Local BTR-4 FBX is not present in the worktree; generated authored external-only "
            f"game visual: {GENERATED_SOURCE}"
        )
        imported_path = import_glb_combined(GENERATED_SOURCE, DESTINATION, ASSET_NAME)
        source_kind = "authored_external_visual"
        source = GENERATED_SOURCE

    unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)
    SENTINEL.write_text(
        f"asset={imported_path}\nsource={source}\nsource_kind={source_kind}\n",
        encoding="utf-8",
    )
    log(f"BTR-4 canonical visual ready at {imported_path}; source_kind={source_kind}")


if __name__ == "__main__":
    main()
