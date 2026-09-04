import json
import re
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parents[1]
STATE_ROOT = PROJECT_DIR / "Saved" / "LocalModelInbox"
BINDINGS = STATE_ROOT / "runtime_bindings.json"
SUCCESS = STATE_ROOT / "runtime_bindings_success.txt"

# Ordered from exact families to broad gameplay classes. This is intentionally wider than the
# old five-asset production list: every downloaded weapon must resolve to a live weapon consumer.
RULES = [
    ("AK74", r"(?:^|[^a-z0-9])ak[ _.-]?74(?:m)?(?:[^a-z0-9]|$)"),
    ("AR15", r"(?:^|[^a-z0-9])ar[ _.-]?15(?:[^a-z0-9]|$)"),
    ("BALLISTA", r"ballista"),
    ("KAR98", r"kar[ _.-]?98(?:k)?"),
    ("MAKAROV", r"makarov|(?:^|[^a-z0-9])pm[ _.-]?(?:pistol|gun)(?:[^a-z0-9]|$)"),
    ("M72", r"(?:^|[^a-z0-9])m[ _.-]?72(?:[^a-z0-9]|$)|m72[ _.-]?law|light[ _.-]?anti[ _.-]?armor"),
    ("ASSAULT_GENERIC", r"scar[ _.-]?(?:l|h)?|hk[ _.-]?416|g36|famas|(?:^|[^a-z0-9])aug(?:[^a-z0-9]|$)|galil|acr|assault[ _.-]?rifle"),
    ("SNIPER_GENERIC", r"sniper|dragunov|(?:^|[^a-z0-9])svd(?:[^a-z0-9]|$)|(?:^|[^a-z0-9])awp(?:[^a-z0-9]|$)|m24|m40|barrett|ballistic[ _.-]?rifle"),
    ("SHOTGUN_GENERIC", r"shotgun|mossberg|benelli|spas[ _.-]?12|saiga[ _.-]?12"),
    ("SMG_GENERIC", r"submachine|(?:^|[^a-z0-9])smg(?:[^a-z0-9]|$)|(?:^|[^a-z0-9])uzi(?:[^a-z0-9]|$)|p90|ump[ _.-]?45|vector"),
    ("PISTOL_GENERIC", r"pistol|handgun|glock|beretta|desert[ _.-]?eagle|deagle|sig[ _.-]?sauer|usp"),
    ("LMG_GENERIC", r"light[ _.-]?machine[ _.-]?gun|machine[ _.-]?gun|(?:^|[^a-z0-9])lmg(?:[^a-z0-9]|$)|rpk|pkm"),
    ("LAUNCHER_GENERIC", r"rocket[ _.-]?launcher|grenade[ _.-]?launcher|bazooka|panzerfaust|at4"),
    ("RIFLE_GENERIC", r"rifle|carbine"),
]

LIVE_WEAPON_CATEGORIES = {
    "M16_M4", "AK47", "AK74", "AR15", "MP5", "M1911", "M700", "M14", "MAC10", "TEC9",
    "LEVER_ACTION", "LAUNCHER", "M249", "REMINGTON870", "BALLISTA", "KAR98", "MAKAROV", "M72",
    "ASSAULT_GENERIC", "SNIPER_GENERIC", "SHOTGUN_GENERIC", "SMG_GENERIC", "PISTOL_GENERIC",
    "LMG_GENERIC", "LAUNCHER_GENERIC", "RIFLE_GENERIC",
}


def classify_weapon(text: str):
    value = text.lower().replace("\\", "/")
    for category, pattern in RULES:
        if re.search(pattern, value):
            return category
    return None


def main():
    if not BINDINGS.is_file():
        raise SystemExit(f"runtime binding manifest missing: {BINDINGS}")

    data = json.loads(BINDINGS.read_text(encoding="utf-8-sig"))
    changed = 0
    resolved_sources = {}
    unresolved = []

    for field in ("static_assets", "skeletal_assets"):
        for entry in data.get(field, []):
            category = str(entry.get("category") or "")
            source = str(entry.get("source") or "")
            path = str(entry.get("path") or "")
            probe = f"{source} {path}"

            # Old intake could call e.g. ak-74m.zip UNCLASSIFIED because the filename contained no
            # generic word such as rifle. Recheck both buckets against the actual source/path now.
            if category in {"WEAPON_OTHER", "UNCLASSIFIED"}:
                mapped = classify_weapon(probe)
                if mapped:
                    entry["category"] = mapped
                    category = mapped
                    changed += 1
                elif category == "WEAPON_OTHER":
                    unresolved.append({
                        "source": source,
                        "asset": path,
                        "category": "WEAPON_OTHER",
                        "status": "UNBOUND",
                        "reason": "weapon_runtime_class_unresolved",
                    })

            if category in LIVE_WEAPON_CATEGORIES:
                resolved_sources[source] = category

    # Make source_status tell the same truth as the runtime manifest. A factual import/load failure
    # is never upgraded from UNBOUND merely because its filename matches a weapon regex.
    for row in data.get("source_status", []):
        source = str(row.get("source") or "")
        row_category = str(row.get("category") or "")
        row_status = str(row.get("status") or "").upper()
        if row_status == "UNBOUND":
            continue
        if source in resolved_sources:
            row["category"] = resolved_sources[source]
            row["status"] = "BOUND"
            row["binding"] = "LIVE_WEAPON_RUNTIME_OVERRIDE"
        elif row_category in {"WEAPON_OTHER", "UNCLASSIFIED"}:
            mapped = classify_weapon(source + " " + str(row.get("asset") or ""))
            if mapped:
                row["category"] = mapped
                row["status"] = "BOUND"
                row["binding"] = "LIVE_WEAPON_RUNTIME_OVERRIDE"
                resolved_sources[source] = mapped
                changed += 1
            elif row_category == "WEAPON_OTHER":
                row["status"] = "UNBOUND"
                row["reason"] = "weapon_runtime_class_unresolved"

    # Preserve unrelated failures, replace only our own weapon-runtime unresolved rows.
    retained = [
        row for row in data.get("unbound_models", [])
        if str(row.get("reason") or "") != "weapon_runtime_class_unresolved"
    ]
    seen = {(str(x.get("source")), str(x.get("asset")), str(x.get("reason"))) for x in retained}
    for row in unresolved:
        key = (str(row.get("source")), str(row.get("asset")), str(row.get("reason")))
        if key not in seen:
            retained.append(row)
            seen.add(key)

    # Independent reconciliation: every source_status row still marked UNBOUND after normalization
    # must remain an acceptance blocker even if an upstream importer forgot to mirror it.
    for row in data.get("source_status", []):
        if str(row.get("status") or "").upper() != "UNBOUND":
            continue
        key = (str(row.get("source")), str(row.get("asset")), str(row.get("reason")))
        if key not in seen:
            retained.append(row)
            seen.add(key)
    data["unbound_models"] = retained

    summary = data.setdefault("summary", {})
    summary["live_weapon_sources"] = len(resolved_sources)
    summary["unresolved_weapon_models"] = len(unresolved)
    summary["unbound_models"] = len(retained)
    data["all_models_bound"] = len(retained) == 0

    BINDINGS.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    if data["all_models_bound"]:
        SUCCESS.write_text("PASS45_LOCAL_INBOX_IMPORT_BINDING=PASS\n", encoding="utf-8")
    elif SUCCESS.exists():
        SUCCESS.unlink()

    print(
        f"LOCAL_WEAPON_NORMALIZE resolved={len(resolved_sources)} "
        f"unresolved={len(unresolved)} changed={changed} all_models_bound={int(data['all_models_bound'])}"
    )
    if unresolved:
        for row in unresolved:
            print(f"UNBOUND_WEAPON source={row['source']} asset={row['asset']}")
        raise SystemExit(21)


if __name__ == "__main__":
    main()
