import hashlib
import json
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parents[1]
STATE_ROOT = PROJECT_DIR / "Saved" / "LocalModelInbox"
PREPARED = STATE_ROOT / "prepared_sources.json"
DUPLICATES = STATE_ROOT / "duplicate_sources.json"


def digest_file(path: Path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def dedupe_rows(rows, source_key="source"):
    kept = []
    duplicates = []
    seen = {}

    for row in rows or []:
        source = Path(str(row.get(source_key) or ""))
        if not source.is_file():
            kept.append(row)
            continue

        try:
            sha256 = digest_file(source)
        except Exception:
            kept.append(row)
            continue

        key = (sha256, source.suffix.lower())
        if key in seen:
            duplicate = dict(row)
            duplicate["sha256"] = sha256
            duplicate["duplicate_of"] = seen[key]
            duplicates.append(duplicate)
            continue

        seen[key] = str(source)
        enriched = dict(row)
        enriched["sha256"] = sha256
        kept.append(enriched)

    return kept, duplicates


def main():
    if not PREPARED.is_file():
        raise SystemExit(f"prepared manifest missing: {PREPARED}")

    data = json.loads(PREPARED.read_text(encoding="utf-8-sig"))
    report = {
        "schema": "oster-conflict-local-source-dedupe-v1",
        "duplicate_count": 0,
        "duplicates": [],
    }

    for field in ("raw_models", "hud_images"):
        kept, duplicates = dedupe_rows(data.get(field, []))
        data[field] = kept
        for row in duplicates:
            row["field"] = field
        report["duplicates"].extend(duplicates)

    report["duplicate_count"] = len(report["duplicates"])
    data["exact_duplicate_sources_skipped"] = report["duplicate_count"]

    PREPARED.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    DUPLICATES.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(
        f"LOCAL_ASSET_DEDUPE kept_raw={len(data.get('raw_models', []))} "
        f"kept_hud={len(data.get('hud_images', []))} skipped_exact_duplicates={report['duplicate_count']}"
    )


if __name__ == "__main__":
    main()
