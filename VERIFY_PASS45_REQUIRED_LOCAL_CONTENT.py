#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
CONTENT = PROJECT / "Content"
EVIDENCE = ROOT / "Logs" / "PASS45_REQUIRED_CONTENT_PREFLIGHT.txt"

REQUIRED_LOCAL_PRODUCTION_WEAPONS = (
    (
        "Remington 870",
        CONTENT / "Production" / "Weapons" / "Remington870" / "SM_Remington870.uasset",
        "/Game/Production/Weapons/Remington870/SM_Remington870",
    ),
    (
        "M249",
        CONTENT / "Production" / "Weapons" / "M249" / "SM_M249.uasset",
        "/Game/Production/Weapons/M249/SM_M249",
    ),
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--report-only",
        action="store_true",
        help="Report current gaps without failing. Intended for source CI where known binary content gaps are explicit.",
    )
    args = parser.parse_args()

    missing = []
    present = []
    for label, file_path, object_path in REQUIRED_LOCAL_PRODUCTION_WEAPONS:
        if file_path.is_file() and file_path.stat().st_size > 0:
            present.append((label, file_path, object_path))
        else:
            missing.append((label, file_path, object_path))

    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "PASS45_REQUIRED_CONTENT_PREFLIGHT=PASS" if not missing else "PASS45_REQUIRED_CONTENT_PREFLIGHT=FAIL",
        f"REQUIRED_WEAPONS={len(REQUIRED_LOCAL_PRODUCTION_WEAPONS)}",
        f"PRESENT={len(present)}",
        f"MISSING={len(missing)}",
    ]
    for label, file_path, object_path in present:
        lines.append(f"READY_FOR_UE_VALIDATION={label}|{object_path}|{file_path.relative_to(ROOT)}")
    for label, file_path, object_path in missing:
        lines.append(f"CONTENT_GAP={label}|{object_path}|{file_path.relative_to(ROOT)}")
    EVIDENCE.write_text("\n".join(lines) + "\n", encoding="utf-8")

    if missing:
        print("PASS45 REQUIRED LOCAL CONTENT: CONTENT GAP")
        for label, file_path, object_path in missing:
            print(f"[CONTENT GAP] {label}")
            print(f"  canonical UE path: {object_path}")
            print(f"  required local file: {file_path}")
        print("Evidence:", EVIDENCE)
        if args.report_only:
            print("REPORT-ONLY: known content gaps remain explicit; source CI may continue.")
            return 0
        print("STRICT ACCEPTANCE BLOCKED: do not run final Pass45 acceptance until legitimate authored assets exist locally.")
        return 21

    print("PASS45 REQUIRED LOCAL CONTENT: PASS")
    for label, _, object_path in present:
        print(f"- {label}: {object_path}")
    print("This proves file presence only. UE 5.8 material/runtime/visual validation is still required.")
    print("Evidence:", EVIDENCE)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
