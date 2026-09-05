import json
from pathlib import Path

import unreal

import import_all_local_inbox_assets as inbox
import import_local_production_weapon_assets as production_weapons
import import_production_vehicle_assets as production

PROJECT_DIR = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STATE_ROOT = PROJECT_DIR / "Saved" / "LocalModelInbox"
BINDINGS = STATE_ROOT / "runtime_bindings.json"
SUCCESS = STATE_ROOT / "runtime_bindings_success.txt"

# Scan every project/plugin content mount that Unreal actually knows about. This avoids the old
# brittle hard-coded pack list, so assets downloaded later through Fab/Marketplace are picked up
# automatically without another source edit.
CORE_AUTHORED_PREFIXES = (
    "/Game/R13",
    "/Game/QuantumCharacter",
    "/Game/Maps",
    "/Game/UI",
)
IGNORED_MOUNT_PREFIXES = (
    "/Engine",
    "/Script",
    "/Temp",
    "/Transient",
    "/Memory",
)


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


def _read_status_sentinel(path):
    if not path.is_file():
        return None, []
    try:
        lines = path.read_text(encoding="utf-8-sig", errors="replace").splitlines()
    except Exception as exc:
        return None, [f"SENTINEL_READ_FAILED={type(exc).__name__}:{exc}"]

    status = None
    details = []
    for line in lines:
        text = line.strip()
        if text.startswith("STATUS="):
            status = text.partition("=")[2].strip().upper()
        elif text.startswith("GAP=") or text.startswith("CONTENT_GAP="):
            details.append(text)
    return status, details


def _run_required_ingest(label, runner, sentinel, reason):
    try:
        if sentinel.exists():
            sentinel.unlink()
    except Exception as exc:
        failure = {
            "source": f"REQUIRED_INGEST:{label}",
            "category": "PRODUCTION_REQUIRED",
            "status": "UNBOUND",
            "reason": f"{reason}_sentinel_reset_failed:{type(exc).__name__}:{exc}",
            "sentinel": str(sentinel),
        }
        inbox.warn(
            f"{label} stale status sentinel could not be cleared; aggregate asset PASS is blocked: "
            f"{type(exc).__name__}: {exc}"
        )
        return [failure]

    try:
        runner()
    except Exception as exc:
        failure = {
            "source": f"REQUIRED_INGEST:{label}",
            "category": "PRODUCTION_REQUIRED",
            "status": "UNBOUND",
            "reason": f"{reason}_exception:{type(exc).__name__}:{exc}",
            "sentinel": str(sentinel),
        }
        inbox.warn(
            f"{label} ingest failed; continuing independent Fab/inbox catalog, "
            "but aggregate asset PASS is blocked: "
            f"{type(exc).__name__}: {exc}"
        )
        return [failure]

    status, details = _read_status_sentinel(sentinel)
    if status == "PASS":
        return []

    failure = {
        "source": f"REQUIRED_INGEST:{label}",
        "category": "PRODUCTION_REQUIRED",
        "status": "UNBOUND",
        "reason": f"{reason}:status={status or 'MISSING'}",
        "sentinel": str(sentinel),
    }
    if details:
        failure["details"] = details[:16]

    inbox.warn(
        f"{label} ingest returned {status or 'MISSING'}; continuing independent Fab/inbox catalog, "
        "but aggregate asset PASS is blocked."
    )
    return [failure]


def _is_core_authored_path(object_path):
    package = object_path.split(".", 1)[0]
    return any(package == prefix or package.startswith(prefix + "/") for prefix in CORE_AUTHORED_PREFIXES)


def _discover_content_mounts(registry):
    roots = {"/Game"}
    try:
        cached_paths = registry.get_all_cached_paths()
    except Exception:
        cached_paths = []

    for raw_path in cached_paths:
        path = str(raw_path).replace("\\", "/")
        if not path.startswith("/"):
            continue
        if any(path == prefix or path.startswith(prefix + "/") for prefix in IGNORED_MOUNT_PREFIXES):
            continue
        parts = path.strip("/").split("/", 1)
        if parts and parts[0]:
            roots.add("/" + parts[0])

    return sorted(roots)


def _catalog_existing_project_models(bindings, quantum_skeleton_path):
    registry = unreal.AssetRegistryHelpers.get_asset_registry()

    # Prime /Game first so Fab/Marketplace folders are visible, then discover any project plugin mounts.
    registry.scan_paths_synchronous(["/Game"], True)
    content_roots = _discover_content_mounts(registry)
    try:
        registry.scan_paths_synchronous(content_roots, True)
    except Exception:
        # /Game is always the minimum valid project mount; individual plugin paths are handled below.
        registry.scan_paths_synchronous(["/Game"], True)

    existing_paths = {entry.get("path") for entry in bindings.get("static_assets", [])}
    existing_paths.update(entry.get("path") for entry in bindings.get("skeletal_assets", []))

    discovered = 0
    static_count = 0
    skeletal_count = 0
    failures = []
    scanned_roots = []

    for root in content_roots:
        try:
            assets = registry.get_assets_by_path(root, recursive=True, include_only_on_disk_assets=False)
        except TypeError:
            try:
                assets = registry.get_assets_by_path(root, True)
            except Exception as exc:
                failures.append({
                    "source": f"PROJECT_CONTENT:{root}",
                    "category": "UNCLASSIFIED",
                    "status": "UNBOUND",
                    "reason": f"asset_registry_scan_failed:{type(exc).__name__}:{exc}",
                })
                continue
        except Exception as exc:
            failures.append({
                "source": f"PROJECT_CONTENT:{root}",
                "category": "UNCLASSIFIED",
                "status": "UNBOUND",
                "reason": f"asset_registry_scan_failed:{type(exc).__name__}:{exc}",
            })
            continue

        scanned_roots.append(root)
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
            if _is_core_authored_path(object_path):
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
    return discovered, static_count, skeletal_count, failures, scanned_roots


def main():
    # A success marker is valid only for the current aggregate pass. Clear any previous run before
    # the first operation so an early exception cannot leave stale green evidence behind.
    if SUCCESS.exists():
        SUCCESS.unlink()

    # Run the normal models_game_OC import first in this SAME Unreal process.
    inbox.main()

    # Production sub-importers intentionally keep scanning independent assets after a partial gap.
    # The aggregate pipeline may also continue cataloging, but it must preserve every required
    # production GAP in runtime_bindings.json so the final success sentinel cannot lie.
    required_ingest_failures = []
    required_ingest_failures.extend(
        _run_required_ingest(
            "PRODUCTION_VEHICLES",
            production.main,
            production.SUCCESS_SENTINEL,
            "production_vehicle_import_not_pass",
        )
    )

    # Exact M249 and Remington 870 sources are staged before this Python pass. Missing/failed exact
    # sources do not block discovery of unrelated downloaded packs, but they DO block aggregate PASS.
    required_ingest_failures.extend(
        _run_required_ingest(
            "EXACT_PRODUCTION_WEAPONS",
            production_weapons.main,
            production_weapons.SUCCESS_SENTINEL,
            "production_weapon_import_not_pass",
        )
    )

    if not BINDINGS.is_file():
        raise RuntimeError(f"base runtime binding manifest missing after inbox import: {BINDINGS}")

    bindings = json.loads(BINDINGS.read_text(encoding="utf-8-sig"))
    if required_ingest_failures:
        bindings.setdefault("unbound_models", []).extend(required_ingest_failures)
        bindings.setdefault("source_status", []).extend(required_ingest_failures)

    quantum = inbox.load_asset(inbox.QUANTUM_BODY)
    quantum_skeleton_path = inbox.skeleton_path(quantum)

    discovered, static_count, skeletal_count, failures, scanned_roots = _catalog_existing_project_models(
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
        "required_production_ingest_failures": len(required_ingest_failures),
        "unbound_models": len(bindings["unbound_models"]),
    }
    bindings["all_models_bound"] = len(bindings["unbound_models"]) == 0
    bindings["project_content_roots"] = scanned_roots
    bindings["excluded_core_authored_roots"] = list(CORE_AUTHORED_PREFIXES)

    BINDINGS.write_text(json.dumps(bindings, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    if bindings["all_models_bound"]:
        SUCCESS.write_text("PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS\n", encoding="utf-8")
        inbox.log(
            "PASS local inbox + UE/Fab/project-plugin model catalog bound. "
            f"summary={bindings['summary']} roots={scanned_roots} manifest={BINDINGS}"
        )
    else:
        if SUCCESS.exists():
            SUCCESS.unlink()
        inbox.warn(
            "GAP local inbox + project/plugin content contains unbound models. "
            f"count={len(bindings['unbound_models'])} roots={scanned_roots} manifest={BINDINGS}"
        )


if __name__ == "__main__":
    main()
