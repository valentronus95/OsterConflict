from pathlib import Path
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
ENV = os.environ.copy()
ENV["PYTHONUTF8"] = "1"
ENV["PYTHONIOENCODING"] = "utf-8"

CHECKS = [
    "VERIFY_R13_FRONTEND_MENU_GUARD.py",
    "VERIFY_R13_TICKABLE_SUBSYSTEM_LINKAGE.py",
    "VERIFY_R13_1_STABILIZATION.py",
    "VERIFY_R13_3_DEPLOYMENT_SPAWN.py",
    "VERIFY_R13_3_DEPLOYMENT_RECONCILIATION.py",
    "VERIFY_R13_RUNTIME_SPAWN_BRIDGES.py",
    "VERIFY_R13_RUNTIME_REGRESSIONS.py",
]

for name in CHECKS:
    path = ROOT / name
    if not path.is_file():
        raise SystemExit(f"R13 DEPLOYMENT VERIFY FAIL: missing verifier {name}")
    print(f"===== {name} =====")
    result = subprocess.run([sys.executable, str(path)], cwd=ROOT, env=ENV)
    if result.returncode:
        raise SystemExit(result.returncode)

print("R13 STAGED DEPLOYMENT VERIFICATION: PASS")
print(f"Executed {len(CHECKS)} frontend / deployment / spawn / reconciliation source gates.")
