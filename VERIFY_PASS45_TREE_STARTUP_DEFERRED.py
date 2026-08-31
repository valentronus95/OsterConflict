#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
WORLD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 TREE STARTUP DEFERRED FAIL: {message}")


source = WORLD.read_text(encoding="utf-8", errors="replace")
try:
    constructor = source.split("AOCWorldSectorOster::AOCWorldSectorOster()", 1)[1].split(
        "void AOCWorldSectorOster::BeginPlay()", 1
    )[0]
except IndexError:
    fail("could not isolate AOCWorldSectorOster native constructor")

paths = (
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
)
for path in paths:
    if path in constructor:
        fail(f"KiteDemo tree path is still synchronously reachable from constructor/CDO: {path}")
    if path not in source:
        fail(f"exact tree identity disappeared instead of being safely deferred: {path}")

for forbidden in (
    "FObjectFinder<UStaticMesh> DeciduousTreeMesh",
    "FObjectFinder<UStaticMesh> Pine01Mesh",
    "FObjectFinder<UStaticMesh> Pine03Mesh",
):
    if forbidden in source:
        fail(f"synchronous tree finder survived: {forbidden}")

for required in (
    '#include "Engine/AssetManager.h"',
    '#include "Engine/StreamableManager.h"',
    'FParse::Param(FCommandLine::Get(), TEXT("Pass45LoadKiteDemoTrees"))',
    "RequestAsyncLoad(",
    "ResolveObject()",
    "PASS45_TREE_STARTUP_DEFERRED_READY",
    "PASS45_KITEDEMO_TREE_STARTUP_QUARANTINED",
    "PASS45_KITEDEMO_TREE_ASYNC_LOAD_REQUESTED",
    "PASS45_KITEDEMO_TREE_ASYNC_LOAD_READY",
    "startup_sync_tree_loads=0",
    "material_compatibility=pending",
    "runtime_acceptance=0",
):
    if required not in source:
        fail(f"startup-deferred tree contract missing {required!r}")

veg = source.split("void AOCWorldSectorOster::BuildVegetation()", 1)[1]
for required in (
    "const bool bTreeMeshesReady",
    "if (!bTreeMeshesReady)",
    "ground_cover_only=1",
    "tree_bounds_access=0",
    "return;",
):
    if required not in veg:
        fail(f"BuildVegetation no longer protects constructor-time tree bounds: {required!r}")

if veg.find("if (!bTreeMeshesReady)") > veg.find("AddGroundedTree(Component"):
    fail("tree readiness guard occurs after authored-tree bounds/instance path")

if "LoadObject<UStaticMesh>" in source and any(path in source for path in paths):
    # Exact paths are permitted only as FSoftObjectPath values; reject any direct static-mesh LoadObject use.
    for line in source.splitlines():
        if "LoadObject<UStaticMesh>" in line and "KiteDemo" in line:
            fail("direct synchronous LoadObject for KiteDemo tree returned")

print("PASS45 TREE STARTUP DEFERRED PASS")
print("- native constructor/CDO no longer contains HillTree_02/ScotsPine synchronous object finders")
print("- normal runtime quarantines the rejected UE 5.8 tree compile path before first-frame startup")
print("- exact assets remain available only through explicit opt-in FStreamableManager async loading")
print("- BuildVegetation constructs ground-cover zoning without touching tree bounds until meshes are loaded")
print("STATUS: SOURCE STARTUP RECOVERY ONLY; UE 5.8 Quick Normal rerun is still mandatory")
