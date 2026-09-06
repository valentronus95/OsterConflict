#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DEFAULT_GAMEPLAY_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"


def main() -> int:
    gameplay_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_GAMEPLAY_LOG
    if not gameplay_path.is_file():
        print(f"PASS45 FLASH WORLD VFX RUNTIME: FAIL: missing gameplay log: {gameplay_path}")
        return 2

    gameplay = gameplay_path.read_text(encoding="utf-8", errors="replace")
    errors: list[str] = []

    if "PASS45_FLASH_WORLD_VFX_RUNTIME_READY" not in gameplay:
        errors.append("missing distinct authored flash-grenade world VFX runtime evidence")
    for forbidden in (
        "PASS45_FLASH_WORLD_VFX_CONTENT_GAP",
        "PASS45_FLASH_WORLD_VFX_LOAD_FAIL",
        "PASS45_FLASH_WORLD_VFX_RUNTIME_FAIL",
    ):
        if forbidden in gameplay:
            errors.append(f"flash-grenade presentation failure observed: {forbidden}")

    if errors:
        print("PASS45 FLASH WORLD VFX RUNTIME: FAIL")
        for error in errors:
            print("[FAIL]", error)
        print("- gameplay flash/LOS semantics may remain functional, but item 24 requires distinct visible frag/smoke/flash presentation")
        return 1

    print("PASS45 FLASH WORLD VFX RUNTIME: PASS")
    print("- distinct authored flash-grenade world presentation emitted runtime READY evidence")
    print("- no flash VFX content/load/runtime failure marker was observed")
    print("STATUS: AUTOMATED FLASH PRESENTATION EVIDENCE PASS; direct UE 5.8 visual acceptance is still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
