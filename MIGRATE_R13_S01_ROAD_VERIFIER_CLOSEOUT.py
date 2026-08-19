from pathlib import Path

ROOT = Path(__file__).resolve().parent
VERIFY = ROOT / "VERIFY_R13_LOCATION_FIRST_S01_ROAD_TOPOLOGY.py"

old_runtime = '''if "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);" not in world:
    fail("remaining Park->NorthCivic shared path changed before its path split audit")
'''
new_runtime = '''if "AddBox(Sidewalks, Mid + FVector(0,0,15), FVector(Delta.Size2D(), 260, 18), LinkYaw);" in world:
    fail("legacy derived Park->NorthCivic sidewalk returned after explicit ownership split")
if "FOCLocationSectorS01RoadData::ParkNorthCivicPathSegments()" not in world:
    fail("runtime no longer consumes the explicit Park->NorthCivic path split manifest")
'''

old_print = '''print("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY: PASS")
print(
    f"S01 bounds approx X[{xmin:.1f},{xmax:.1f}] Y[{ymin:.1f},{ymax:.1f}] cm; "
    "all 7 audited BuildRoadNetwork crossings are explicit continuity-preserving ownership splits; "
    "Park->NorthCivic is the only remaining unsplit crossing path."
)
'''
new_print = '''print("R13 LOCATION-FIRST S01 ROAD TOPOLOGY VERIFY: PASS")
print(
    f"S01 bounds approx X[{xmin:.1f},{xmax:.1f}] Y[{ymin:.1f},{ymax:.1f}] cm; "
    "all 7 audited BuildRoadNetwork crossings are explicit continuity-preserving ownership splits; "
    "Park->NorthCivic runtime ownership is explicitly split and validated by the dedicated path verifier."
)
'''

old_comment = "# Park -> CultureParkNorth is the only remaining unsplit path crossing."
new_comment = "# Preserve the original Park -> CultureParkNorth crossing classification; runtime ownership is split separately."

text = VERIFY.read_text(encoding="utf-8")
changed = False

for old, new, label in [
    (old_runtime, new_runtime, "runtime contract"),
    (old_print, new_print, "status output"),
    (old_comment, new_comment, "classification comment"),
]:
    if old in text:
        text = text.replace(old, new, 1)
        changed = True
    elif new not in text:
        raise SystemExit(f"S01 ROAD VERIFIER CLOSEOUT FAIL: unexpected {label} state")

if changed:
    VERIFY.write_text(text, encoding="utf-8")
    print("S01 ROAD VERIFIER CLOSEOUT: PASS")
else:
    print("S01 ROAD VERIFIER CLOSEOUT: ALREADY APPLIED")
