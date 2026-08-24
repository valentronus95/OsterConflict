from pathlib import Path
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
ENV = os.environ.copy()
# The verification corpus contains Ukrainian UTF-8 source/docs. Windows runners otherwise inherit
# cp1252 for both default text decoding and stdout, which makes valid Unicode markers look missing
# and can crash while printing the diagnostic. Force every child verifier into UTF-8 mode.
ENV['PYTHONUTF8'] = '1'
ENV['PYTHONIOENCODING'] = 'utf-8'


def run_verifier(path: Path) -> None:
    print(f'===== {path.name} =====')
    result = subprocess.run([sys.executable, str(path)], cwd=ROOT, env=ENV)
    if result.returncode:
        raise SystemExit(result.returncode)


ordered = [
    'S04', 'S05', 'S06', 'S07', 'S08', 'S09', 'S10', 'S11', 'S12', 'S13',
    'S14A', 'S14B', 'S15A', 'S15B', 'S16A', 'S16B', 'S16C', 'S17A', 'S17B',
    'S18A', 'S18B', 'S18C'
]
for tag in ordered:
    file_path = ROOT / f'VERIFY_{tag}.py'
    if file_path.exists():
        run_verifier(file_path)

for verifier in [
    'VERIFY_S18C_HARDENING_R1.py',
    'VERIFY_S19C_SOURCE.py',
    'VERIFY_R6_LAUNCH_KIT.py',
    'VERIFY_R7_LOGIC_PHYSICS.py',
    'VERIFY_R8_UE58_TARGETS.py',
    'VERIFY_R9_UHT_TRAUMA.py',
    'VERIFY_R10_CXX_BATCH_FIX.py',
    'VERIFY_R11_VISUAL_FOUNDATION.py',
    'VERIFY_GAMEPLAY_INPUT_PASS_31.py',
    'VERIFY_MUSEUM_LAYER_BUDGET_PASS_32.py',
    'VERIFY_RUNTIME_ACCEPTANCE_PASS_33.py',
    'VERIFY_RUNTIME_LOCATION_MAP_PASS_35.py',
    'VERIFY_WEAPON_MATERIAL_LOWCPU_PERF_PASS_36.py',
    'VERIFY_VISIBLE_MUSEUM_WEAPON_PALETTE_PASS_37.py',
    'VERIFY_RUNTIME_RUNAWAY_HEAT_PASS_38.py',
    'VERIFY_VISUAL_QUALITY_TICK_BUDGET_PASS_39.py',
]:
    run_verifier(ROOT / verifier)

print('ALL SOURCE + PASS39 VISUAL/TICK BUDGET + PASS38 RUNAWAY/HEAT + PASS37 VISIBLE MUSEUM/PALETTE + PASS36 WEAPON/PERF + PASS35 LOCATION/MAP + PASS33 RUNTIME ACCEPTANCE + PASS32 MUSEUM BUDGET + PASS31 INPUT + R11 VISUAL + R10 CXX + R9 UHT + R8 UE5.8 TARGET + R7 LOGIC/PHYSICS VERIFIERS: PASS')