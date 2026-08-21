from pathlib import Path
import re, sys

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
if hasattr(sys.stderr, 'reconfigure'):
    sys.stderr.reconfigure(encoding='utf-8', errors='replace')

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
    'Source/OsterConflict/Public/OCGameMode.h',
    'Source/OsterConflict/Private/OCGameMode.cpp',
    'Source/OsterConflict/Public/OCCapturePoint.h',
    'Source/OsterConflict/Private/OCCapturePoint.cpp',
    'Source/OsterConflict/Public/OCTeamSpawnPoint.h',
    'Source/OsterConflict/Private/OCTeamSpawnPoint.cpp',
    'Docs/SESSION_07_README_UA.md',
    'Docs/OSTER_MAP_ARCHITECTURE_S07.md',
    'Docs/S07_TEST_MATRIX.md',
    'Docs/ROADMAP_SESSIONS.md',
]
missing = [f for f in required if not (root / f).exists()]
if missing:
    print('Missing:', *missing, sep='\n - ')
    sys.exit(1)

markers = {
    'OCWorldSectorOster.cpp': [
        'MapWidthCm =',
        'MapHeightCm =',
        'FVector AOCWorldSectorOster::CollegeAnchor()',
        'FVector AOCWorldSectorOster::ParkAnchor()',
        'BuildMuseumAndStadium();',
        'BuildCentralPark();',
        'BuildCollegeSector();',
        'BuildResidentialBlocks();',
        'BuildVegetation();',
        'OSTER LOCAL HISTORY MUSEUM / TATARIVSKA 30',
        'OSTER COLLEGE / SOLOMII KRUSHELNYTSKOI 7A',
    ],
    'OCGameMode.cpp': [
        'SpawnOsterCenterSector();',
        'SpawnActor<AOCWorldSectorOster>',
        'AOCWorldSectorOster::MuseumAnchor()',
        'AOCWorldSectorOster::StadiumAnchor()',
        'AOCWorldSectorOster::ParkAnchor()',
        'AOCWorldSectorOster::CollegeAnchor()',
        'TEXT("A")', 'TEXT("B")', 'TEXT("C")',
        'SpawnSeeds',
        'bBase',
    ],
    'ROADMAP_SESSIONS.md': [
        'S07 — Остер Greybox, Sector A [ВИКОНАНО В ЦЬОМУ АРХІВІ]',
    ],
    'SESSION_07_README_UA.md': [
        'Називний відмінок: **Остер**',
        'Родовий відмінок: **Остра**',
        'Чернігівського району',
        'км',
    ],
}
for name, needles in markers.items():
    path = next((p for p in root.rglob(name) if 'Intermediate' not in str(p)), None)
    text = path.read_text(errors='ignore') if path else ''
    for n in needles:
        if n not in text:
            print(f'Missing marker {n!r} in {name}')
            sys.exit(1)

# Old tiny arena must not be spawned by the active GameMode anymore.
gm = (root/'Source/OsterConflict/Private/OCGameMode.cpp').read_text(errors='ignore')
if 'SpawnActor<AOCTestArena>' in gm or 'SpawnPrototypeArena();' in gm:
    print('Old OCTestArena still active in S07 GameMode')
    sys.exit(1)

# Basic delimiter sanity for C++ source. Strings/comments stripped enough to catch integration accidents.
for p in list((root/'Source').rglob('*.h')) + list((root/'Source').rglob('*.cpp')):
    t = p.read_text(errors='ignore')
    x = re.sub(r'//.*', '', t)
    x = re.sub(r'/\*.*?\*/', '', x, flags=re.S)
    x = re.sub(r'"(?:\\.|[^"\\])*"', '""', x)
    for a, b in [('(', ')'), ('[', ']'), ('{', '}')]:
        depth = 0
        for ch in x:
            if ch == a:
                depth += 1
            elif ch == b:
                depth -= 1
                if depth < 0:
                    print('Delimiter underflow', p, a, b)
                    sys.exit(1)
        if depth:
            print('Delimiter mismatch', p, a, b, depth)
            sys.exit(1)

# UHT reflected headers: generated include must remain the last include.
for p in (root/'Source/OsterConflict/Public').rglob('*.h'):
    lines = p.read_text(errors='ignore').splitlines()
    generated = [i for i, line in enumerate(lines) if '.generated.h"' in line]
    if generated:
        include_lines = [i for i, line in enumerate(lines) if line.strip().startswith('#include')]
        if generated[-1] != max(include_lines):
            print('generated.h is not last include:', p)
            sys.exit(1)

# Network regression: every declared Server RPC in character/controller keeps an _Implementation.
for hname in ['OCCharacter.h', 'OCPlayerController.h']:
    hp = root/'Source/OsterConflict/Public'/hname
    cp = root/'Source/OsterConflict/Private'/hname.replace('.h', '.cpp')
    ht, ct = hp.read_text(), cp.read_text()
    names = re.findall(r'UFUNCTION\(Server,[^\)]*\)\s*\n\s*void\s+(\w+)\s*\(', ht)
    for n in names:
        if f'{n}_Implementation' not in ct:
            print('Missing RPC implementation', n)
            sys.exit(1)

# Ukrainian project wording regression requested by owner.
for p in list((root/'Docs').rglob('*.md')) + [root/'README.md']:
    text = p.read_text(errors='ignore')
    if 'карту Остера' in text or 'карта Остера' in text:
        print('Incorrect Ukrainian genitive remains:', p)
        sys.exit(1)

print('S07 structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len, markers.values()))} S07 markers.')
