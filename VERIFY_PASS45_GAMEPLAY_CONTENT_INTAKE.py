from pathlib import Path
import csv
import sys

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
CONTENT = PROJECT / "Content"
RECEIPT = PROJECT / "SourceAssets" / "ThirdParty" / "Gameplay" / "LOCAL_CONTENT_RECEIPT.csv"

required_dirs = [
    CONTENT / "AK-47",
    CONTENT / "Fire_EXP_Vol01_Free" / "Niagara",
    CONTENT / "KiteDemo" / "Environments",
    CONTENT / "Mega_Street_Props_Pack",
    CONTENT / "Megaplant_Library",
    CONTENT / "PN_FoliageCollection",
    CONTENT / "PotaVFX_Smoke",
    CONTENT / "SampleAnimationPack",
    CONTENT / "VehicleVarietyPack",
]

sandbag_root = CONTENT / "Fab" / "Megascans" / "3D"
errors = []

for path in required_dirs:
    if not path.exists():
        errors.append(f"missing required intake path: {path.relative_to(ROOT)}")

if not sandbag_root.exists() or not any("Sandbag" in p.name for p in sandbag_root.iterdir() if p.is_dir()):
    errors.append("missing Fab Megascans military sandbag intake")

if not RECEIPT.exists():
    errors.append("missing LOCAL_CONTENT_RECEIPT.csv")
else:
    with RECEIPT.open("r", encoding="utf-8-sig", newline="") as fh:
        rows = list(csv.DictReader(fh))
    if not rows:
        errors.append("LOCAL_CONTENT_RECEIPT.csv is empty")
    else:
        required_receipt_fragments = [
            "FPSArms3D/",
            "HardLines/Footsteps/",
            "HardLines/Shotgun/",
            "BorderRun/VehicleEngine/",
        ]
        normalized = [str(row.get("RelativePath", "")).replace("\\", "/") for row in rows]
        for fragment in required_receipt_fragments:
            if not any(fragment in value for value in normalized):
                errors.append(f"receipt missing category: {fragment}")

if (ROOT / "30").exists():
    errors.append("stray root scratch file '30' must not exist")

if errors:
    print("PASS45_GAMEPLAY_CONTENT_INTAKE_FAIL runtime_acceptance=0")
    for error in errors:
        print(f"- {error}")
    sys.exit(1)

print("PASS45_GAMEPLAY_CONTENT_INTAKE_SOURCE_READY acquired=1 provenance_receipt=1 runtime_acceptance=0")
print("NOTE: source acquisition readiness does not imply migration, runtime visual acceptance, or shipping readiness.")
