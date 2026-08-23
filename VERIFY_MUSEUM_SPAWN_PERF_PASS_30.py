from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
TEAM = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCTeamSpawnPoint.cpp").read_text(encoding="utf-8")
GUARD = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCMuseumSpawnGuardSubsystem.cpp").read_text(encoding="utf-8")
WINDOW = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCMuseumBreakableWindow.cpp").read_text(encoding="utf-8")
R137 = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR137MuseumPhotoModelSubsystem.cpp").read_text(encoding="utf-8")
R138 = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR138MuseumInteractiveArchitectureSubsystem.cpp").read_text(encoding="utf-8")
R143 = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR143MuseumSiteVegetationSubsystem.cpp").read_text(encoding="utf-8")
R145 = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR145MuseumTreeLayoutSubsystem.cpp").read_text(encoding="utf-8")
FOLIAGE = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCDenseGroundFoliageSubsystem.cpp").read_text(encoding="utf-8")

errors = []

def need(text, token, label):
    if token not in text:
        errors.append(f"missing {label}: {token}")

def forbid(text, token, label):
    if token in text:
        errors.append(f"forbidden {label}: {token}")

# BASE must live outside the museum shell, not at MuseumAnchor or the old 14.5 m offsets.
for token in (
    "FVector(-2600.0f, -3200.0f, 120.0f)",
    "FVector(2600.0f, -3200.0f, 120.0f)",
    "PASS30_BASE_RELOCATED_OUTSIDE_MUSEUM",
):
    need(TEAM, token, "exterior canonical BASE")
forbid(TEAM, "FVector(-1450.0f, -900.0f, 120.0f)", "old TeamOne interior-adjacent BASE")
forbid(TEAM, "FVector(1450.0f, 900.0f, 120.0f)", "old TeamTwo interior-adjacent BASE")

need(GUARD, "constexpr float MuseumNoSpawnRadiusCm = 3000.0f;", "museum no-spawn radius")
need(GUARD, "bOutsideMuseum && bNearMuseum", "deployment exterior acceptance")
need(GUARD, "PASS30_BASE_DEPLOYMENT_RECOVERED_OUTSIDE_MUSEUM", "interior deployment recovery marker")
need(GUARD, "PASS30_MUSEUM_EXTERIOR_BASES_READY", "exterior bases readiness marker")
forbid(GUARD, "World->SpawnActor<AOCTeamSpawnPoint>(\n            AOCTeamSpawnPoint::StaticClass(), Museum, FRotator::ZeroRotator, SpawnParams);\n        if (!Point)", "unconfigured museum-center BASE persistence")

# The screenshot's stretched rustic frame must not return.
forbid(WINDOW, "Window_Frame_Part.Window_Frame_Part", "distorted authored cabin frame")
forbid(WINDOW, "FitAuthoredFramePart", "axis-stretched frame fitter")
need(WINDOW, "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY", "clean window marker")
need(WINDOW, "Component->SetCastShadow(false);", "window shadow budget")

# Remove unsupported pale interior slabs and reduce duplicate shell/detail cost.
need(R137, "constexpr float SourceMuseumCleanupRadiusCm = 5000.0f;", "wider source landmark cleanup")
for token in ("InteriorPartitionLeft", "InteriorPartitionRight", "InteriorPartitionHeader"):
    forbid(R138, token, "unsupported interior partition")
need(R138, "PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED", "interior cleanup marker")
need(R138, "Component->SetCastShadow(false);", "museum architecture shadow budget")
need(R138, "Component->SetCullDistances(0, 30000);", "museum detail cull budget")

# FPS recovery budgets.
grid = re.search(r"constexpr float GridStep = ([0-9.]+)f;", FOLIAGE)
batch = re.search(r"constexpr int32 CellsPerBatch = (\d+);", FOLIAGE)
if not grid or float(grid.group(1)) < 4000.0:
    errors.append("dense foliage grid must be >= 4000 cm for Pass 30")
if not batch or int(batch.group(1)) > 4:
    errors.append("dense foliage batch must be <= 4 cells for Pass 30")
need(FOLIAGE, "PASS30_FOLIAGE_BUDGET_READY", "Pass 30 foliage runtime marker")
need(R145, "Component->SetCastShadow(false);", "museum tree shadow budget")
need(R145, "Component->SetCanEverAffectNavigation(false);", "museum tree nav budget")
for cull in ("R143Museum_GrassA\"), 15000", "R143Museum_GroundPlantA\"), 12000"):
    need(R143, cull, "museum vegetation cull budget")

if errors:
    print("MUSEUM SPAWN/PERF PASS 30: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("MUSEUM SPAWN/PERF PASS 30: PASS")
print("- BASE is outside the museum exclusion footprint and interior deployments are recovered")
print("- distorted museum window frame and unsupported interior slabs are removed")
print("- museum/global foliage, shadows and navigation work are reduced")
print("STATUS: SOURCE CONTRACT ONLY; UE 5.8 runtime FPS/movement acceptance still required")
