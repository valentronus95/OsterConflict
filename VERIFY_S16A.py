from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCGeoReference.h',
    'Source/OsterConflict/Private/OCGeoReference.cpp',
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
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
    # S16A established georeference/topology. Pass 44 explicitly supersedes its old 2.4 km blockout size.
    # Keep the reference topology markers, but require the current compact primary authoring contract instead.
    'OCWorldSectorOster.cpp': [
        'MinPlayableX = -78000.0f', 'MaxPlayableX =  18000.0f',
        'MinPlayableY = -12000.0f', 'MaxPlayableY =  82000.0f',
        'MapWidthCm = MaxPlayableX - MinPlayableX', 'MapHeightCm = MaxPlayableY - MinPlayableY',
        'Ground->SetRelativeLocation(FVector(MapCenterX, MapCenterY, -100.0f))',
        'IntersectsPlayableAuthoringBounds', 'IsPointInsidePlayableAuthoringBounds',
        'BuildHydrography();', 'BuildVerifiedReferenceMarkers();',
        'Waterways', 'Bridges', 'ReferenceMarkers',
        'S16A topology pass', 'official general plan',
        'FOCGeoReference::CentralPark()', 'FOCGeoReference::CultureParkNorth()',
        'S16A VERIFIED ANCHOR', '10500, 6800',
        'S16A variation: houses/lots are intentionally imperfect',
        'PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY'
    ],
    'OCGameMode.cpp': [
        'SpawnActor<AOCWorldSectorOster>', 'AOCWorldSectorOster::ParkAnchor()',
        'FVector(-106000.0f, -90000.0f, 40.0f)',
        'FVector(106000.0f, 90000.0f, 40.0f)'
    ],
    # Historical documentation remains historical evidence. It may describe the old 2.4 km milestone,
    # but it is not allowed to force current runtime geometry back to that extent.
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
for stale in [
    'MapWidthCm = 240000.0f', 'MapHeightCm = 240000.0f',
    'FVector(-104000.0f, -92000.0f', 'FVector( 104000.0f,  92000.0f',
    'FVector(-112000, -25000', 'FVector( 82000, -52000'
]:
    if stale in world:
        print('Superseded Pass 44 world authoring returned:', stale); sys.exit(1)

# Museum origin should be deterministic and coordinates separated from layout code.
gh=(root/'Source/OsterConflict/Public/OCGeoReference.h').read_text(errors='ignore')
gc=(root/'Source/OsterConflict/Private/OCGeoReference.cpp').read_text(errors='ignore')
if 'ToLocalCm' not in gh or 'EastMeters' not in gc or 'NorthMeters' not in gc:
    print('Georeference transform missing'); sys.exit(1)

# S16A must not accidentally bundle the 74MB public planning reference PDF.
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

print('S16A structural verification: PASS (Pass 44 compact authoring supersedes old 2.4 km runtime extent)')
print(f'Checked {len(required)} required files and {sum(map(len,markers.values()))} S16A/Pass44 markers.')
