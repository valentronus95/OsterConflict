from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCBuildVersion.h','Source/OsterConflict/Private/OsterConflict.cpp',
 'Source/OsterConflict/Private/Tests/OCReleaseSmokeTests.cpp','Source/OsterConflict/Private/OCGameMode.cpp',
 'Config/DefaultGame.ini','OsterConflict.uproject',
 'Scripts/S18B/CREATE_RELEASE_MAP.py','Scripts/S18B/BuildS18B.ps1','Scripts/S18B/BUILD_S18B_ALL.bat',
 'Scripts/S18B/POST_BUILD_AUDIT.py','Scripts/S18B/SMOKE_LOCAL.ps1','Scripts/S18B/COLLECT_RC_LOGS.ps1',
 'Docs/SESSION_18B_README_UA.md','Docs/FIRST_BUILD_RC_GATE_S18B.md','Docs/WINDOWS_TOOLCHAIN_SETUP_S18B.md','Docs/S18B_TEST_MATRIX.md'
]
for rel in required:
    if not (P/rel).exists(): raise SystemExit(f'MISSING {rel}')
alltext='\n'.join((P/r).read_text(errors='ignore') for r in required if (P/r).suffix in {'.h','.cpp','.md','.ini','.bat','.ps1','.py','.uproject'})
markers=[
 '0.0.18B-S18B','NetworkProtocol = 18','/Game/Maps/OsterConflict_Runtime','oc.BuildInfo',
 'OsterConflict.Release.BuildFingerprint','OsterConflict.Release.GeoReferenceOrigin','OsterConflict.Release.FactionNames',
 'AutoDeploy','PythonScriptPlugin','EditorLoadingAndSavingUtils.new_blank_map','save_map(world, MAP_PATH)',
 'RunUBT.bat','RunUAT.bat','UnrealEditor-Cmd.exe','OsterConflictEditor','OsterConflictClient','OsterConflictServer',
 'BuildCookRun','-cook','-stage','-pak','-package','-archive','POST_BUILD_AUDIT: PASS',
 'S18B_BUILD_MANIFEST.json','PACKAGED LOCAL SMOKE: PASS','Fatal error:','Assertion failed:'
]
missing=[m for m in markers if m not in alltext]
if missing: raise SystemExit('MISSING MARKERS: '+', '.join(missing))
# plugin is valid JSON and enabled
j=json.loads((P/'OsterConflict.uproject').read_text())
if not any(x.get('Name')=='PythonScriptPlugin' and x.get('Enabled') for x in j.get('Plugins',[])):
    raise SystemExit('PythonScriptPlugin not enabled')
# generated.h order remains valid in all UHT headers
for path in (P/'Source/OsterConflict/Public').rglob('*.h'):
    lines=path.read_text(errors='ignore').splitlines(); inc=[x.strip() for x in lines if x.strip().startswith('#include')]
    if any('generated.h' in x for x in inc) and 'generated.h' not in inc[-1]:
        raise SystemExit(f'generated.h order: {path.relative_to(P)}')
# delimiter sanity
cppfiles=list((P/'Source/OsterConflict').rglob('*.h'))+list((P/'Source/OsterConflict').rglob('*.cpp'))
for path in cppfiles:
    text=path.read_text(errors='ignore')
    for a,b in [('(',')'),('{','}'),('[',']')]:
        if text.count(a)!=text.count(b): raise SystemExit(f'Delimiter mismatch {a}{b}: {path.relative_to(P)}')
# source milestone must not contain generated binaries/caches
for bad in ['Binaries','Intermediate','Saved','DerivedDataCache']:
    if (P/bad).exists(): raise SystemExit(f'Forbidden generated dir in source milestone: {bad}')
print(f'S18B structural verification: PASS\nChecked {len(required)} required files, {len(markers)} build/RC markers and {len(cppfiles)} C++ files.')
