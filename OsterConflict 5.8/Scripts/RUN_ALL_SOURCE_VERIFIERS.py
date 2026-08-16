from pathlib import Path
import subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
verifiers=[]
for n in ['S04','S05','S06','S07','S08','S09','S10','S11','S12','S13','S14A','S14B','S15A','S15B','S16A','S16B','S16C','S17A','S17B','S18A','S18B','S18C','S18C_HARDENING_R1','S19C_SOURCE','R7_LOGIC_PHYSICS','R8_UE58_TARGETS','R9_UHT_TRAUMA','R10_CXX_BATCH_FIX','R11_VISUAL_FOUNDATION']:
    f=ROOT/f'VERIFY_{n}.py'
    if f.exists(): verifiers.append(f)
for f in verifiers:
    print('==',f.name)
    r=subprocess.run([sys.executable,str(f)],cwd=ROOT)
    if r.returncode: sys.exit(r.returncode)
print(f'ALL SOURCE VERIFIERS: PASS ({len(verifiers)})')
