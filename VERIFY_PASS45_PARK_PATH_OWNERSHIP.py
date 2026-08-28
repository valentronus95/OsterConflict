#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
WORLD_H = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCWorldSectorOster.h"
WORLD_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCWorldSectorOster.cpp"
UPGRADER_CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCAuthoredWorldSurfaceUpgradeSubsystem.cpp"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


world_h = read(WORLD_H)
world_cpp = read(WORLD_CPP)
upgrader_cpp = read(UPGRADER_CPP)

req("TObjectPtr<UInstancedStaticMeshComponent> ParkPaths;" in world_h,
    "AOCWorldSectorOster does not own a canonical ParkPaths component")
req('ParkPaths = MakeISM(TEXT("ParkPaths"), TEXT("BlockAll"));' in world_cpp,
    "ParkPaths is not created as a source-owned default ISM")
req("ExpectedParkPaths = 5" in world_cpp,
    "central-park source contract does not declare exactly five ParkPaths")
req(world_cpp.count("AddBox(ParkPaths,") == 5,
    f"expected exactly five AddBox(ParkPaths, ...) calls, found {world_cpp.count('AddBox(ParkPaths,')}")

# These are the four central alleys plus the link to the north civic grove. They must not leak back into Sidewalks.
for forbidden in (
    "AddBox(Sidewalks, Park + FVector(0, 0, 14), FVector(17800, 360, 18));",
    "AddBox(Sidewalks, Park + FVector(0, -300, 14), FVector(360, 13200, 18));",
    "AddBox(Sidewalks, Park + FVector(1800, 900, 14), FVector(11800, 260, 18), 31.0f);",
    "AddBox(Sidewalks, Park + FVector(-2300, 1300, 14), FVector(9300, 240, 18), -28.0f);",
    "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);",
):
    req(forbidden not in world_cpp, f"park path leaked back into Sidewalks: {forbidden}")

for needle in (
    "SM_Stonepath_Var01.SM_Stonepath_Var01",
    "ExpectedParkPathCount = 5",
    "ExistingParkPaths->GetInstanceCount() != 5",
    "RemainingInSidewalks != 0",
    "UpgradeCubeFamily(ParkPaths, ParkPathMesh",
    "PASS45_AUTHORED_PARK_PATH_SURFACE_READY",
):
    req(needle in upgrader_cpp, f"bounds-aware authored ParkPaths upgrade contract missing: {needle}")

# Tactical map must not depend on the Sidewalks ISM family; moving these five paths therefore cannot erase topology.
tactical = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCTacticalMapVisual.cpp"
tactical_cpp = read(tactical)
req("do NOT read Roads/Sidewalks/Buildings/LandmarkBlocks ISM families here" in tactical_cpp,
    "tactical-map independence from world blockout ISMs is no longer explicit")

if errors:
    print("PASS45 PARK PATH OWNERSHIP: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 PARK PATH OWNERSHIP: PASS")
print("- AOCWorldSectorOster source-owns a dedicated ParkPaths ISM")
print("- exactly five central-park path proxies are authored into ParkPaths, not Sidewalks")
print("- existing bounds-aware upgrader maps ParkPaths to SM_Stonepath_Var01 and rejects overlap/count drift")
print("- tactical topology is independent of Sidewalks ISM ownership")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 runtime visual acceptance remains required")
