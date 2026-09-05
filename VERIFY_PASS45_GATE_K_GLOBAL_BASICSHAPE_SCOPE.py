#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCVisualFidelityGateKSubsystem.h"
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCVisualFidelityGateKSubsystem.cpp"
RUNTIME_VERIFY = ROOT / "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py"
WORKFLOW = ROOT / ".github" / "workflows" / "pass45-gate-k-global-basicshape-scope.yml"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 GATE K GLOBAL BASICSHAPE SCOPE FAIL: {message}")


for path in (HEADER, GUARD, RUNTIME_VERIFY, WORKFLOW):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
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
    "PASS45_GATE_K_RUNTIME_WATCH_ACTIVE",
    "ContinuousObservationIntervalSeconds = 2.0f",
    "if (!bReadyLogged && ElapsedSeconds < 3.0f) return;",
    "if (bReadyLogged && ElapsedSeconds < NextObservationSeconds) return;",
    "NextObservationSeconds = ElapsedSeconds + ContinuousObservationIntervalSeconds;",
    "continuous_watch=1",
    "late_spawn_detection=1",
    "scan_interval_seconds=2.0",
)
for needle in required:
    if needle not in text:
        fail(f"runtime observer missing {needle!r}")

for needle in (
    "NextObservationSeconds",
    "bReadyLogged",
    "low-frequency observation watch active",
):
    if needle not in header:
        fail(f"runtime observer header missing continuous-watch contract {needle!r}")

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

# A clean first scan may not stop the subsystem. The old implementation set bFinished immediately before the
# BasicShape check and therefore never observed a weapon/grenade/vehicle spawned later in gameplay.
old_one_shot = "bFinished = true;\n    if (BasicShapeComponents > 0)"
if old_one_shot in text:
    fail("Gate K regressed to one-shot completion before late gameplay spawns")
if "if (!bReadyLogged)" not in text:
    fail("initial READY is not separated from the continuing watch")

# Current-head runtime evidence must reject a stale narrow or one-shot READY marker. Require all zero-count/scope
# and continuous-watch fields on the READY line itself, plus a separate WATCH_ACTIVE marker.
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
    '"continuous_watch=1"',
    '"late_spawn_detection=1"',
    '"scan_interval_seconds=2.0"',
    '"gate_k_complete=1"',
    'watch_lines = [line for line in text.splitlines() if "PASS45_GATE_K_RUNTIME_WATCH_ACTIVE" in line]',
    "Gate K WATCH line missing current-watch field",
    "Gate K READY line missing current-scope field",
)
for needle in runtime_required:
    if needle not in runtime_verify:
        fail(f"runtime evidence verifier missing current-scope/watch guard {needle!r}")

# Changing the header, observer or runtime evidence verifier must rerun this dedicated contract on PR and main.
for path_token in (
    "OsterConflict/Source/OsterConflict/Public/OCVisualFidelityGateKSubsystem.h",
    "OsterConflict/Source/OsterConflict/Private/OCVisualFidelityGateKSubsystem.cpp",
    "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py",
    "VERIFY_PASS45_GATE_K_GLOBAL_BASICSHAPE_SCOPE.py",
):
    if workflow.count(path_token) < 2:
        fail(f"workflow does not trigger on both PR/main changes for {path_token}")

print("PASS45 GATE K GLOBAL BASICSHAPE SCOPE PASS: all gameplay actors observed continuously after startup; late-spawned visible BasicShapes invalidate the run; hidden non-rendered proxies excluded; stale one-shot READY logs rejected; observer remains non-mutating")
