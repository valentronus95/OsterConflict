from pathlib import Path
import subprocess,sys
ROOT=Path(__file__).resolve().parent; P=ROOT/'OsterConflict'
required=[
 'Scripts/S18C/PREFLIGHT_S18C.py','Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1','Scripts/S18C/ANALYZE_BUILD_LOG.py',
 'Docs/SESSION_18C_README_UA.md','Docs/S18C_TEST_MATRIX.md','Scripts/S18B/BuildS18B.ps1','Scripts/S18B/SMOKE_LOCAL.ps1'
]
for r in required:
    if not (P/r).exists(): raise SystemExit('MISSING '+r)
run=subprocess.run([sys.executable,str(P/'Scripts/S18C/PREFLIGHT_S18C.py')],cwd=P,text=True,capture_output=True)
print(run.stdout,end='')
if run.returncode: print(run.stderr,end=''); raise SystemExit(run.returncode)
text='\n'.join((P/r).read_text(errors='ignore') for r in required)
for marker in ['PREFLIGHT_S18C','WINDOWS_TOOLCHAIN_PREFLIGHT','ANALYZE_BUILD_LOG','UE 5.8','UBT','UAT']:
    if marker not in text: raise SystemExit('MISSING MARKER '+marker)
print('S18C structural verification: PASS')
