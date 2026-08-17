from pathlib import Path
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
ENV = os.environ.copy()
ENV["PYTHONUTF8"] = "1"
ENV["PYTHONIOENCODING"] = "utf-8"


def run_verifier(path: Path) -> None:
    print(f"===== {path.name} =====")
    result = subprocess.run([sys.executable, str(path)], cwd=ROOT, env=ENV)
    if result.returncode:
        raise SystemExit(result.returncode)


ordered = [
    "S04", "S05", "S06", "S07", "S08", "S09", "S10", "S11", "S12", "S13",
    "S14A", "S14B", "S15A", "S15B", "S16A", "S16B", "S16C", "S17A", "S17B",
    "S18A", "S18B", "S18C",
]
for tag in ordered:
    verifier = ROOT / f"VERIFY_{tag}.py"
    if verifier.exists():
        run_verifier(verifier)

for name in [
    "VERIFY_S18C_HARDENING_R1.py",
    "VERIFY_S19C_SOURCE.py",
    "VERIFY_R6_LAUNCH_KIT.py",
    "VERIFY_R7_LOGIC_PHYSICS.py",
    "VERIFY_R8_UE58_TARGETS.py",
    "VERIFY_R9_UHT_TRAUMA.py",
    "VERIFY_R10_CXX_BATCH_FIX.py",
    "VERIFY_R11_VISUAL_FOUNDATION.py",
    "VERIFY_R13_PC_TEST_HARDENING.py",
]:
    run_verifier(ROOT / name)

print("ALL SOURCE + R13 PC HARDENING + R11 VISUAL + R10 CXX + R9 UHT + R8 UE5.8 TARGET + R7 LOGIC/PHYSICS VERIFIERS: PASS")
