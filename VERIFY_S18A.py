from pathlib import Path
import re
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCGameMode.h','Source/OsterConflict/Private/OCGameMode.cpp',
 'Source/OsterConflict/Public/OCAIController.h','Source/OsterConflict/Private/OCAIController.cpp',
 'Source/OsterConflict/Public/OCInteractableDoor.h','Source/OsterConflict/Private/OCInteractableDoor.cpp',
 'Source/OsterConflict/Private/OCAmbientAudioZone.cpp','Source/OsterConflict/Private/OCBreakableWindow.cpp',
 'Source/OsterConflict/Private/OCDestructibleProp.cpp','Source/OsterConflict/Private/OCCapturePoint.cpp',
 'Source/OsterConflict/Public/OCPlayerController.h','Source/OsterConflict/Private/OCPlayerController.cpp',
 'Config/DefaultGame.ini','Config/DefaultEngine.ini',
 'Docs/SESSION_18A_README_UA.md','Docs/PERFORMANCE_BUDGETS_S18A.md','Docs/S18A_TEST_MATRIX.md','Docs/PACKAGING_READINESS_S18A.md',
 'Scripts/RUN_S18A_SERVER_8_BALANCED.bat','Scripts/RUN_S18A_SERVER_16_BALANCED.bat',
 'Scripts/RUN_S18A_SERVER_16_LOWCPU.bat','Scripts/RUN_S18A_SERVER_16_INSIGHTS.bat',
 'Scripts/RUN_S18A_CLIENT_INSIGHTS.bat','Scripts/AUDIT_RELEASE_TREE.py','Scripts/RUN_ALL_SOURCE_VERIFIERS.py'
]
for rel in required:
    if not (P/rel).exists(): raise SystemExit(f'MISSING {rel}')
alltext='\n'.join((P/r).read_text(errors='ignore') for r in required if (P/r).suffix in {'.h','.cpp','.md','.ini','.bat','.py'})
markers=[
 'PerfProfile','LowCPU','Balanced','Quality','AIThinkIntervalScale','BuildPerformanceSnapshot','PerfReport','ServerRequestPerfReport','ClientReceivePerfReport',
 'MaxPersistentCorpses = FMath::Min(MaxPersistentCorpses, 10)','MaxPersistentCorpses = FMath::Min(MaxPersistentCorpses, 16)',
 'Tuning.ThinkInterval * GM->GetAIThinkIntervalScale()',
 'PrimaryActorTick.bStartWithTickEnabled = false','OnRep_Open','SetActorTickEnabled(false)','SetNetUpdateFrequency(4.0f)',
 'PrimaryActorTick.TickInterval=0.20f','NM_DedicatedServer','SetNetUpdateFrequency(1.0f)',
 'SetNetUpdateFrequency(2.0f)','SetNetUpdateFrequency(10.0f)',
 'ProjectVersion=0.0.18','Networking Insights','World Partition','HLOD','AUDIT_RELEASE_TREE: PASS'
]
missing=[m for m in markers if m not in alltext]
if missing: raise SystemExit('MISSING MARKERS: '+', '.join(missing))
# generated.h remains last include in touched UHT headers
for rel in ['Source/OsterConflict/Public/OCGameMode.h','Source/OsterConflict/Public/OCAIController.h','Source/OsterConflict/Public/OCInteractableDoor.h','Source/OsterConflict/Public/OCPlayerController.h']:
    lines=(P/rel).read_text(errors='ignore').splitlines(); inc=[x.strip() for x in lines if x.strip().startswith('#include')]
    if not inc or 'generated.h' not in inc[-1]: raise SystemExit(f'generated.h order: {rel}')
# new RPC declarations must have implementations
h=(P/'Source/OsterConflict/Public/OCPlayerController.h').read_text(errors='ignore')
cpp=(P/'Source/OsterConflict/Private/OCPlayerController.cpp').read_text(errors='ignore')
for rpc in ['ServerRequestPerfReport','ClientReceivePerfReport']:
    if f'{rpc}_Implementation' not in cpp: raise SystemExit(f'RPC implementation missing: {rpc}')
# delimiter sanity all project C++
cppfiles=list((P/'Source/OsterConflict').rglob('*.h'))+list((P/'Source/OsterConflict').rglob('*.cpp'))
for path in cppfiles:
    text=path.read_text(errors='ignore')
    for a,b in [('(',')'),('{','}'),('[',']')]:
        if text.count(a)!=text.count(b): raise SystemExit(f'Delimiter mismatch {a}{b}: {path.relative_to(P)}')
# avoid accidental project-generated/cache payloads
for bad in ['Binaries','Intermediate','Saved','DerivedDataCache']:
    if (P/bad).exists(): raise SystemExit(f'Forbidden generated dir in source milestone: {bad}')
print(f'S18A structural verification: PASS\nChecked {len(required)} required files, {len(markers)} optimization/release markers and {len(cppfiles)} C++ files.')
