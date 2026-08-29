#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCVisualFidelityGateKSubsystem.cpp"
RUNTIME_VERIFY = ROOT / "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py"
WORKFLOW = ROOT / ".github" / "workflows" / "pass45-gate-k-global-basicshape-scope.yml"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 GATE K GLOBAL BASICSHAPE SCOPE FAIL: {message}")


for path in (GUARD, RUNTIME_VERIFY, WORKFLOW):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

text = GUARD.read_text(encoding="utf-8", errors="replace")
runtime_verify = RUNTIME_VERIFY.read_text(encoding="utf-8", errors="replace")
workflow = WORKFLOW.read_text(encoding="utf-8", errors="replace")

required = (
    "IsRuntimeVisibleBasicShape",
    "Component->bHiddenInGame",
    "Actor->IsHidden()",
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
        fail(f"runtime observer missing {needle!r}")

# The observer must honor both ways Unreal suppresses runtime rendering: an actor may be hidden as a whole while
# its registered components retain their own visible/hidden flags, and a component may itself be hidden in game.
# Either case is non-rendered content and must not manufacture a Gate K BasicShape failure.
if "if (!Actor || Actor->IsHidden()) return;" not in text:
    fail("actor-level hidden state is not excluded before BasicShape component counting")

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

# Current-head runtime evidence must reject a stale narrow READY marker. Require all zero-count/scope fields on the
# READY line itself, not merely somewhere else in the gameplay log.
runtime_required = (
    'ready_lines = [line for line in text.splitlines() if "PASS45_GATE_K_RUNTIME_READY" in line]',
    "ready_line = ready_lines[-1]",
    '"visible_basicshape_components=0"',
    '"visible_basicshape_instances=0"',
    '"landmark_basicshape_components=0"',
    '"landmark_basicshape_instances=0"',
    '"scope=all_gameplay_actors"',
    '"runtime_visible_only=1"',
    '"hidden_in_game_ignored=1"',
    '"gate_k_complete=1"',
    "Gate K READY line missing current-scope field",
)
for needle in runtime_required:
    if needle not in runtime_verify:
        fail(f"runtime evidence verifier missing current-scope guard {needle!r}")

# Changing either the observer or runtime evidence verifier must rerun this dedicated contract on PR and main.
for path_token in (
    "OsterConflict/Source/OsterConflict/Private/OCVisualFidelityGateKSubsystem.cpp",
    "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py",
    "VERIFY_PASS45_GATE_K_GLOBAL_BASICSHAPE_SCOPE.py",
):
    if workflow.count(path_token) < 2:
        fail(f"workflow does not trigger on both PR/main changes for {path_token}")

print("PASS45 GATE K GLOBAL BASICSHAPE SCOPE PASS: all gameplay actors observed; actor-hidden and component-hidden non-rendered proxies excluded; stale narrow READY logs rejected; observer remains non-mutating")
