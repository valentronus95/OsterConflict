from pathlib import Path
import re, sys
root=Path(__file__).resolve().parent/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCTeamTypes.h',
 'Source/OsterConflict/Public/OCGameState.h','Source/OsterConflict/Private/OCGameState.cpp',
 'Source/OsterConflict/Public/OCCapturePoint.h','Source/OsterConflict/Private/OCCapturePoint.cpp',
 'Source/OsterConflict/Public/OCTeamSpawnPoint.h','Source/OsterConflict/Private/OCTeamSpawnPoint.cpp',
 'Source/OsterConflict/Public/OCGameMode.h','Source/OsterConflict/Private/OCGameMode.cpp',
 'Source/OsterConflict/Public/OCPlayerState.h','Source/OsterConflict/Private/OCPlayerState.cpp',
 'Source/OsterConflict/Private/OCCharacter.cpp','Source/OsterConflict/Private/OCHealthComponent.cpp',
 'Source/OsterConflict/Private/OCWeaponBase.cpp','Source/OsterConflict/Private/OCHUD.cpp',
 'Docs/SESSION_06_README_UA.md','Docs/CONQUEST_ARCHITECTURE_S06.md','Docs/S06_TEST_MATRIX.md'
]
missing=[f for f in required if not (root/f).exists()]
if missing:
 print('Missing:',*missing,sep='\n - '); sys.exit(1)
markers={
 'OCGameMode.cpp':['GameStateClass = AOCGameState::StaticClass()','AssignBalancedTeam','RemoveTicketsServer','ApplyTicketBleed','RestartPrototypeRound','CanDealDamage','FindBestSpawnTransform'],
 'OCCapturePoint.cpp':['CaptureProgress','bContested','SetOwnerServer','GetOverlappingActors','ResetPointServer'],
 'OCTeamSpawnPoint.cpp':['IsAvailableForTeam','LinkedCapturePointId'],
 'OCPlayerState.cpp':['DOREPLIFETIME(AOCPlayerState, TeamId)','DOREPLIFETIME(AOCPlayerState, PlayerRole)'],
 'OCCharacter.cpp':['SelfState->IsMedic()','SelfState->GetTeamId() != TargetState->GetTeamId()','GetOCMatchPhase() == EOCMatchPhase::Ended'],
 'OCHealthComponent.cpp':['CanDealDamage(InstigatedBy, DamagedActor)'],
 'OCWeaponBase.cpp':['CanDealDamage(Shooter->GetController(), PelletHit.GetActor())'],
 'OCHUD.cpp':['DrawMatchStatus','TEAM 1','TEAM 2','GetTickets','GetOwnerTeam']
}
for name, needles in markers.items():
 path=next((p for p in root.rglob(name) if 'Intermediate' not in str(p)),None)
 text=path.read_text(errors='ignore') if path else ''
 for n in needles:
  if n not in text:
   print(f'Missing marker {n!r} in {name}'); sys.exit(1)
# Basic delimiter sanity excluding strings/comments imperfect but useful.
for p in list((root/'Source').rglob('*.h'))+list((root/'Source').rglob('*.cpp')):
 t=p.read_text(errors='ignore')
 # strip // comments and strings enough for brace sanity
 x=re.sub(r'//.*','',t)
 x=re.sub(r'"(?:\\.|[^"\\])*"','""',x)
 for a,b in [('(',')'),('[',']'),('{','}')]:
  depth=0
  for ch in x:
   if ch==a: depth+=1
   elif ch==b:
    depth-=1
    if depth<0:
     print('Delimiter underflow',p,a,b);sys.exit(1)
  if depth:
   print('Delimiter mismatch',p,a,b,depth);sys.exit(1)
# Every UFUNCTION(Server...) in character/controller must have Implementation.
for hname in ['OCCharacter.h','OCPlayerController.h']:
 hp=root/'Source/OsterConflict/Public'/hname
 cp=root/'Source/OsterConflict/Private'/hname.replace('.h','.cpp')
 ht=hp.read_text(); ct=cp.read_text()
 names=re.findall(r'UFUNCTION\(Server,[^\)]*\)\s*\n\s*void\s+(\w+)\s*\(',ht)
 for n in names:
  if f'{n}_Implementation' not in ct:
   print('Missing RPC implementation',n);sys.exit(1)
# Catch two regressions observed during S06 integration.
char=(root/'Source/OsterConflict/Private/OCCharacter.cpp').read_text()
if 'PrimaryWeapon = SpawnInventoryWeaponServer(StarterPrimaryWeaponClass);\n    PrimaryWeapon = SpawnInventoryWeaponServer' in char:
 print('Duplicate starter primary spawn');sys.exit(1)
hud=(root/'Source/OsterConflict/Private/OCHUD.cpp').read_text()
if 'if (!Attachments.IsEmpty())\n        if (!Attachments.IsEmpty())' in hud:
 print('Duplicate attachment if');sys.exit(1)
# Generated header should remain final include in reflected headers.
for p in (root/'Source/OsterConflict/Public').rglob('*.h'):
 t=p.read_text(errors='ignore').splitlines()
 gens=[i for i,l in enumerate(t) if '.generated.h"' in l]
 if gens:
  last_include=max(i for i,l in enumerate(t) if l.strip().startswith('#include'))
  if gens[-1]!=last_include:
   print('generated.h is not last include:',p);sys.exit(1)
print('S06 structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len,markers.values()))} S06 markers.')
