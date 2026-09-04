import json
from pathlib import Path

import unreal

import import_all_local_inbox_assets as inbox

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STATE_ROOT = PROJECT_DIR / "Saved" / "LocalModelInbox"
BINDINGS = STATE_ROOT / "runtime_bindings.json"
SUCCESS = STATE_ROOT / "runtime_bindings_success.txt"

# These are the asset packs that arrived through Unreal/Fab/Marketplace or the production import path.
# Core authored game roots (R13, QuantumCharacter, Maps, UI) are intentionally not duplicated into this pool.
PROJECT_CONTENT_ROOTS = [
    "/Game/AK-47",
    "/Game/AdvancedVillagePack",
    "/Game/Fab",
    "/Game/Fire_EXP_Vol01_Free",
    "/Game/KiteDemo",
    "/Game/Mega_Street_Props_Pack",
    "/Game/Megaplant_Library",
    "/Game/Modular_Rural_Cabin",
    "/Game/PN_FoliageCollection",
    "/Game/PotaVFX_Smoke",
    "/Game/Production",
    "/Game/Scene_RoadsideConstruction",
    "/Game/Scene_UnfinishedBuilding",
    "/Game/Street_Props_Pack_V1",
    "/Game/TileableForestRoad",
    "/Game/VehicleVarietyPack",
]


def _asset_class_name(asset_data):
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        try:
            return str(asset_data.asset_class)
        except Exception:
            return ""


def _object_path(asset_data):
    try:
        package = str(asset_data.package_name)
        name = str(asset_data.asset_name)
        if package and name:
            return f"{package}.{name}"
    except Exception:
        pass
    try:
        path = str(asset_data.object_path)
        return path if path else None
    except Exception:
        return None


def _dedupe(bindings, key):
    unique = []
    seen = set()
    for entry in bindings.get(key, []):
        path = entry.get("path")
        if not path or path in seen:
            continue
        seen.add(path)
        unique.append(entry)
    bindings[key] = unique


def _catalog_existing_project_models(bindings, quantum_skeleton_path):
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous(PROJECT_CONTENT_ROOTS, True)

    existing_paths = {entry.get("path") for entry in bindings.get("static_assets", [])}
    existing_paths.update(entry.get("path") for entry in bindings.get("skeletal_assets", []))

    discovered = 0
    static_count = 0
    skeletal_count = 0
    failures = []

    for root in PROJECT_CONTENT_ROOTS:
        try:
            assets = registry.get_assets_by_path(root, recursive=True, include_only_on_disk_assets=False)
        except TypeError:
            assets = registry.get_assets_by_path(root, True)
        except Exception as exc:
            failures.append({
                "source": f"PROJECT_CONTENT:{root}",
                "category": "UNCLASSIFIED",
                "status": "UNBOUND",
                "reason": f"asset_registry_scan_failed:{type(exc).__name__}:{exc}",
            })
            continue

        for data in assets:
            class_name = _asset_class_name(data)
            if "StaticMesh" not in class_name and "SkeletalMesh" not in class_name:
                continue

            object_path = _object_path(data)
            if not object_path:
                failures.append({
                    "source": f"PROJECT_CONTENT:{root}",
                    "category": "UNCLASSIFIED",
                    "status": "UNBOUND",
                    "reason": "asset_registry_object_path_missing",
                })
                continue
            if object_path in existing_paths:
                continue

            discovered += 1
            category = inbox.category_for(object_path)
            source = f"PROJECT_CONTENT:{object_path}"

            if "StaticMesh" in class_name:
                # A static human-looking mesh is scenery, not a fake playable skin.
                if category == "CHARACTER_SKIN":
                    category = "PROP"
                bindings.setdefault("static_assets", []).append({
                    "path": object_path,
                    "category": category,
                    "source": source,
                })
                bindings.setdefault("source_status", []).append({
                    "source": source,
                    "category": category,
                    "status": "BOUND",
                    "binding": "PROJECT_CONTENT_STATIC_RUNTIME_POOL",
                    "asset": object_path,
                })
                existing_paths.add(object_path)
                static_count += 1
                continue

            # Skeletal assets need a real load so skin compatibility is factual instead of guessed from a filename.
            before = len(bindings.setdefault("source_status", []))
            inbox.classify_loaded_asset(
                object_path, source, category, quantum_skeleton_path,
                bindings, bindings["source_status"]
            )
            if len(bindings["source_status"]) == before:
                failures.append({
                    "source": source,
                    "category": category,
                    "status": "UNBOUND",
                    "reason": "project_skeletal_asset_not_classified",
                    "asset": object_path,
                })
            elif bindings["source_status"][-1].get("status") == "UNBOUND":
                failures.append(bindings["source_status"][-1])
            else:
                skeletal_count += 1
                existing_paths.add(object_path)

    bindings.setdefault("unbound_models", []).extend(failures)
    return discovered, static_count, skeletal_count, failures


def main():
    # Run the normal models_game_OC import first in this SAME Unreal process.
    inbox.main()

    if not BINDINGS.is_file():
        raise RuntimeError(f"base runtime binding manifest missing after inbox import: {BINDINGS}")

    bindings = json.loads(BINDINGS.read_text(encoding="utf-8-sig"))
    quantum = inbox.load_asset(inbox.QUANTUM_BODY)
    quantum_skeleton_path = inbox.skeleton_path(quantum)

    discovered, static_count, skeletal_count, failures = _catalog_existing_project_models(
        bindings, quantum_skeleton_path
    )

    for key in ("static_assets", "skeletal_assets", "hud_textures", "hud_widget_classes"):
        _dedupe(bindings, key)

    # Deduplicate failure rows as well so a pack scanned on multiple launches cannot multiply noise.
    unique_failures = []
    seen_failures = set()
    for row in bindings.get("unbound_models", []):
        identity = (row.get("source"), row.get("reason"), row.get("asset"))
        if identity in seen_failures:
            continue
        seen_failures.add(identity)
        unique_failures.append(row)
    bindings["unbound_models"] = unique_failures

    compatible_skins = [
        x for x in bindings.get("skeletal_assets", [])
        if x.get("category") == "CHARACTER_SKIN" and x.get("character_compatible")
    ]
    bindings["summary"] = {
        "static_assets": len(bindings.get("static_assets", [])),
        "skeletal_assets": len(bindings.get("skeletal_assets", [])),
        "compatible_character_skins": len(compatible_skins),
        "hud_textures": len(bindings.get("hud_textures", [])),
        "hud_widget_classes": len(bindings.get("hud_widget_classes", [])),
        "project_content_models_discovered": discovered,
        "project_content_static_bound": static_count,
        "project_content_skeletal_bound": skeletal_count,
        "project_content_failures": len(failures),
        "unbound_models": len(bindings["unbound_models"]),
    }
    bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0
    bindings["project_content_roots"] = PROJECT_CONTENT_ROOTS

    BINDINGS.write_text(json.dumps(bindings, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    if bindings["all_models_bound"]:
        SUCCESS.write_text("PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS\n", encoding="utf-8")
        inbox.log(
            "PASS local inbox + UE/Fab project model catalog bound. "
            f"summary={bindings['summary']} manifest={BINDINGS}"
        )
    else:
        if SUCCESS.exists():
            SUCCESS.unlink()
        inbox.warn(
            "GAP local inbox + project content contains unbound models. "
            f"count={len(bindings['unbound_models'])} manifest={BINDINGS}"
        )


if __name__ == "__main__":
    main()
