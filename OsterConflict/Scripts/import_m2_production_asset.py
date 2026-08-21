from pathlib import Path
import sys

import unreal

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from generate_m2_game_visual import build_m2_glb
from import_production_vehicle_assets import import_glb_combined

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_DIR = PROJECT_DIR / "SourceAssets" / "Production" / "Weapons" / "M2"
CACHE_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache" / "M2"
DOWNLOADED_SOURCE = SOURCE_DIR / "m2_50cal_machinegun_cc0.glb"
GENERATED_SOURCE = CACHE_DIR / "m2_browning_oc_authored.glb"
DESTINATION = "/Game/Production/Weapons/M2"
ASSET_NAME = "SM_M2_Browning"
SENTINEL = CACHE_DIR / "m2_import_success.txt"


def log(message):
    unreal.log(f"[OC M2 Import] {message}")


def choose_source():
    if DOWNLOADED_SOURCE.exists():
        log(f"Using downloaded M2 source: {DOWNLOADED_SOURCE}")
        return DOWNLOADED_SOURCE, "downloaded"

    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    build_m2_glb(GENERATED_SOURCE)
    if not GENERATED_SOURCE.exists() or GENERATED_SOURCE.stat().st_size <= 0:
        raise RuntimeError("Authored M2 GLB generation failed.")
    log(f"Downloaded M2 source is not present in Git/worktree; generated authored game-visual source: {GENERATED_SOURCE}")
    return GENERATED_SOURCE, "authored"


def main():
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    if SENTINEL.exists():
        SENTINEL.unlink()

    source, source_kind = choose_source()
    imported_path = import_glb_combined(source, DESTINATION, ASSET_NAME)
    unreal.EditorAssetLibrary.save_directory(DESTINATION, only_if_is_dirty=False, recursive=True)

    SENTINEL.write_text(
        f"asset={imported_path}\nsource={source}\nsource_kind={source_kind}\n",
        encoding="utf-8",
    )
    log(f"M2 production visual ready at {imported_path}; source_kind={source_kind}")


if __name__ == "__main__":
    main()
