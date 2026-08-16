from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
    'Source/OsterConflict/Private/OCGameMode.cpp',
    'Docs/SESSION_09_README_UA.md',
    'Docs/OSTER_REFERENCE_MANIFEST_S09.md',
    'Docs/S09_TEST_MATRIX.md',
    'Docs/ROADMAP_SESSIONS.md',
]
missing = [f for f in required if not (root / f).exists()]
if missing:
    print('Missing:', *missing, sep='\n - '); sys.exit(1)

markers = {
    'OCWorldSectorOster.h': [
        'reference-driven', 'ResidentialRoofs', 'LandmarkRoofs', 'LandmarkWindows',
        'StadiumDetails', 'ParkDetails', 'AddGableRoof', 'AddFacadeWindow'
    ],
    'OCWorldSectorOster.cpp': [
        'SOLONYNA HOUSE', 'Stadium:',
        'SOLOMII KRUSHELNYTSKOI 7A',
        'red-brick single-storey wings', '10500, 6800', 'Columns = 9', 'Rows = 4',
        'Small skate/active-recreation pad', 'Detached rear shed/outbuilding',
        'AddGableRoof(ResidentialRoofs', 'tall conifers'
    ],
    'SESSION_09_README_UA.md': ['reference-driven', '105×68', '4 поверхи', 'Приватний сектор'],
    'OSTER_REFERENCE_MANIFEST_S09.md': ['Travels in Ukraine', 'OTG.cn.ua', 'Матеріально-технічна база', 'Остер з висоти пташиного польоту'],
    'ROADMAP_SESSIONS.md': ['S09 — Остер: парк + коледж + reference fidelity pass [ВИКОНАНО В ЦЬОМУ АРХІВІ]'],
}
for name, needles in markers.items():
    paths=list(root.rglob(name)); text=paths[0].read_text(errors='ignore') if paths else ''
    for needle in needles:
        if needle not in text:
            print(f'Missing marker {needle!r} in {name}'); sys.exit(1)

# delimiter sanity
for p in list((root/'Source').rglob('*.h')) + list((root/'Source').rglob('*.cpp')):
    t=p.read_text(errors='ignore')
    x=re.sub(r'//.*','',t); x=re.sub(r'/\*.*?\*/','',x,flags=re.S); x=re.sub(r'"(?:\\.|[^"\\])*"','""',x)
    for a,b in [('(',')'),('[',']'),('{','}')]:
        d=0
        for ch in x:
            if ch==a: d+=1
            elif ch==b:
                d-=1
                if d<0: print('Delimiter underflow',p,a,b); sys.exit(1)
        if d: print('Delimiter mismatch',p,a,b,d); sys.exit(1)

# generated.h last include
for p in (root/'Source/OsterConflict/Public').rglob('*.h'):
    lines=p.read_text(errors='ignore').splitlines()
    gen=[i for i,l in enumerate(lines) if '.generated.h"' in l]
    if gen:
        inc=[i for i,l in enumerate(lines) if l.strip().startswith('#include')]
        if gen[-1] != max(inc): print('generated.h is not last include',p); sys.exit(1)

# older gameplay must remain active
gm=(root/'Source/OsterConflict/Private/OCGameMode.cpp').read_text(errors='ignore')
for needle in ['SpawnActor<AOCWorldSectorOster>', 'SpawnActor<AOCEnterableHouse>', 'ObjectiveSeeds', 'SpawnSeeds']:
    if needle not in gm: print('Gameplay/map regression', needle); sys.exit(1)
if 'SpawnActor<AOCTestArena>' in gm: print('Old arena became active'); sys.exit(1)

print('S09 structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len, markers.values()))} S09 markers.')
