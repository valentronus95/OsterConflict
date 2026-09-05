import json
from pathlib import Path

import unreal

from import_production_vehicle_assets import import_btr_fbx, import_glb_combined


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
CACHE_ROOT = PROJECT_DIR / "Saved" / "LocalProductionSourceIntake" / "Weapons"
MANIFEST_PATH = CACHE_ROOT / "weapon_sources.json"
RESULT_DIR = PROJECT_DIR / "Saved" / "ProductionAssetImportCache"
SUCCESS_SENTINEL = RESULT_DIR / "production_weapon_import_result.txt"

TARGETS = {
    "M249": ("/Game/Production/Weapons/M249", "SM_M249"),
    "Remington870": ("/Game/Production/Weapons/Remington870", "SM_Remington870"),
}


def log(message):
    unreal.log(f"[OC Local Production Weapon Import] {message}")


def warn(message):
    unreal.log_warning(f"[OC Local Production Weapon Import] {message}")


def canonical_asset_path(destination: str, asset_name: str) -> str:
    return f"{destination}/{asset_name}"


def import_source(source: Path, destination: str, asset_name: str) -> str:
    extension = source.suffix.lower()
    if extension == ".glb":
        return import_glb_combined(source, destination, asset_name)
    if extension == ".fbx":
        return import_btr_fbx(source, source.parent, destination, asset_name)
    raise RuntimeError(f"unsupported exact weapon source format: {source}")


def main():
    RESULT_DIR.mkdir(parents=True, exist_ok=True)
    if SUCCESS_SENTINEL.exists():
        SUCCESS_SENTINEL.unlink()

    imported = []
    present = []
    gaps = []

    pending = []
    for label, (destination, asset_name) in TARGETS.items():
        asset_path = canonical_asset_path(destination, asset_name)
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            present.append(asset_path)
            log(f"SKIP already imported {label}: {asset_path}")
        else:
            pending.append(label)

    if not pending:
        lines = ["STATUS=PASS"]
        lines.extend(f"PRESENT={path}" for path in present)
        SUCCESS_SENTINEL.write_text("\n".join(lines) + "\n", encoding="utf-8")
        log("RESULT all exact production weapons are already present; no re-import performed.")
        return

    if not MANIFEST_PATH.is_file():
        gaps.append("WEAPON_SOURCE_MANIFEST_MISSING")
        warn(f"CONTENT GAP: local production weapon manifest is missing: {MANIFEST_PATH}")
        lines = ["STATUS=GAP"]
        lines.extend(f"PRESENT={path}" for path in present)
        lines.extend(f"GAP={gap}" for gap in gaps)
        SUCCESS_SENTINEL.write_text("\n".join(lines) + "\n", encoding="utf-8")
        return

    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8-sig"))

    for label, (destination, asset_name) in TARGETS.items():
        asset_path = canonical_asset_path(destination, asset_name)
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            continue

        source_value = manifest.get(label)
        if not source_value:
            gaps.append(f"{label}_SOURCE_MISSING")
            warn(f"CONTENT GAP: {label} exact local source is missing")
            continue

        source = Path(source_value)
        if not source.is_file():
            gaps.append(f"{label}_SOURCE_NOT_FOUND={source}")
            warn(f"CONTENT GAP: {label} manifest source no longer exists: {source}")
            continue

        try:
            imported_path = import_source(source, destination, asset_name)
            unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)
            imported.append(imported_path)
            log(f"IMPORTED {label}: {source} -> {imported_path}")
        except Exception as exc:
            gaps.append(f"{label}_IMPORT_FAILED={type(exc).__name__}:{exc}")
            unreal.log_error(f"[OC Local Production Weapon Import] {label} import failed: {exc}")

    final_present = []
    for label, (destination, asset_name) in TARGETS.items():
        asset_path = canonical_asset_path(destination, asset_name)
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            final_present.append(asset_path)
        elif not any(gap.startswith(label + "_") for gap in gaps):
            gaps.append(f"{label}_CANONICAL_ASSET_MISSING_AFTER_IMPORT={asset_path}")

    status = "PASS" if len(final_present) == len(TARGETS) else "GAP"
    lines = [f"STATUS={status}"]
    lines.extend(f"PRESENT={path}" for path in final_present)
    lines.extend(f"IMPORTED={path}" for path in imported)
    lines.extend(f"GAP={gap}" for gap in gaps)
    SUCCESS_SENTINEL.write_text("\n".join(lines) + "\n", encoding="utf-8")

    log(
        f"RESULT present={len(final_present)}/{len(TARGETS)} newly_imported={len(imported)} "
        f"gaps={len(gaps)} sentinel={SUCCESS_SENTINEL}"
    )


if __name__ == "__main__":
    main()
