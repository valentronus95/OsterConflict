from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
forbidden={'Binaries','Intermediate','Saved','DerivedDataCache','.vs'}
found=[]
for d in ROOT.rglob('*'):
    if d.is_dir() and d.name in forbidden: found.append(str(d.relative_to(ROOT)))
large=[]
for f in ROOT.rglob('*'):
    if f.is_file() and f.stat().st_size > 50*1024*1024: large.append((f.stat().st_size,str(f.relative_to(ROOT))))
uproject=ROOT/'OsterConflict.uproject'
server=ROOT.parent/'OsterConflict'/ 'Source' if False else None
print('S18A release-tree audit')
print('project:',ROOT)
print('forbidden build/cache dirs:', found or 'none')
print('files >50 MiB:', large or 'none')
print('uproject:', 'OK' if uproject.exists() else 'MISSING')
if found or not uproject.exists(): sys.exit(1)
print('AUDIT_RELEASE_TREE: PASS')
