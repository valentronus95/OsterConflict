#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCCombatVisualComponent.h"
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCCombatVisualComponent.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 COMBAT DISMEMBERMENT PROXY RETIREMENT FAIL: {message}")


for path in (HEADER, CPP):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

header = HEADER.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

# Gameplay trauma/ragdoll remains intact, but rejected cylinder limb art must be fail-closed by default until an
# authored detached-limb presentation exists.
for needle in (
    "bool bAllowDismemberment = false;",
    "legacy detached-limb path still uses an Engine BasicShape Cylinder",
    "Authoritative damage, trauma severity and ragdoll behavior remain active",
):
    if needle not in header:
        fail(f"header missing fail-closed contract {needle!r}")

if "bool bAllowDismemberment = true;" in header:
    fail("production default re-enabled rejected dismemberment proxy")

for needle in (
    "if (!bAllowDismemberment || !Event.bFatal || Event.BloodSeverity != EOCBloodSeverity::Extreme)",
    "Mesh->SetAllBodiesSimulatePhysics(true);",
    "Mesh->SetSimulatePhysics(true);",
    "/Engine/BasicShapes/Cylinder.Cylinder",
):
    if needle not in cpp:
        fail(f"combat source contract changed unexpectedly: missing {needle!r}")

# This is deliberate fail-honesty: the legacy proxy code can remain as a dev/content-gap path, but current
# production defaults may not render it. Do not pretend this closes authored gore content.
print("PASS45 COMBAT DISMEMBERMENT PROXY RETIREMENT PASS: rejected Cylinder limb chunks are disabled by production default; trauma/ragdoll stays active; authored detached-limb content remains a content gap")
