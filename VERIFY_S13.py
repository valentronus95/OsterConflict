from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parent/'OsterConflict'
req=[
'Source/OsterConflict/Public/OCAIController.h','Source/OsterConflict/Private/OCAIController.cpp',
'Source/OsterConflict/Public/OCBotCharacter.h','Source/OsterConflict/Private/OCBotCharacter.cpp',
'Source/OsterConflict/Public/OCBotTypes.h','Docs/SESSION_13_README_UA.md','Docs/AI_ARCHITECTURE_S13.md','Docs/S13_TEST_MATRIX.md','Docs/GORE_ANIMATION_SPEC_S13.md',
'Scripts/RUN_S13_SERVER_8BOTS_NORMAL.bat','Scripts/RUN_S13_SERVER_16BOTS_HARD.bat','Scripts/RUN_S13_SANDBOX_BOTS.bat']
missing=[x for x in req if not (ROOT/x).exists()]
if missing: print('MISSING',missing);sys.exit(2)
texts='\n'.join(p.read_text(encoding='utf-8',errors='ignore') for p in ROOT.rglob('*') if p.suffix in {'.h','.cpp','.ini','.md','.bat','.cs'})
markers=['UAIPerceptionComponent','UAISenseConfig_Sight','EOCBotDifficulty','Easy','Normal','Hard','Veteran','BotDifficulty','?Bots=8',
'MoveToActor','MoveToLocation','ProjectPointToNavigation','StartAIReviveServer','TryEnterVehicleServer','SetAIDriveInputsServer','SpawnFourBots','ClearBots',
'RuntimeGeneration=Dynamic','AIModule','NavigationSystem','GameplayTasks','RequestStimuliListenerUpdate','CancelAIReviveServer','RestartBotController']
miss=[m for m in markers if m not in texts]
if miss: print('MISSING MARKERS',miss);sys.exit(3)
# every generated header after normal includes in class headers
for h in (ROOT/'Source/OsterConflict/Public').glob('*.h'):
    lines=h.read_text(encoding='utf-8',errors='ignore').splitlines()
    gi=[i for i,l in enumerate(lines) if '.generated.h"' in l]
    if gi and any(l.startswith('#include ') for l in lines[gi[0]+1:]):
        print('BAD GENERATED INCLUDE ORDER',h.name);sys.exit(4)
# RPC declarations must have implementation
headers='\n'.join(p.read_text(encoding='utf-8',errors='ignore') for p in (ROOT/'Source/OsterConflict/Public').glob('*.h'))
cpps='\n'.join(p.read_text(encoding='utf-8',errors='ignore') for p in (ROOT/'Source/OsterConflict/Private').glob('*.cpp'))
for name in re.findall(r'UFUNCTION\(Server[^)]*\)\s*(?:\n\s*)?void\s+(\w+)\s*\(',headers):
    if f'{name}_Implementation' not in cpps:
        print('RPC IMPLEMENTATION MISSING',name);sys.exit(5)
print('S13 structural verification: PASS')
print(f'Checked {len(req)} required files and {len(markers)} AI markers.')
