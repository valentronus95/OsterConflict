from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
    'Source/OsterConflict/Private/OCR137MuseumPhotoModelSubsystem.cpp',
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
        'Private generic residences are intentionally omitted', 'LandmarkRoofs', 'LandmarkWindows',
        'StadiumDetails', 'ParkDetails', 'AddGableRoof', 'AddFacadeWindow'
    ],
    'OCWorldSectorOster.cpp': [
        'BuildCentralPark();', 'BuildCollegeSector();', 'BuildMuseumAndStadium();',
        'Stadium:', 'SOLOMII KRUSHELNYTSKOI 7A',
        'PASS45_MUSEUM_LEGACY_BLOCKOUT_SOURCE_RETIRED', '10500, 6800', 'Columns = 9', 'Rows = 4',
        'Small skate/active-recreation pad',
        'PASS45_WORLD_GENERIC_RESIDENTIAL_RETIRED'
    ],
    'OCR137MuseumPhotoModelSubsystem.cpp': [
        'PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY',
        'visible_shell_owner=R137',
        'PASS45_MUSEUM_AUTHORED_SHELL_FAIL',
        'basicshape_fallback=0',
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

world=(root/'Source/OsterConflict/Private/OCWorldSectorOster.cpp').read_text(errors='ignore')
world_h=(root/'Source/OsterConflict/Public/OCWorldSectorOster.h').read_text(errors='ignore')
r137=(root/'Source/OsterConflict/Private/OCR137MuseumPhotoModelSubsystem.cpp').read_text(errors='ignore')

# Pass45 supersedes S09 private-sector approximations. Keep the public-reference manifest and
# reference-driven POI work, but never force rejected arbitrary residence/fence generators back.
for stale in (
    'BuildResidentialBlocks();',
    'void AOCWorldSectorOster::BuildResidentialBlocks()',
    'BuildSolomiiKrushelnytskoiStreet();',
    'void AOCWorldSectorOster::BuildSolomiiKrushelnytskoiStreet()',
    'AddGableRoof(ResidentialRoofs',
):
    if stale in world:
        print('Pass45 rejected S09 private-sector visual returned:', stale); sys.exit(1)
for stale in ('void BuildResidentialBlocks();', 'void BuildSolomiiKrushelnytskoiStreet();'):
    if stale in world_h:
        print('Pass45 rejected S09 private-sector declaration returned:', stale); sys.exit(1)

# Pass45 item 32 supersedes the old S09 museum Landmark* blockout. Museum reference fidelity now belongs to the
# authored R13.7 exterior owner; canonical current world source must not recreate a second visible shell.
museum_begin = world.find('void AOCWorldSectorOster::BuildMuseumAndStadium()')
stadium_begin = world.find('    // Stadium:', museum_begin)
if museum_begin < 0 or stadium_begin < 0:
    print('Cannot isolate Museum source section'); sys.exit(1)
museum_source = world[museum_begin:stadium_begin]
for stale in (
    'AddBox(LandmarkBlocks, Museum',
    'AddBox(LandmarkDetails, Museum',
    'AddGableRoof(LandmarkRoofs, Museum',
    'AddFacadeWindow(LandmarkWindows, Museum',
    'red-brick single-storey wings',
):
    if stale in museum_source:
        print('Pass45 retired S09 Museum world blockout returned:', stale); sys.exit(1)
if museum_source.count('AddBox(Fences, Museum') != 3:
    print('Museum perimeter fence proxy count changed unexpectedly'); sys.exit(1)
for needle in (
    'SuppressLegacyMuseum(World);',
    'PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY',
    'runtime_photo_acceptance=0',
):
    if needle not in r137:
        print('R13.7 Museum authoritative-owner regression', needle); sys.exit(1)

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

# Older gameplay/city-sector routing remains active, but the unreferenced S08 house placement must not.
gm=(root/'Source/OsterConflict/Private/OCGameMode.cpp').read_text(errors='ignore')
for needle in ['SpawnActor<AOCWorldSectorOster>', 'PASS45_GENERIC_ENTERABLE_HOUSE_RETIRED', 'ObjectiveSeeds', 'SpawnSeeds']:
    if needle not in gm: print('Gameplay/map regression', needle); sys.exit(1)
if 'SpawnActor<AOCEnterableHouse>' in gm:
    print('Pass45 rejected generic enterable-house normal-runtime spawn returned'); sys.exit(1)
if 'SpawnActor<AOCTestArena>' in gm: print('Old arena became active'); sys.exit(1)

print('S09 structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len, markers.values()))} S09 markers.')
print('Pass45 forward-port: public-reference POI fidelity retained; rejected private-sector generators and legacy Museum world blockout remain retired.')
