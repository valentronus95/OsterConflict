#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCVisualFidelityGateKSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 GATE K GLOBAL BASICSHAPE SCOPE FAIL: {message}")


if not GUARD.is_file():
    fail(f"missing {GUARD.relative_to(ROOT)}")

text = GUARD.read_text(encoding="utf-8", errors="replace")

required = (
    "IsRuntimeVisibleBasicShape",
    "Component->bHiddenInGame",
    "for (TActorIterator<AActor> It(World); It; ++It)",
    "CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);",
    "scope=all_gameplay_actors",
    "runtime_visible_only=1",
    "hidden_in_game_ignored=1",
    "PASS45_VISUAL_FIDELITY_CONTENT_GAP",
    "PASS45_GATE_K_RUNTIME_FAIL",
    "PASS45_GATE_K_RUNTIME_READY",
)
for needle in required:
    if needle not in text:
        fail(f"missing {needle!r}")

# The final observer must not mutate scenery to manufacture a pass.
for forbidden in ("SetVisibility(false", "SetHiddenInGame(true", "DestroyComponent"):
    if forbidden in text:
        fail(f"runtime observer mutates presentation via {forbidden!r}")

# Regression: the old scope counted only the Oster sector and four tagged landmarks. The sector iterator may still
# validate unique ownership, but it must not be the place where BasicShapes are counted.
if "CountVisibleBasicShapes(*It, BasicShapeComponents" in text:
    fail("BasicShape count regressed to sector-only scan")

# One generic actor scan is enough to cover characters, weapons, ordnance, vehicles and future gameplay owners.
if text.count("CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);") != 1:
    fail("global actor BasicShape observation must have exactly one counting site")

print("PASS45 GATE K GLOBAL BASICSHAPE SCOPE PASS: all gameplay actors observed; hidden-in-game collision/proxy components excluded; observer remains non-mutating")
