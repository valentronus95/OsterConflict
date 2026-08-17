from pathlib import Path
import subprocess,sys
ROOT=Path(__file__).resolve().parent
ordered=['S04','S05','S06','S07','S08','S09','S10','S11','S12','S13','S14A','S14B','S15A','S15B','S16A','S16B','S16C','S17A','S17B','S18A','S18B','S18C']
for tag in ordered:
    f=ROOT/f'VERIFY_{tag}.py'
    if not f.exists(): continue
    print(f'===== {f.name} =====')
    r=subprocess.run([sys.executable,str(f)],cwd=ROOT)
    if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_S18C_HARDENING_R1.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_S18C_HARDENING_R1.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_S19C_SOURCE.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_S19C_SOURCE.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R6_LAUNCH_KIT.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R6_LAUNCH_KIT.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R7_LOGIC_PHYSICS.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R7_LOGIC_PHYSICS.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R8_UE58_TARGETS.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R8_UE58_TARGETS.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R9_UHT_TRAUMA.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R9_UHT_TRAUMA.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R10_CXX_BATCH_FIX.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R10_CXX_BATCH_FIX.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R11_VISUAL_FOUNDATION.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R11_VISUAL_FOUNDATION.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('===== VERIFY_R13_PC_TEST_HARDENING.py =====')
r=subprocess.run([sys.executable,str(ROOT/'VERIFY_R13_PC_TEST_HARDENING.py')],cwd=ROOT)
if r.returncode: raise SystemExit(r.returncode)
print('ALL SOURCE + R13 PC HARDENING + R11 VISUAL + R10 CXX + R9 UHT + R8 UE5.8 TARGET + R7 LOGIC/PHYSICS VERIFIERS: PASS')
