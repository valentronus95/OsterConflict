#!/usr/bin/env python3
"""Pass45 Gate K regression: landmark scan + Culture House authored-shell retirement."""

from pathlib import Path

ROOT = Path(__file__).resolve().parent
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCVisualFidelityGateKSubsystem.cpp"
CULTURE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR146CultureHousePhotoModelSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 GATE K LANDMARK SCOPE FAIL: {message}")


for path in (GUARD, CULTURE):
    if not path.is_file():
        fail(f"missing {path.relative_to(ROOT)}")

guard = GUARD.read_text(encoding="utf-8", errors="replace")
culture = CULTURE.read_text(encoding="utf-8", errors="replace")

for needle in (
    'MuseumPhotoModelTag(TEXT("R137_MuseumPhotoModel"))',
    'CultureHousePhotoModelTag(TEXT("R146_CultureHouseModel"))',
    'SilpoPhotoModelTag(TEXT("R140_SilpoModel"))',
    'AuthoritativeStadiumTag(TEXT("R13_StadionOsterAuthoritative"))',
    "const bool bMuseum = Actor->ActorHasTag(MuseumPhotoModelTag);",
    "const bool bCultureHouse = Actor->ActorHasTag(CultureHousePhotoModelTag);",
    "const bool bSilpo = Actor->ActorHasTag(SilpoPhotoModelTag);",
    "LandmarkBasicShapeComponents",
    "LandmarkBasicShapeInstances",
    "CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);",
    "reason=landmark_owner_count",
    "stadium=%d museum=%d culture=%d silpo=%d",
    "landmark_basicshape_components=%d",
    "landmark_basicshape_instances=%d",
    "R137_MuseumPhotoModel,R146_CultureHouseModel,R140_SilpoModel",
    "landmark_basicshape_components=0 landmark_basicshape_instances=0",
    "museum_owners=1 culture_owners=1 silpo_owners=1",
):
    if needle not in guard:
        fail(f"missing landmark-scope contract {needle!r}")

stale_stadium_only = 'if (!Actor || !Actor->ActorHasTag(AuthoritativeStadiumTag)) continue;'
if stale_stadium_only in guard:
    fail("stale stadium-only Gate K actor scan was resurrected")

if 'PASS45_GATE_K_RUNTIME_READY' not in guard or 'PASS45_VISUAL_FIDELITY_CONTENT_GAP' not in guard:
    fail("Gate K final READY/CONTENT_GAP markers are incomplete")

# 2026-08-27 runtime rejection: Culture House was still a visible Cube/Cylinder blockout.
# Its authoritative owner must now be built only from committed modular meshes and their authored materials.
for forbidden in (
    "/Engine/BasicShapes/",
    "BasicShapeMaterial",
    "Cube.Cube",
    "Cylinder.Cylinder",
    "MakeColor(",
    "MakeColorMaterial(",
):
    if forbidden in culture:
        fail(f"Culture House resurrected rejected primitive/material path {forbidden!r}")

for needle in (
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Pillar.Wall_Pillar',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Door_01.Door_01',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_Roof_8x4m.Porch_Roof_8x4m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m',
    '/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m',
    'Model->Tags.Add(TEXT("R146_CultureHouseModel"));',
    'PASS45_CULTURE_HOUSE_AUTHORED_SHELL_READY',
    'basicshape_visible=0',
    'basicshape_material=0',
    'PASS45_CULTURE_HOUSE_AUTHORED_SHELL_FAIL',
    'basicshape_fallback=0',
):
    if needle not in culture:
        fail(f"Culture House authored-shell contract missing {needle!r}")

if culture.count('AddFittedAuthoredMesh(Columns,') != 1:
    fail("six-column facade must remain a single data loop over the authored pillar component")
if 'const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };' not in culture:
    fail("Culture House six-column source identity changed unexpectedly")

print("PASS45 GATE K LANDMARK BASICSHAPE SCOPE: PASS")
print("- Museum, Culture House, Silpo and Stadium remain inside final-world BasicShape inspection")
print("- Culture House visible shell now uses committed modular wall/pillar/door/roof/foundation/forecourt assets")
print("- Culture House has no Engine BasicShape mesh/material fallback and fails closed if authored assets are missing")
print("STATUS: SOURCE CONTRACT ONLY; Museum/Silpo authored-shell retirement and local UE 5.8 visual acceptance remain required")
