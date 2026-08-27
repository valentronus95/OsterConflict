#!/usr/bin/env python3
"""Pass45 Gate K regression: final-world BasicShape scope includes all canonical landmark shell owners."""

from pathlib import Path

ROOT = Path(__file__).resolve().parent
GUARD = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCVisualFidelityGateKSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"PASS45 GATE K LANDMARK SCOPE FAIL: {message}")


if not GUARD.is_file():
    fail(f"missing {GUARD.relative_to(ROOT)}")

guard = GUARD.read_text(encoding="utf-8", errors="replace")

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

print("PASS45 GATE K LANDMARK BASICSHAPE SCOPE: PASS")
print("- Museum, Culture House, Silpo and Stadium are all inside final-world BasicShape inspection")
print("- canonical landmark owner counts are fail-closed before Gate K can report READY")
print("- visible landmark Cube/Cylinder presentation now blocks Gate K instead of false-passing")
print("STATUS: SOURCE CONTRACT ONLY; authored landmark replacement and local UE 5.8 visual acceptance remain required")
