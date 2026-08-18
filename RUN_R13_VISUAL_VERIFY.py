from pathlib import Path
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
ENV = os.environ.copy()
ENV["PYTHONUTF8"] = "1"
ENV["PYTHONIOENCODING"] = "utf-8"

CHECKS = [
    "VERIFY_R13_4_ENVIRONMENT_DRESSING.py",
    "VERIFY_R13_4_FOLIAGE_DIVERSITY.py",
    "VERIFY_R13_4_VISUAL_BATCH_CONSOLIDATION.py",
    "VERIFY_R13_4_RESIDENTIAL_INFILL.py",
    "VERIFY_R13_4_RESIDENTIAL_INFILL_FENCES.py",
    "VERIFY_R13_5_LANDMARK_SITE_DRESSING.py",
    "VERIFY_R13_5_CENTRAL_PARK_DRESSING.py",
    "VERIFY_R13_5_CENTRAL_PARK_CANOPY.py",
    "VERIFY_R13_5_ROADSIDE_INFRASTRUCTURE.py",
    "VERIFY_R13_5_COLLEGE_STADIUM_VISUALS.py",
    "VERIFY_R13_5_MUSEUM_PROTECTION.py",
    "VERIFY_R13_5_VISUAL_ASSET_PATHS.py",
    "VERIFY_R13_OSTER_PROP_ART.py",
    "VERIFY_R13_PARK_FURNITURE.py",
    "VERIFY_R13_LANDMARK_WINDOWS.py",
    "VERIFY_R13_LANDMARK_ROOFS.py",
    "VERIFY_R13_MUSEUM_CHIMNEYS.py",
]

for name in CHECKS:
    path = ROOT / name
    if not path.is_file():
        raise SystemExit(f"R13 VISUAL VERIFY FAIL: missing verifier {name}")
    print(f"===== {name} =====")
    result = subprocess.run([sys.executable, str(path)], cwd=ROOT, env=ENV)
    if result.returncode:
        raise SystemExit(result.returncode)

print("R13 CONSOLIDATED VISUAL VERIFICATION: PASS")
print(f"Executed {len(CHECKS)} environment / landmark / fence / protection / asset-path visual source gates.")
