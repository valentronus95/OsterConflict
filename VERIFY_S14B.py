from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCTraumaTypes.h',
    'Source/OsterConflict/Public/OCCombatVisualComponent.h',
    'Source/OsterConflict/Private/OCCombatVisualComponent.cpp',
    'Source/OsterConflict/Public/OCDestructibleProp.h',
    'Source/OsterConflict/Private/OCDestructibleProp.cpp',
    'Docs/SESSION_14B_README_UA.md',
    'Docs/TRAUMA_DESTRUCTION_ARCHITECTURE_S14B.md',
    'Docs/S14B_TEST_MATRIX.md',
]
missing=[x for x in required if not (root/x).exists()]
if missing:
    print('Missing:', *missing, sep='\n  '); sys.exit(1)

checks = {
    'Blood severity enum': ('Source/OsterConflict/Public/OCTraumaTypes.h', 'EOCBloodSeverity'),
    'Extreme blood': ('Source/OsterConflict/Public/OCTraumaTypes.h', 'Extreme'),
    'Body zones': ('Source/OsterConflict/Public/OCTraumaTypes.h', 'HeadNeck'),
    'Dismemberment mask': ('Source/OsterConflict/Public/OCTraumaTypes.h', 'DismembermentMask'),
    'Replicated trauma': ('Source/OsterConflict/Public/OCCombatVisualComponent.h', 'ReplicatedUsing=OnRep_LastTraumaEvent'),
    'Gore cvar': ('Source/OsterConflict/Private/OCCombatVisualComponent.cpp', 'oc.GoreLevel'),
    'Ragdoll': ('Source/OsterConflict/Private/OCCombatVisualComponent.cpp', 'SetAllBodiesSimulatePhysics'),
    'Hide bone': ('Source/OsterConflict/Private/OCCombatVisualComponent.cpp', 'HideBoneByName'),
    'Local chunk cleanup': ('Source/OsterConflict/Private/OCCombatVisualComponent.cpp', 'LocalChunkLifetime'),
    'Weapon trauma': ('Source/OsterConflict/Private/OCWeaponBase.cpp', 'RecordPointTraumaServer'),
    'Turret trauma': ('Source/OsterConflict/Private/OCArmedVehicleBase.cpp', 'RecordPointTraumaServer'),
    'Grenade radial trauma': ('Source/OsterConflict/Private/OCGrenadeProjectile.cpp', 'RecordRadialTraumaServer'),
    'Impact routing': ('Source/OsterConflict/Public/OCWeaponBase.h', 'MulticastImpactFX'),
    'Flesh surface': ('Source/OsterConflict/Private/OCWeaponBase.cpp', 'EOCImpactSurface::Flesh'),
    'Destructible prop': ('Source/OsterConflict/Public/OCDestructibleProp.h', 'AOCDestructibleProp'),
    'Local destruction chunks': ('Source/OsterConflict/Private/OCDestructibleProp.cpp', 'LocalChunkCount'),
    'Corpse lifetime': ('Source/OsterConflict/Public/OCGameMode.h', 'CorpseLifetimeSeconds = 30.0f'),
    'Corpse cap': ('Source/OsterConflict/Public/OCGameMode.h', 'MaxPersistentCorpses = 20'),
    'Corpse queue': ('Source/OsterConflict/Private/OCGameMode.cpp', 'RegisterCorpse'),
    'Destruction lane': ('Source/OsterConflict/Private/OCGameMode.cpp', 'DestructionSeeds'),
}
for name,(file,token) in checks.items():
    if token not in (root/file).read_text(errors='ignore'):
        print(f'FAIL marker: {name} -> {file}: {token}'); sys.exit(1)

# generated.h must be the final project include in UHT headers.
for h in (root/'Source/OsterConflict/Public').glob('*.h'):
    text=h.read_text(errors='ignore')
    if 'generated.h' not in text: continue
    includes=[line.strip() for line in text.splitlines() if line.strip().startswith('#include')]
    generated=[i for i,line in enumerate(includes) if '.generated.h"' in line]
    if not generated or generated[-1] != len(includes)-1:
        print('FAIL generated.h order:', h.name); sys.exit(1)

# Basic balanced delimiter audit with strings/comments stripped enough for source sanity.
def strip_cpp(text):
    text=re.sub(r'/\*.*?\*/','',text,flags=re.S)
    text=re.sub(r'//.*','',text)
    text=re.sub(r'"(?:\\.|[^"\\])*"','""',text)
    text=re.sub(r"'(?:\\.|[^'\\])*'","''",text)
    return text

cpp_files=list((root/'Source').rglob('*.cpp'))+list((root/'Source').rglob('*.h'))
for f in cpp_files:
    t=strip_cpp(f.read_text(errors='ignore'))
    for a,b in [('(',')'),('{','}'),('[',']')]:
        depth=0
        for ch in t:
            if ch==a: depth+=1
            elif ch==b:
                depth-=1
                if depth<0:
                    print('FAIL delimiter', f, a+b); sys.exit(1)
        if depth!=0:
            print('FAIL delimiter balance', f, a+b, depth); sys.exit(1)

# Character header regression: no accidental duplicate private declaration left behind.
char_h=(root/'Source/OsterConflict/Public/OCCharacter.h').read_text()
if char_h.count('void CancelGiveUpServer();') != 1:
    print('FAIL duplicate CancelGiveUpServer declaration'); sys.exit(1)

print('S14B structural verification: PASS')
print(f'Checked {len(required)} required files, {len(checks)} S14B markers and {len(cpp_files)} C++ headers/sources.')
