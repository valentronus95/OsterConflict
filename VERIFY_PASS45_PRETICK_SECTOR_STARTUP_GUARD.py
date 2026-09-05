#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CONFIG = ROOT / "OsterConflict" / "Config" / "DefaultEngine.ini"
RUNTIME_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCGameModeRuntimeSafe.h"
RUNTIME_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCGameModeRuntimeSafe.cpp"
BLOCK0_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCBlock0GroundFoundationSubsystem.cpp"
STADIUM_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13StadiumSurfaceSubsystem.h"
STADIUM_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13StadiumSurfaceSubsystem.cpp"
LAUNCHER = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 PRETICK SECTOR STARTUP GUARD FAIL: {message}")


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


config = read(CONFIG)
runtime_h = read(RUNTIME_H)
runtime_cpp = read(RUNTIME_CPP)
block0_cpp = read(BLOCK0_CPP)
stadium_h = read(STADIUM_H)
stadium_cpp = read(STADIUM_CPP)
launcher = read(LAUNCHER)

for needle in (
    "GlobalDefaultGameMode=/Script/OsterConflict.OCGameModeRuntimeSafe",
    "GlobalDefaultServerGameMode=/Script/OsterConflict.OCGameModeRuntimeSafe",
):
    if needle not in config:
        fail(f"runtime-safe GameMode is not the configured startup owner: {needle!r}")

if "virtual void BeginPlay() override;" not in runtime_h:
    fail("runtime-safe GameMode lost the pre-first-tick duplicate-retirement override")

try:
    init_section = runtime_cpp.split("void AOCGameModeRuntimeSafe::InitGame", 1)[1].split(
        "void AOCGameModeRuntimeSafe::BeginPlay", 1
    )[0]
    begin_section = runtime_cpp.split("void AOCGameModeRuntimeSafe::BeginPlay", 1)[1].split(
        "void AOCGameModeRuntimeSafe::RestartPlayer", 1
    )[0]
except IndexError:
    fail("could not isolate runtime-safe InitGame/BeginPlay lifecycle")

for needle in (
    'Pass45PreTickOsterSectorTag(TEXT("PASS45_PreTickOsterSector"))',
    "GatherLiveOsterSectors(GetWorld(), ExistingSectors)",
    "SpawnActor<AOCWorldSectorOster>",
    "PASS45_PRETICK_OSTER_SECTOR_READY",
    "sector_count=1",
    "authored_before_world_begin_play=1",
    "heavy_tree_startup_loads=0",
    "runtime_acceptance=0",
):
    if needle not in init_section and needle not in runtime_cpp:
        fail(f"pre-tick Oster sector bootstrap lost {needle!r}")

if "SpawnActor<AOCWorldSectorOster>" not in init_section:
    fail("AOCWorldSectorOster is no longer authored during InitGame before UWorldSubsystem BeginPlay")

for needle in (
    "Super::BeginPlay();",
    "PASS45_OSTER_SECTOR_SINGLE_OWNER_READY",
    "CanonicalSector",
    "Sector->Destroy();",
    "sector_count=1",
    "before_first_tick=1",
    "heavy_tree_startup_loads=0",
    "runtime_acceptance=0",
):
    if needle not in begin_section:
        fail(f"first-tick single-sector retirement contract lost {needle!r}")

for needle in (
    "PASS45_BLOCK0_PRETICK_GROUND_FAIL",
    "oster_sector_count_%d",
    "ApplyAuthoredGroundBeforeFirstTick",
    "PASS45_BLOCK0_PRETICK_GROUND_READY",
):
    if needle not in block0_cpp:
        fail(f"Block0 pre-tick ground gate lost fail-closed sector contract {needle!r}")

if "UCLASS(Abstract)" not in stadium_h:
    fail("runtime-rejected stadium WorldSubsystem can auto-instantiate again during gameplay START")
if "PASS45_STADIUM_GAMEPLAY_START_QUARANTINE" not in stadium_h:
    fail("stadium startup quarantine lost explicit runtime-rejected rationale")

for tree_path in (
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
):
    if tree_path not in stadium_cpp:
        fail(f"guard can no longer prove the quarantined stadium dependency exists: {tree_path}")

if "void UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay" not in stadium_cpp:
    fail("quarantined stadium implementation unexpectedly disappeared instead of staying explicit")
if "LoadObject<UStaticMesh>" not in stadium_cpp:
    fail("quarantined stadium sync-load implementation is no longer detectable")

if "Pass45LoadKiteDemoTrees" in launcher:
    fail("canonical gameplay launcher opted back into the rejected KiteDemo tree load path")

print("PASS45 PRETICK SECTOR STARTUP GUARD PASS")
print("- OCGameModeRuntimeSafe is the configured startup GameMode for game and server")
print("- one lightweight AOCWorldSectorOster exists during InitGame before UWorldSubsystem::OnWorldBeginPlay")
print("- the legacy base-GameMode sector duplicate is retired before the first gameplay tick")
print("- Block0 retains explicit oster_sector_count fail-closed evidence and can now see the pre-tick sector")
print("- the second START-time stadium HillTree_02/ScotsPineTall_01 sync-load owner is abstract/quarantined")
print("- canonical launcher does not opt into deferred KiteDemo trees")
print("STATUS: SOURCE RECOVERY ONLY; factual UE 5.8 runtime remains REJECTED until a current-head run passes")
