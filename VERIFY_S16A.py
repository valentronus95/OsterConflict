from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCGeoReference.h',
    'Source/OsterConflict/Private/OCGeoReference.cpp',
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
    'Source/OsterConflict/Private/OCLocationSectorS01RoadData.cpp',
    'Source/OsterConflict/Private/OCGameMode.cpp',
    'Docs/SESSION_16A_README_UA.md',
    'Docs/OSTER_REFERENCE_MANIFEST_S16A.md',
    'Docs/MAP_REFERENCE_OSTER.md',
    'Docs/S16A_TEST_MATRIX.md',
    'Docs/ROADMAP_SESSIONS.md',
]
missing=[f for f in required if not (root/f).exists()]
if missing:
    print('Missing:', *missing, sep='\n - '); sys.exit(1)

markers={
    'OCGeoReference.h': [
        'OriginLatitude = 50.948239', 'OriginLongitude = 30.883865',
        'CentralPark()', 'CultureParkNorth()', 'FormerCityAdministration()',
        'HistoricCourtBuilding()', 'ResurrectionChurch()', 'EOCReferenceConfidence'
    ],
    'OCGeoReference.cpp': [
        '50.949182, 30.879127', '50.951645, 30.875861', '50.954943, 30.875112',
        '50.949419, 30.877258', '50.952622, 30.877788', '50.954472, 30.873668',
        'MetersPerDegreeLongitude'
    ],
    'OCWorldSectorOster.cpp': [
        'MapWidthCm = 240000.0f', 'MapHeightCm = 240000.0f',
        'BuildHydrography();', 'BuildVerifiedReferenceMarkers();',
        'Waterways', 'Bridges', 'ReferenceMarkers',
        'void AOCWorldSectorOster::BuildRoadNetwork()',
        'const FVector Park = ParkAnchor();', 'const FVector College = CollegeAnchor();',
        'FOCGeoReference::CentralPark()', 'FOCGeoReference::CultureParkNorth()',
        'S16A VERIFIED ANCHOR', '10500, 6800',
        'FOCLocationSectorPlan::IsInsideKrushelnytskaCollegePark(Block.Origin)'
    ],
    'OCGameMode.cpp': [
        'SpawnActor<AOCWorldSectorOster>', 'AOCWorldSectorOster::ParkAnchor()',
        'FVector(-106000.0f, -90000.0f, 40.0f)',
        'FVector(106000.0f, 90000.0f, 40.0f)'
    ],
    'SESSION_16A_README_UA.md': [
        '2.4 × 2.4 км', 'Генеральний план м. Остер', '50.951645, 30.875861',
        'Confidence model'
    ],
    'OSTER_REFERENCE_MANIFEST_S16A.md': [
        'MuseumSolonyna', 'CentralCityPark', 'CultureHouseParkNorth',
        'Генеральний план', 'Приватна забудова', 'not copied into the distributable game archive'
    ],
    'MAP_REFERENCE_OSTER.md': ['Museum / Solonyna estate', 'Central City Park', 'Official Oster community planning page'],
    'ROADMAP_SESSIONS.md': ['S16A — Reference-driven Oster map / georeference [ВИКОНАНО В ЦЬОМУ АРХІВІ]']
}
for name,needles in markers.items():
    paths=list(root.rglob(name)); text=paths[0].read_text(errors='ignore') if paths else ''
    for needle in needles:
        if needle not in text:
            print(f'Missing marker {needle!r} in {name}'); sys.exit(1)

world=(root/'Source/OsterConflict/Private/OCWorldSectorOster.cpp').read_text(errors='ignore')
road_data=(root/'Source/OsterConflict/Private/OCLocationSectorS01RoadData.cpp').read_text(errors='ignore')

# S16A originally authored the Krushelnytska spine as one direct 112000 cm corridor. Location-first S01 later split
# that exact corridor at workflow ownership boundaries. Accept either the historical direct representation or the
# current explicit split manifest, but never accept the spine disappearing entirely.
legacy_road_spine = re.search(
    r'AddRoadWithWalks\s*\(\s*FVector\s*\(\s*-33500\s*,\s*25000\s*,\s*RoadZ\s*\)\s*,\s*'
    r'FVector\s*\(\s*112000\s*,\s*920\s*,\s*16\s*\)\s*,\s*91\.5f\s*\)',
    world,
)
split_road_spine = (
    'FOCLocationSectorS01RoadData::KrushelnytskaSpineSegments()' in world and
    'S01_KR_SPINE_SOUTH_SHARED' in road_data and
    'S01_KR_SPINE_INSIDE' in road_data and
    'S01_KR_SPINE_NORTH_SHARED' in road_data and
    road_data.count('91.5f, true') >= 3
)
if not legacy_road_spine and not split_road_spine:
    print('Missing S16A Krushelnytska road-spine structure (legacy direct or S01 ownership split)'); sys.exit(1)

# Museum origin should be deterministic and coordinates separated from layout code.
gh=(root/'Source/OsterConflict/Public/OCGeoReference.h').read_text(errors='ignore')
gc=(root/'Source/OsterConflict/Private/OCGeoReference.cpp').read_text(errors='ignore')
if 'ToLocalCm' not in gh or 'EastMeters' not in gc or 'NorthMeters' not in gc:
    print('Georeference transform missing'); sys.exit(1)

# S16A must not accidentally bundle the large public planning reference PDF.
for p in root.rglob('*'):
    if p.is_file() and p.suffix.lower()=='.pdf':
        print('Unexpected external PDF bundled in game project:',p); sys.exit(1)

# Keep generated/runtime junk out of the source milestone.
for bad in ['Binaries','Intermediate','DerivedDataCache','Saved']:
    if any(p.is_dir() and p.name==bad for p in root.rglob(bad)):
        print('Build/runtime junk in archive:',bad); sys.exit(1)

# C++ delimiter sanity.
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

# UHT generated include stays last include in reflected headers.
for p in (root/'Source/OsterConflict/Public').rglob('*.h'):
    lines=p.read_text(errors='ignore').splitlines()
    gen=[i for i,l in enumerate(lines) if '.generated.h"' in l]
    if gen:
        inc=[i for i,l in enumerate(lines) if l.strip().startswith('#include')]
        if gen[-1] != max(inc): print('generated.h is not last include',p); sys.exit(1)

# No duplicate C++ method definitions for the georef/world-sector classes.
for cpp_name,class_name in [('OCGeoReference.cpp','FOCGeoReference'),('OCWorldSectorOster.cpp','AOCWorldSectorOster')]:
    text=(root/'Source/OsterConflict/Private'/cpp_name).read_text(errors='ignore')
    names=re.findall(rf'\b{class_name}::(\w+)\s*\(',text)
    dup=sorted({n for n in names if names.count(n)>1})
    if dup:
        print('Duplicate method definitions',cpp_name,dup); sys.exit(1)

print('S16A structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len,markers.values()))} S16A markers plus legacy-or-split Krushelnytska road spine.')
