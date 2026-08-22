from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_DIR = PROJECT_DIR / "Saved" / "AutomationReports" / "ProductionModels"
REPORT_PATH = REPORT_DIR / "required_weapon_asset_preflight.txt"
SUCCESS_SENTINEL = REPORT_DIR / "required_weapon_asset_preflight_success.txt"

# Runtime evidence from UE 5.8 is authoritative here. The restored Stein packages keep SKM_* object
# names, but UE reports MP5/1911/M700/M14/Mac10/Tec9/LeverAction as StaticMesh assets. Treating the
# filename prefix as a class contract blocked a valid local checkout and also disagreed with runtime.
REQUIRED_ASSETS = (
    ("AK-47", "/Game/AK-47/Mesh/SKM_AK-47", unreal.SkeletalMesh),
    ("MP5", "/Game/R13/Weapons/Stein/MP5/SKM_MP5", unreal.StaticMesh),
    ("M1911", "/Game/R13/Weapons/Stein/1911/SKM_1911", unreal.StaticMesh),
    ("M700", "/Game/R13/Weapons/Stein/M700/SKM_M700", unreal.StaticMesh),
    ("M14", "/Game/R13/Weapons/Stein/M14/SKM_M14", unreal.StaticMesh),
    ("MAC-10", "/Game/R13/Weapons/Stein/Mac10/SKM_Mac10", unreal.StaticMesh),
    ("TEC-9", "/Game/R13/Weapons/Stein/Tec9/SKM_Tec9", unreal.StaticMesh),
    ("Lever Action", "/Game/R13/Weapons/Stein/LeverAction/SKM_LeverAction", unreal.StaticMesh),
    ("M249 real fallback", "/Game/R13/Weapons/machinegun", unreal.StaticMesh),
    ("Remington 870 real fallback", "/Game/R13/Weapons/shotgun", unreal.StaticMesh),
    ("Anti-Armor Launcher", "/Game/R13/Weapons/rocketlauncherModern", unreal.StaticMesh),
)


def log(message):
    unreal.log(f"[OC Weapon Asset Preflight] {message}")


def main():
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    if SUCCESS_SENTINEL.exists():
        SUCCESS_SENTINEL.unlink()

    report = ["OSTER CONFLICT REQUIRED REAL WEAPON ASSET PREFLIGHT"]
    failures = []

    for label, object_path, expected_type in REQUIRED_ASSETS:
        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        loaded = asset is not None
        type_ok = loaded and isinstance(asset, expected_type)
        result = "PASS" if type_ok else "FAIL"
        actual_type = asset.get_class().get_name() if loaded else "MISSING"
        report.append(
            f"{label} | path={object_path} | loaded={loaded} | "
            f"expected={expected_type.__name__} | actual={actual_type} | RESULT={result}"
        )
        if not type_ok:
            failures.append(f"{label}: {object_path} ({actual_type})")

    report.append("")
    report.append(f"SUMMARY={len(REQUIRED_ASSETS) - len(failures)}/{len(REQUIRED_ASSETS)} PASS")
    REPORT_PATH.write_text("\n".join(report) + "\n", encoding="utf-8")

    if failures:
        for failure in failures:
            unreal.log_error(f"[OC Weapon Asset Preflight] {failure}")
        unreal.log_error(
            f"[OC Weapon Asset Preflight] FAILED. Normal gameplay is blocked before primitive weapon "
            f"fallbacks can be shown. Report: {REPORT_PATH}"
        )
        return

    SUCCESS_SENTINEL.write_text("REQUIRED_REAL_WEAPON_ASSETS=PASS\n", encoding="utf-8")
    log(f"PASS: all {len(REQUIRED_ASSETS)} required real weapon visuals load in a fresh UE process.")
    log(f"Report: {REPORT_PATH}")


if __name__ == "__main__":
    main()
