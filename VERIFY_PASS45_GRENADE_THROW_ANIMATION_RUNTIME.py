#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
DEFAULT_GAMEPLAY_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"


def main() -> int:
    gameplay_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_GAMEPLAY_LOG
    if not gameplay_path.is_file():
        print(f"PASS45 GRENADE THROW ANIMATION RUNTIME: FAIL: missing gameplay log: {gameplay_path}")
        return 2

    gameplay = gameplay_path.read_text(encoding="utf-8", errors="replace")
    errors: list[str] = []

    if "PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY" not in gameplay:
        errors.append("factual grenade throw presentation bridge was not exercised")
    if "PASS45_GRENADE_THROW_AUDIO_RUNTIME_READY" not in gameplay:
        errors.append("missing authored grenade throw audio runtime evidence")
    if "PASS45_GRENADE_THROW_AUDIO_CONTENT_GAP" in gameplay:
        errors.append("authored grenade throw audio failed to load")
    if "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_CONTENT_GAP" in gameplay:
        errors.append("authored first-person grenade hand/throw/recover animation is still a content gap")
    if "PASS45_GRENADE_THROW_AUTHORED_ANIMATION_RUNTIME_READY" not in gameplay:
        errors.append("missing accepted authored grenade throw animation runtime evidence")

    if errors:
        print("PASS45 GRENADE THROW ANIMATION RUNTIME: FAIL")
        for error in errors:
            print("[FAIL]", error)
        print("- gameplay throw semantics may be valid, but item 24 cannot pass on a Blueprint/event bridge alone")
        return 1

    print("PASS45 GRENADE THROW ANIMATION RUNTIME: PASS")
    print("- factual grenade throw exercised the presentation bridge")
    print("- authored throw audio loaded and played from the replicated presentation event")
    print("- authored hand/throw/recover animation emitted runtime READY evidence")
    print("- no grenade throw animation content-gap marker was observed")
    print("STATUS: AUTOMATED ANIMATION EVIDENCE PASS; direct UE 5.8 visual acceptance is still required")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
